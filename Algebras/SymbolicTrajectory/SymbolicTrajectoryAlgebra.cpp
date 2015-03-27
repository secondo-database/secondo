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
This file contains the operators of the Symbolic Trajectory Algebra.

\section{Defines and Includes}

*/

#include "Algorithms.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

namespace stj {
  
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

GenTC<PatPersistent> patternTC;

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
\section{Operator ~contains~}

contains: labels x label -> bool
contains: places x place -> bool
contains: labels x labels -> bool
contains: places x places -> bool

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
    if (Label::checkType(second) || Labels::checkType(second)) {
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
    return Label::checkType(nl->Second(args)) ? 0 : 2;
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
    signature = "labels x label -> bool";
    appendSignature("places x place -> bool");
    syntax    = "_ contains _;";
    meaning   = "Checks whether a labels/places object contains a label/place.";
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
  if (first->IsDefined() && second->IsDefined()) {
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

/*
\section{Operator ~distance~}

distance: T x T -> double,   where T in {place(s), label(s)}

\subsection{Type Mapping}

*/
ListExpr distanceTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    return NList::typeError("Expecting two arguments.");
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if ((Label::checkType(first)   && Label::checkType(second))   || 
      (Labels::checkType(first)  && Labels::checkType(second))  ||
      (Place::checkType(first)   && Place::checkType(second))   ||
      (Places::checkType(first)  && Places::checkType(second))  ||
      (MLabel::checkType(first)  && MLabel::checkType(second))  || 
      (MLabels::checkType(first) && MLabels::checkType(second)) || 
      (MPlace::checkType(first)  && MPlace::checkType(second))  || 
      (MPlaces::checkType(first) && MPlaces::checkType(second))) {
    return nl->SymbolAtom(CcReal::BasicType());
  }
  return NList::typeError("Expecting T x T, where T in {(m)place(s), "
                          "(m)label(s)}");
}

/*
\subsection{Selection Function}

*/
int distanceSelect(ListExpr args) {
  if (Label::checkType(nl->First(args)))   return 0; 
  if (Labels::checkType(nl->First(args)))  return 1;
  if (Place::checkType(nl->First(args)))   return 2; 
  if (Places::checkType(nl->First(args)))  return 3;
  if (MLabel::checkType(nl->First(args)))  return 4; 
  if (MLabels::checkType(nl->First(args))) return 5;
  if (MPlace::checkType(nl->First(args)))  return 6; 
  if (MPlaces::checkType(nl->First(args))) return 7;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int distanceVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  T *second = static_cast<T*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcReal *res = static_cast<CcReal*>(result.addr);
  if (first->IsDefined() && second->IsDefined()) {
    res->Set(true, 0);
    //res->Set(true, first->Distance(*second));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct distanceInfo : OperatorInfo {
  distanceInfo() {
    name      = "distance";
    signature = "T x T -> bool,   where T in {(m)label(s), (m)place(s)}";
    syntax    = "distance(_ , _);";
    meaning   = "Computes a Levenshtein-based distance between the objects. The"
                " result is normalized to [0,1] for each of the data types.";
  }
};

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
      m->MergeAdd(unit);
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
                " time intervals may overlap. Undefined units are allowed and "
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
  src->Atperiods(*per, *res);
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
    return NList(PatPersistent::BasicType()).listExpr();
  }
  return NList::typeError("Expecting a text!");
}

/*
\subsection{Value Mapping}

*/
int topatternVM(Word* args, Word& result, int message, Word& local,
                Supplier s) {
  FText* pText = static_cast<FText*>(args[0].addr);
  result = qp->ResultStorage(s);
  PatPersistent* p = static_cast<PatPersistent*>(result.addr);
  Pattern* pattern = 0;
  if (pText->IsDefined()) {
    pattern = Pattern::getPattern(pText->toText(), false);
  }
  else {
    cout << "undefined text" << endl;
    return 0;
  }
  if (pattern) {
    p->Set(true, pText->toText());
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
    signature = " Text -> " + PatPersistent::BasicType();
    syntax    = "_ topattern";
    meaning   = "Creates a Pattern from a Text.";
  }
};

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
  ListExpr mtype = nl->First(nl->First(args));
  ListExpr ptype = nl->First(nl->Second(args));
  if ((!MLabel::checkType(mtype) && !MPlace::checkType(mtype) &&
       !MLabels::checkType(mtype) && !MPlaces::checkType(mtype)) ||
      (!FText::checkType(ptype) && !PatPersistent::checkType(ptype))) {
    return NList::typeError("Expecting mlabel(s)/mplace(s) x text/pattern");
  }
  string query = nl->ToString(nl->Second(nl->Second(args)));
  Word res;
  bool isConst = QueryProcessor::ExecuteQuery(query, res);
  if (isConst) {
    if (FText::checkType(nl->First(nl->Second(args)))) {
      ((FText*)res.addr)->DeleteIfAllowed();
    } 
    else {
      delete (PatPersistent*)res.addr;
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->OneElemList(nl->BoolAtom(isConst)),
                           nl->SymbolAtom(CcBool::BasicType()));
}

/*
\subsection{Selection Function}

*/
int matchesSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args))) ? 4 : 0;
  }
  if (MPlace::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args))) ? 5 : 1;
  }
  if (MLabels::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args))) ? 6 : 2;
  }
  if (MPlaces::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args))) ? 7 : 3;
  }
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class M, class P>
int matchesVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  M *traj = static_cast<M*>(args[0].addr);
  P* pText = static_cast<P*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* b = static_cast<CcBool*>(result.addr);
  Pattern *p = 0;
  if (message != CLOSE) {
    if ((static_cast<CcBool*>(args[2].addr))->GetValue()) { //2nd argument const
      if (!local.addr) {
        if (pText->IsDefined() && traj->IsDefined()) {
          local.addr = Pattern::getPattern(pText->toText(), false);
        }
        else {
          cout << "Undefined pattern text or trajectory." << endl;
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
        ExtBool res = p->matches(traj);
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
        p = Pattern::getPattern(pText->toText(), false);
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
        ExtBool res = p->matches(traj);
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
  "( <text> {mlabel(s)|mplace(s)} x {pattern|text} -> bool </text--->"
  "<text> M matches P </text--->"
  "<text> Checks whether the trajectory M matches the pattern P.\n"
  "<text> query mlabel1 matches '(_ \"Eving\") *' </text--->) )";

ValueMapping matchesVMs[] = {matchesVM<MLabel, FText>, matchesVM<MPlace, FText>,
  matchesVM<MLabels, FText>, matchesVM<MPlaces, FText>, 
  matchesVM<MLabel, PatPersistent>, matchesVM<MPlace, PatPersistent>,
  matchesVM<MLabels, PatPersistent>, matchesVM<MPlaces, PatPersistent>};

Operator matches("matches", matchesSpec, 8, matchesVMs, matchesSelect,
                 matchesTM);

/*
\section{Operator ~tmatches~}

\subsection{Type Mapping}

*/
ListExpr tmatchesTM(ListExpr args) {
  string err = "the expected syntax is: tuple(X) x attrname x (text | pattern)";
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if (!nl->HasLength(nl->First(args), 2)) {
    return listutils::typeError(err + " (first argument is not a tuple)");
  }
  if (!Tuple::checkType(nl->First(nl->First(args))) || 
      !listutils::isSymbol(nl->First(nl->Second(args))) ||
     (!FText::checkType(nl->First(nl->Third(args))) && 
      !PatPersistent::checkType(nl->First(nl->Third(args))))) {
    return listutils::typeError(err);
  }
  cout << nl->ToString(nl->Second(nl->First(nl->First(args)))) << endl;
  ListExpr attrs = nl->Second(nl->First(nl->First(args)));
  string name = nl->SymbolValue(nl->First(nl->Second(args)));
  ListExpr type;
  int index = listutils::findAttribute(attrs, name, type);
  if (!index) {
    return listutils::typeError("Attribute " + name + " not found in tuple.");
  }
  if (!MLabel::checkType(type) && !MLabels::checkType(type) &&
      !MPlace::checkType(type) && !MPlaces::checkType(type)) {
    return listutils::typeError("Attribute " + name + " is not a symbolic "
                               "trajectory (MLabel, MLabels, MPlace, MPlaces)");
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->IntAtom(index - 1)), 
                           nl->SymbolAtom(CcBool::BasicType()));
}

/*
\subsection{Selection Function}

*/
int tmatchesSelect(ListExpr args) {
  return FText::checkType(nl->Third(args)) ? 0 : 1;
}

/*
\subsection{Value Mapping}

*/
template<class P>
int tmatchesVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  Tuple *tuple = static_cast<Tuple*>(args[0].addr);
  CcInt *attrno = static_cast<CcInt*>(args[3].addr);
  P* pat = static_cast<P*>(args[2].addr);
  Pattern *p = 0;
  CcBool* res = static_cast<CcBool*>(result.addr);
  res->SetDefined(false);
  if (pat->IsDefined() && attrno->IsDefined()) {
    p = Pattern::getPattern(pat->toText(), false, tuple);
    if (p) {
      ExtBool result = p->tmatches(tuple, attrno->GetIntval());
      if (result == TRUE) {
        res->Set(true, true); 
      }
      else if (result == FALSE) {
        res->Set(true, false);  
      }
      delete p;
    }
    else {
      cout << "invalid pattern" << endl;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
const string tmatchesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> tuple(X) x attrname x (pattern | text) -> bool </text--->"
  "<text> t matches [attr, p] </text--->"
  "<text> Checks whether the moving type attributes of the tuple match the\n"
  "pattern. The given attribute name has to be the name of a symbolic\n"
  "trajectory attribute. It is treated as a master attribute."
  "<text>query Part feed filter[. matches [ML, "
  "'* (_ _ superset{\"BKA\"}) *']] count </text--->) )";

ValueMapping tmatchesVMs[] = {tmatchesVM<FText>, tmatchesVM<PatPersistent>};

Operator tmatches("tmatches", tmatchesSpec, 2, tmatchesVMs, tmatchesSelect,
                  tmatchesTM);

/*
\section{Operator ~createunitrtree~}

\subsection{Type Mapping}

*/
ListExpr createunitrtreeTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Exactly 2 arguments expected.");
  }
  if (!listutils::isSymbol(nl->Second(args))) {
    return listutils::typeError("Attribute name expected as 2nd argument.");
  }
  if (!listutils::isTupleStream(nl->First(args))) {
    return listutils::typeError("Tuple stream expected as 1st argument.");
  }
  string attrName = nl->SymbolValue(nl->Second(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr attrType;
  int attrIndex = listutils::findAttribute(attrList, attrName, attrType);
  if (attrIndex <= 0) {
    return listutils::typeError("Specified attribute name does not occur in the"
                                " tuple stream.");
  }
  if (!Tools::isSymbolicType(attrType)) { // TODO: add all moving types
    return listutils::typeError(nl->ToString(attrType) + " is not a valid "
                                "attribute type.");
  }
  ListExpr first;
  ListExpr rest = attrList;
  int j(1), tidIndex(0);
  while (!nl->IsEmpty(rest)) {
    first = nl->First(rest);
    rest = nl->Rest(rest);
    if (nl->SymbolValue(nl->Second(first)) == TupleIdentifier::BasicType()) {
      if (tidIndex != 0) {
        return listutils::typeError("Exactly one tuple identifier attribute "
                                    "expected within the tuple stream.");
      }
      tidIndex = j;
    }
    j++;
  }
  if (tidIndex <= 0) {
    return listutils::typeError("Exactly one tuple identifier attribute "
                                "expected within the tuple stream.");
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->TwoElemList(nl->IntAtom(attrIndex),
                                           nl->IntAtom(tidIndex)),
                           nl->FourElemList(
                 nl->SymbolAtom(R_Tree<1, NewPair<TupleId, int> >::BasicType()),
                                            nl->Second(nl->First(args)),
                                            attrType,
                                            nl->BoolAtom(false)));
}

/*
\subsection{Selection Function}

*/
int createunitrtreeSelect(ListExpr args) {
  ListExpr tList = nl->First(nl->Rest(nl->First(args)));
  ListExpr aList = nl->Second(tList);
  string aName = nl->SymbolValue(nl->Second(args));
  ListExpr aType;
  listutils::findAttribute(aList, aName, aType);
  if (MLabel::checkType(aType))  return 0;
  if (MLabels::checkType(aType)) return 1;
  if (MPlace::checkType(aType))  return 2;
  if (MPlaces::checkType(aType)) return 3;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class M>
int createunitrtreeVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  Word wTuple;
  R_Tree<1, NewPair<TupleId, int> > *rtree =
    (R_Tree<1, NewPair<TupleId, int> >*)qp->ResultStorage(s).addr;
  result.setAddr(rtree);
  if (!rtree->InitializeBulkLoad()) {
    cout << "R-tree not initialized for bulk load" << endl;
    return 0;
  }
  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;
  int tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr)) {
    Tuple *tuple = (Tuple*)wTuple.addr;
    M *traj = (M*)tuple->GetAttribute(attrIndex);
    TupleIdentifier *tid = (TupleIdentifier *)tuple->GetAttribute(tidIndex);
    double start[1], end[1];
    if (traj->IsDefined() && tid->IsDefined()) {
      for (int i = 0; i < traj->GetNoComponents(); i++) {
        SecInterval timeIv(true);
        traj->GetInterval(i, timeIv);
        start[0] = timeIv.start.ToDouble();
        end[0] = timeIv.end.ToDouble();
        Rectangle<1> doubleIv(true, start, end);
        if (doubleIv.IsDefined()) {
          NewPair<TupleId, int> position(tid->GetTid(), i);
          R_TreeLeafEntry<1, NewPair<TupleId, int> > entry(doubleIv, position);
          rtree->InsertBulkLoad(entry);
        }
      }
    }
    deleteIfAllowed(tuple);
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);
  if (!rtree->FinalizeBulkLoad()) {
    cout << "bulk load not finalized" << endl;
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createunitrtreeInfo : OperatorInfo {
  createunitrtreeInfo() {
    name      = "createunitrtree";
    signature = "stream(tuple(X)) x IDENT -> rtree";
    syntax    = "_ createunitrtree [ _ ]";
    meaning   = "Creates an rtree from a tuple stream having a moving type "
                "attribute Each unit in each tuple is indexed separately.";
  }
};

/*
\section{Operator ~indexmatches~}

\subsection{Type Mapping}

*/
ListExpr indexmatchesTM(ListExpr args) {
  const string errMsg = "Expecting a relation, the name of a mT (T in {label(s)"
    ", place(s)}) attribute of that relation, an invfile, an rtree, and a "
    "pattern/text";
  if (nl->HasLength(args, 5)) {
    if (FText::checkType(nl->Fifth(args)) || 
        PatPersistent::checkType(nl->Fifth(args))) {
      if (Relation::checkType(nl->First(args))) {
        ListExpr tList = nl->First(nl->Rest(nl->First(args)));
        if (Tuple::checkType(tList) && listutils::isSymbol(nl->Second(args))) {
          ListExpr aType;
          ListExpr aList = nl->Second(tList);
          string aName = nl->SymbolValue(nl->Second(args));
          int i = listutils::findAttribute(aList, aName, aType);
          if (i == 0) {
            return listutils::typeError(aName + " not found");
          }
          if (!MLabel::checkType(aType) && !MPlace::checkType(aType) &&
              !MLabels::checkType(aType) && !MPlaces::checkType(aType)) {
            return listutils::typeError
                   ("type " + nl->ToString(aType) + " is invalid");
          }
          if (InvertedFile::checkType(nl->Third(args)) &&
              R_Tree<1, TwoLayerLeafInfo>::checkType(nl->Fourth(args))) {
            if (Tools::isSymbolicType(nl->Third(nl->Fourth(args)))) {
              return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(i - 1)),
                nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), tList));
            }
            return listutils::typeError("invalid rtree type " + 
                                     nl->ToString(nl->Third(nl->Fourth(args))));
          }
        }
      }
    }
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int indexmatchesSelect(ListExpr args) {
  ListExpr tList = nl->First(nl->Rest(nl->First(args)));
  ListExpr aList = nl->Second(tList);
  string aName = nl->SymbolValue(nl->Second(args));
  ListExpr aType;
  listutils::findAttribute(aList, aName, aType);
  if (MLabel::checkType(aType)) {
    return (FText::checkType(nl->Fourth(args)) ? 0 : 4);
  }
  if (MLabels::checkType(aType)) {
    return (FText::checkType(nl->Fourth(args)) ? 1 : 5);
  }
  if (MPlace::checkType(aType)) {
    return (FText::checkType(nl->Fourth(args)) ? 2 : 6);
  }
  if (MPlaces::checkType(aType)) {
    return (FText::checkType(nl->Fourth(args)) ? 3 : 7);
  }
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class M, class P>
int indexmatchesVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  IndexMatchesLI *li = (IndexMatchesLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      P *pText = static_cast<P*>(args[4].addr);
      CcInt *attr = static_cast<CcInt*>(args[5].addr);
      InvertedFile *inv = static_cast<InvertedFile*>(args[2].addr);
      R_Tree<1, NewPair<TupleId, int> > *rt = 
                  static_cast<R_Tree<1, NewPair<TupleId, int> >*>(args[3].addr);
      Relation *rel = static_cast<Relation*>(args[0].addr);
      if (pText->IsDefined() && attr->IsDefined()) {
        Pattern *p = Pattern::getPattern(pText->toText(), false);
        if (p) {
          if (p->isValid(M::BasicType())) {
            local.addr = new IndexMatchesLI(rel, inv, rt, attr->GetIntval(), p,
                                      true, Tools::getDataType(M::BasicType()));
          }
          else {
            local.addr = 0;
          }
        }
        else {
          local.addr = 0;
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
     (!FText::checkType(nl->Third(args)) && 
      !PatPersistent::checkType(nl->Third(args)))) {
    return listutils::typeError(err);
  }
  string name = nl->SymbolValue(anlist);
  ListExpr type;
  int index = listutils::findAttribute(nl->Second(nl->Second(stream)),
                                       name, type);
  if (!index) {
    return listutils::typeError("attribute " + name + " not found in tuple");
  }
  if (!MLabel::checkType(type) && !MLabels::checkType(type) &&
      !MPlace::checkType(type) && !MPlaces::checkType(type)) {
    return listutils::typeError("wrong type " + nl->ToString(type)
                                + " of attritube " + name);
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->IntAtom(index - 1)), stream);
}

/*
\subsection{Selection Function}

*/
int filtermatchesSelect(ListExpr args) {
  ListExpr stream = nl->First(args);
  ListExpr anlist = nl->Second(args);
  ListExpr pattern = nl->Third(args);
  string name = nl->SymbolValue(anlist);
  ListExpr type;
  listutils::findAttribute(nl->Second(nl->Second(stream)),
                                       name, type);
  if (MLabel::checkType(type)) return (FText::checkType(pattern) ? 0 : 4);
  if (MLabels::checkType(type)) return (FText::checkType(pattern) ? 1 : 5);
  if (MPlace::checkType(type)) return (FText::checkType(pattern) ? 2 : 6);
  if (MPlaces::checkType(type)) return (FText::checkType(pattern) ? 3 : 7);
  return -1;
}

/*
\subsection{Value Mapping for a Text}

*/
template<class M, class T>
int filtermatchesVM(Word* args, Word& result, int message, Word& local, 
                    Supplier s) {
  FilterMatchesLI<M>* li = (FilterMatchesLI<M>*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      CcInt* ccint = (CcInt*)args[3].addr;
      T* pText = (T*)args[2].addr;
      if (pText->IsDefined() && ccint->IsDefined()) {
        string text = pText->toText();
        local.addr = new FilterMatchesLI<M>(args[0], ccint->GetValue(), text);
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
  return 0;
}

/*
\subsection{Operator Info}

*/
struct filtermatchesInfo : OperatorInfo {
  filtermatchesInfo() {
    name      = "filtermatches";
    signature = "stream(tuple(X)) x IDENT x text -> stream(tuple(X))";
    syntax    = "_ filtermatches [ _ , _ ]";
    meaning   = "Filters a stream containing symbolic trajectories, passing "
                "exactly the tuples whose trajectories match the pattern on "
                "to the output stream.";
  }
};


/*
\section{Operator ~rewrite~}

\subsection{Type Mapping}

*/
ListExpr rewriteTM(ListExpr args) {
  const string errMsg = "Expecting an mT (T in {label(s), place(s)} and a "
                        "pattern/text";
  if (nl->HasLength(args, 2)) {
    ListExpr mtype = nl->First(args);
    if ((MLabel::checkType(mtype) || MLabels::checkType(mtype) ||
         MPlace::checkType(mtype) || MPlaces::checkType(mtype)) &&
        (FText::checkType(nl->Second(args)) ||
         PatPersistent::checkType(nl->Second(args)))) {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                             mtype);
    }
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int rewriteSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args)) ? 4 : 0);
  }
  if (MLabels::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args)) ? 5 : 1);
  }
  if (MPlace::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args)) ? 6 : 2);
  }
  if (MPlaces::checkType(nl->First(args))) {
    return (PatPersistent::checkType(nl->Second(args)) ? 7 : 3);
  }
  return -1;
}

/*
\subsection{Value Mapping (for a text)}

*/
template<class M, class T>
int rewriteVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  M *source = 0;
  T* pText = 0;
  Pattern *p = 0;
  RewriteLI<M> *rewriteLI = 0;
  switch (message) {
    case OPEN: {
      source = static_cast<M*>(args[0].addr);
      pText = static_cast<T*>(args[1].addr);
      if (!pText->IsDefined()) {
        cout << "Error: undefined pattern text." << endl;
        return 0;
      }
      if (!source->IsDefined()) {
        cout << "Error: undefined mlabel." << endl;
        return 0;
      }
      p = Pattern::getPattern(pText->toText(), false);
      if (!p) {
        cout << "Error: pattern not initialized." << endl;
      }
      else {
        if (!p->isValid(M::BasicType())) {
          cout << "Pattern not suitable for type " << M::BasicType() << endl;
        }
        else if (!p->hasAssigns()) {
          cout << "No result specified." << endl;
        }
        else {
          if (p->initAssignOpTrees() && p->initEasyCondOpTrees()) {
            rewriteLI = new RewriteLI<M>(source, p);
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
      rewriteLI = ((RewriteLI<M>*)local.addr);
      result.addr = rewriteLI->getNextResult();
      return (result.addr ? YIELD : CANCEL);
    }
    case CLOSE: {
      if (local.addr) {
        rewriteLI = ((RewriteLI<M>*)local.addr);
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
\subsection{Operator Info}

*/
struct rewriteInfo : OperatorInfo {
  rewriteInfo() {
    name      = "rewrite";
    signature = "mT x P -> stream(mT),   where T in {label(s), place(s)}, "
                "P in {pattern, text}";
    syntax    = "rewrite (_, _)";
    meaning   = "Rewrite a symbolic trajectory according to a rewrite rule.";
  }
};

/*
\section{Operator ~multirewrite~}

\subsection{Type Mapping}

*/
ListExpr multirewriteTM(ListExpr args) {
  if (nl->HasLength(args, 3)) {
    if (!Stream<Tuple>::checkType(nl->First(args))) {
      return NList::typeError("First argument must be a tuple stream.");
    }
    if (!listutils::isSymbol(nl->Second(args))) {
      return NList::typeError("Second argument must be an attribute name.");
    }
    if (!Stream<FText>::checkType(nl->Third(args))) {
      return NList::typeError("Third argument must be a text stream.");
    }
    string attrname = nl->SymbolValue(nl->Second(args));
    ListExpr attrlist = nl->Second(nl->Second(nl->First(args)));
    ListExpr attrtype;
    int index = listutils::findAttribute(attrlist, attrname, attrtype);
    if (!index) {
      return listutils::typeError("Attribute " + attrname + " not found.");
    }
    if (!MLabel::checkType(attrtype) && !MLabels::checkType(attrtype) &&
        !MPlace::checkType(attrtype) && !MPlaces::checkType(attrtype)) {
      return listutils::typeError("Wrong type " + nl->ToString(attrtype)
                                  + " of attritube " + attrname + ".");
    }
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->IntAtom(index - 1)),
                             nl->TwoElemList(nl->SymbolAtom(
                                                Stream<Attribute>::BasicType()),
                                             attrtype));
  }
  return listutils::typeError("Three arguments expected.");
}

/*
\subsection{Selection Function}

*/
int multirewriteSelect(ListExpr args) {
  ListExpr stream = nl->First(args);
  string attrname = nl->SymbolValue(nl->Second(args));
  ListExpr attrtype;
  listutils::findAttribute(nl->Second(nl->Second(stream)),attrname, attrtype);
  if (MLabel::checkType(attrtype))  return 0;
  if (MLabels::checkType(attrtype)) return 1;
  if (MPlace::checkType(attrtype))  return 2;
  if (MPlaces::checkType(attrtype)) return 3;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class M>
int multirewriteVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  MultiRewriteLI<M> *li = (MultiRewriteLI<M>*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      int attrpos = (static_cast<CcInt*>(args[3].addr))->GetIntval();
      local.addr = new MultiRewriteLI<M>(args[0], args[2], attrpos);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextResult() : 0;
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
struct multirewriteInfo : OperatorInfo {
  multirewriteInfo() {
    name      = "multirewrite";
    signature = "stream(tuple(..., mT, ...)) x IDENT x stream(text) -> "
                "stream(mT), where T in {label(s), place(s)}";
    syntax    = "_ rewrite [ _ , _ ]";
    meaning   = "Rewrite a stream of symbolic trajectories.";
  }
};

/*
\section{Operator ~classify~}

\subsection{Type Mapping}

*/
ListExpr classifyTM(ListExpr args) {
  const string errMsg = "Expecting an mT (T in {label(s), place(s)} and a "
                        "classifier.";
  if (nl->HasLength(args, 2)) {
    if (Tools::isSymbolicType(nl->First(args))
     && Classifier::checkType(nl->Second(args))) {
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(FText::BasicType()));
    }
  }
  return listutils::typeError(errMsg);
}

/*
\subsection{Selection Function}

*/
int classifySelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args)))  return 0;
  if (MLabels::checkType(nl->First(args))) return 1;
  if (MPlace::checkType(nl->First(args)))  return 2;
  if (MPlaces::checkType(nl->First(args))) return 3;
  return -1;
}

/*
\subsection{Value Mapping without index}

*/
template<class M>
int classifyVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  ClassifyLI *li = (ClassifyLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      M* source = static_cast<M*>(args[0].addr);
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
    signature = "mT (T in {label(s), place(s)}) x classifier -> stream(text)";
    syntax    = "classify(_ , _)";
    meaning   = "Classifies a trajectory according to a classifier";
  }
};

/*
\section{Operator ~indexclassify~}

\subsection{Type Mapping}

*/
ListExpr indexclassifyTM(ListExpr args) {
  const string errMsg = "Expecting a relation, the name of an mT (T in "
      "{label(s), place(s)}) attribute, an invfile, an rtree, and a classifier";
  if (nl->HasLength(args, 5)) {
    if (Classifier::checkType(nl->Fifth(args))) {
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
          if (!MLabel::checkType(attrType) && !MLabels::checkType(attrType) &&
              !MPlace::checkType(attrType) && !MPlaces::checkType(attrType)) {
            return listutils::typeError
                   ("Type " + nl->ToString(attrType) + " is invalid");
          }
          if (InvertedFile::checkType(nl->Third(args)) &&
              R_Tree<1, TwoLayerLeafInfo>::checkType(nl->Fourth(args))) {
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
\subsection{Selection Function}

*/
int indexclassifySelect(ListExpr args) {
  ListExpr attrList = nl->Second(nl->First(nl->Rest(nl->First(args))));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->Second(args));
  listutils::findAttribute(attrList, attrName, attrType);
  if (MLabel::checkType(attrType)) return 0;
  if (MLabels::checkType(attrType)) return 1;
  if (MPlace::checkType(attrType)) return 2;
  if (MPlaces::checkType(attrType)) return 3;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class M>
int indexclassifyVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  IndexClassifyLI *li = (IndexClassifyLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      InvertedFile *inv = static_cast<InvertedFile*>(args[2].addr);
      R_Tree<1, NewPair<TupleId, int> > *rt = 
                  static_cast<R_Tree<1, NewPair<TupleId, int> >*>(args[3].addr);
      CcInt *attr = static_cast<CcInt*>(args[5].addr);
      Relation *rel = static_cast<Relation*>(args[0].addr);
      if (!attr->IsDefined()) {
        cout << "undefined parameter(s)" << endl;
        local.addr = 0;
        return 0;
      }
      local.addr = new IndexClassifyLI(rel, inv, rt, args[4], attr->GetIntval(),
                                       Tools::getDataType(M::BasicType()));
      return 0;
    }
    case REQUEST: {
      cout << "REQUEST next tuple" << endl;
      result.addr = li ? li->nextResultTuple<M>() : 0;
      cout << "return " << (result.addr ? "TUPLE" : "NULL") << endl;
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
    signature = "rel(tuple(..., mT, ...)) x attrname x invfile x rtree x "
                "classifier -> stream(tuple(string, mT))";
    syntax    = "_ indexclassify [_ , _ , _ , _]";
    meaning   = "Classifies an indexed relation of trajectories according to a "
                " classifier";
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
  const string errMsg = "Expecting mlabel or stream(mlabel).";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(errMsg);
  }
  ListExpr arg = nl->First(args);
  if (MLabel::checkType(arg) || Stream<MLabel>::checkType(arg) ||
     MLabels::checkType(arg) || Stream<MLabels>::checkType(arg) ||
     MPlace::checkType(arg) || Stream<MPlace>::checkType(arg) ||
     MPlaces::checkType(arg) || Stream<MPlaces>::checkType(arg)) {
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
  if (Stream<MLabel>::checkType(arg)) return 4;
  if (Stream<MLabels>::checkType(arg)) return 5;
  if (Stream<MPlace>::checkType(arg)) return 6;
  if (Stream<MPlaces>::checkType(arg)) return 7;
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
  source->Compress(*res);
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
        T* source =(T*) arg.addr;
        T* res = new T(true);
        source->Compress(*res);
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
    signature = "mT -> mT,   where T in {label(s), place(s)}";
    appendSignature("stream(mT) -> stream(mT),   where T in {label(s), "
                    "place(s)}");
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


/*
\section{operator ~createml~}

\subsection{Type Mapping}

*/
ListExpr createmlTM(ListExpr args) {
  const string errMsg = "Expecting an integer and a bool.";
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  if (CcInt::checkType(nl->First(args)) && 
      CcBool::checkType(nl->Second(args))) {
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
  CcBool* ccbool = static_cast<CcBool*>(args[1].addr);
  MLabel* ml = static_cast<MLabel*>(result.addr);
  ml->SetDefined(false);
  if (!ccint->IsDefined() || !ccbool->IsDefined()) {
    cout << "undefined value" << endl;
    return 0;
  }
  map<string, set<string> > transitions;
  if (!Tools::createTransitions(ccbool->GetValue(), transitions)) {
    return 0;
  }
  vector<string> labels;
  if (!Tools::createLabelSequence(ccint->GetValue(), 1, ccbool->GetValue(),
                                  transitions, labels)) {
    return 0;
  }
  ml->SetDefined(true);
  ml->createML(ccint->GetValue(), 0, labels);
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createmlInfo : OperatorInfo {
  createmlInfo() {
    name      = "createml";
    signature = "int x bool -> mlabel";
    syntax    = "createml(_,_)";
    meaning   = "Creates an MLabel, representing a trip either through "
                "Dortmund's districts (iff the second parameter is true) or "
                " Germany's counties. The first parameter determines the size.";
  }
};

/*
\section{Operator ~createmlrel~}

\subsection{Type Mapping}

*/
ListExpr createmlrelTM(ListExpr args) {
  if (nl->ListLength(args) != 4) {
    return listutils::typeError("Four arguments expected.");
  }
  if (nl->IsEqual(nl->First(args), CcInt::BasicType())
   && nl->IsEqual(nl->Second(args), CcInt::BasicType())
   && nl->IsEqual(nl->Third(args), CcString::BasicType())
   && nl->IsEqual(nl->Fourth(args), CcBool::BasicType())) {
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return NList::typeError("Expecting two integers, a string and a bool.");
}

/*
\subsection{Value Mapping}

*/
int createmlrelVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  CcInt* ccint1 = static_cast<CcInt*>(args[0].addr);
  CcInt* ccint2 = static_cast<CcInt*>(args[1].addr);
  CcString* ccstring = static_cast<CcString*>(args[2].addr);
  CcBool* ccbool = static_cast<CcBool*>(args[3].addr);
  int number, size;
  string relName, errMsg;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  if (ccstring->IsDefined() && ccint1->IsDefined() && ccint2->IsDefined() &&
      ccbool->IsDefined()) {
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    relName = ccstring->GetValue();
    if (!sc->IsValidIdentifier(relName, errMsg, true)) { // check relation name
      cout << "Invalid relation name \"" << relName << "\"; " << errMsg << endl;
      res->Set(true, false);
      return 0;
    }
    if (sc->IsObjectName(relName)) {
      cout << relName << " is an existing DB object" << endl;
      return 0;
    }
    if (sc->IsSystemObject(relName)) {
      cout << relName << " is a reserved name" << endl;
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
    map<string, set<string> > transitions;
    if (!Tools::createTransitions(ccbool->GetValue(), transitions)) {
      return 0;
    }
    vector<string> labels;
    if (!Tools::createLabelSequence(size, number, ccbool->GetValue(),
                                    transitions, labels)) {
      return 0;
    }
    for (int i = 0; i < number; i++) {
      tuple = new Tuple(type);
      ml = new MLabel(1);
      ml->createML(size, i, labels);
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
struct createmlrelInfo : OperatorInfo {
  createmlrelInfo() {
    name      = "createmlrel";
    signature = "int x int x string x bool -> bool";
    syntax    = "createmlrelation(_ , _ , _ , _)";
    meaning   = "Creates a relation containing arbitrary many synthetic moving"
                "labels of arbitrary size and stores it into the database. If "
                "the boolean parameter is true, a trip through Dortmund's "
                "districts is simulated (12 different labels); otherwise the "
                "trip is based on German counties (439 different labels).";
  }
};

/*
\section{Operator ~createtrie~}

createtrie: rel(tuple(..., mT, ...)) -> invfile, where T in {label(s), place(s)}

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
      if (i > 0) { // found
        if (MLabel::checkType(attrType) || MLabels::checkType(attrType) ||
            MPlace::checkType(attrType) || MPlaces::checkType(attrType)) {
          return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                  nl->OneElemList(nl->IntAtom(i)),
                                  nl->SymbolAtom(InvertedFile::BasicType()));
        }
      }
    }
  }
  return listutils::typeError("Argument types must be rel(tuple(..., mT, ...)) "
                              "x attrname,   where T in {label(s), place(s)}");
}

/*
\subsection{Selection Function}

*/
int createtrieSelect(ListExpr args) {
  ListExpr attrList = nl->First(nl->Rest(nl->First(nl->Rest(nl->First(args)))));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->Second(args));
  listutils::findAttribute(attrList, attrName, attrType);
  if (MLabel::checkType(attrType)) return 0;
  if (MLabels::checkType(attrType)) return 1;
  if (MPlace::checkType(attrType)) return 2;
  if (MPlaces::checkType(attrType)) return 3;
  return -1; // cannot occur
}

/*
\subsection{Value Mapping}

*/
template<class M>
int createtrieVM(Word* args, Word& result, int message, Word& local,Supplier s){
  Relation *rel = (Relation*)(args[0].addr);
  Tuple *tuple = 0;
  M *src = 0;
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
  unsigned int counter = 0;
  if (M::BasicType() == "mlabel") {
    for (int i = 0; i < rel->GetNoTuples(); i++) {
      tuple = rel->GetTuple(i + 1, false);
      src = (M*)(tuple->GetAttribute(((CcInt*)args[2].addr)->GetIntval() - 1));
      string value;
      for (int j = 0; j < src->GetNoComponents(); j++) {
        ((MLabel*)src)->GetValue(j, value);
        inv->insertString(tuple->GetTupleId(), value, j, 0, cache, trieCache);
      }
      tuple->DeleteIfAllowed();
    }
  }
  else if (M::BasicType() == "mplace") {
    for (int i = 0; i < rel->GetNoTuples(); i++) {
      tuple = rel->GetTuple(i + 1, false);
      src = (M*)(tuple->GetAttribute(((CcInt*)args[2].addr)->GetIntval() - 1));
      pair<string, charPosType> value;
      for (int j = 0; j < src->GetNoComponents(); j++) {
        ((MPlace*)src)->GetValue(j, value);
        inv->insertString(tuple->GetTupleId(), value.first, j, value.second, 
                          cache, trieCache);
      }
      tuple->DeleteIfAllowed();
    }
  }
  else if (M::BasicType() == "mlabels") {
    for (int i = 0; i < rel->GetNoTuples(); i++) {
      tuple = rel->GetTuple(i + 1, false);
      src = (M*)(tuple->GetAttribute(((CcInt*)args[2].addr)->GetIntval() - 1));
      set<string> values;
      for (int j = 0; j < src->GetNoComponents(); j++) {
        ((MLabels*)src)->GetValues(j, values);
        for (set<string>::iterator it = values.begin();it != values.end();it++){
          inv->insertString(tuple->GetTupleId(), *it, j, 0, cache, trieCache);
          counter++;
          if (counter % 100000 == 0) {
            cout << counter << " elements inserted, last was " << *it << endl;
          }
        }
      }
      tuple->DeleteIfAllowed();
    }
  }
  else { // mplaces
    for (int i = 0; i < rel->GetNoTuples(); i++) {
      tuple = rel->GetTuple(i + 1, false);
      src = (M*)(tuple->GetAttribute(((CcInt*)args[2].addr)->GetIntval() - 1));
      set<pair<string, charPosType> > values;
      for (int j = 0; j < src->GetNoComponents(); j++) {
        ((MPlaces*)src)->GetValues(j, values);
        for (set<pair<string, charPosType> >::iterator it = values.begin();
                                                     it != values.end(); it++) {
          inv->insertString(tuple->GetTupleId(), it->first, j, it->second, 
                            cache, trieCache);
          counter++;
          if (counter % 100000 == 0) {
            cout << counter << " elements inserted, last was (" << it->first
                 << ", " << it->second << ")" << endl;
          }
        }
      }
      tuple->DeleteIfAllowed();
    }
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
    signature = "rel(tuple(..., mT, ...)) x attrname -> invfile,   where T in "
                "{label(s), place(s)}";
    syntax    = "_ createtrie [ _ ]";
    meaning   = "Builds an index for a relation of numbered symbolic "
                "trajectories.";
  }
};

/*
\section{Operator ~derivegroups~}

\subsection{Type Mapping}

*/
ListExpr derivegroupsTM(ListExpr args) {
  if (nl->ListLength(args) != 3) {
    return listutils::typeError("Three arguments expected.");
  }
  if (Stream<Tuple>::checkType(nl->First(args))) {
    if (Tuple::checkType(nl->First(nl->Rest(nl->First(args))))) {
      ListExpr attrList =
               nl->First(nl->Rest(nl->First(nl->Rest(nl->First(args)))));
      ListExpr attrType;
      string attrName = nl->SymbolValue(nl->Second(args));
      int i = listutils::findAttribute(attrList, attrName, attrType);
      if (i > 0) { // found
        if (MLabel::checkType(attrType) || MLabels::checkType(attrType) ||
            MPlace::checkType(attrType) || MPlaces::checkType(attrType)) {
          if (CcReal::checkType(nl->Third(args))) {
            ListExpr newAttr = nl->TwoElemList(nl->SymbolAtom("Group"),
                                            nl->SymbolAtom(CcInt::BasicType()));
            ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
            ListExpr rest = nl->Rest(attrList);
            ListExpr last = newAttrList;
            while (rest != nl->Empty()) {
              last = nl->Append(last, nl->First(rest));
              rest = nl->Rest(rest);
            }
            last = nl->Append(last, newAttr);
            ListExpr output = nl->TwoElemList(
                                     nl->SymbolAtom(Stream<Tuple>::BasicType()),
              nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), newAttrList));
            return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                   nl->OneElemList(nl->IntAtom(i - 1)), output);
          }
        }
      }
    }
  }
  return listutils::typeError("Argument types must be stream(tuple(..., mT, "
                  "...)) x attrname x real,   where T in {label(s), place(s)}");
}

/*
\subsection{Selection Function}

*/
int derivegroupsSelect(ListExpr args) {
  ListExpr attrList = nl->First(nl->Rest(nl->First(nl->Rest(nl->First(args)))));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->Second(args));
  listutils::findAttribute(attrList, attrName, attrType);
  if (MLabel::checkType(attrType))   return 0;
  if (MLabels::checkType(attrType))  return 1;
  if (MPlace::checkType(attrType))   return 2;
  if (MPlaces::checkType(attrType))  return 3;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class M>
int derivegroupsVM(Word* args, Word& result, int message, Word& local, 
                   Supplier s) {
  DeriveGroupsLI<M>* li = (DeriveGroupsLI<M>*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      CcReal *threshold = static_cast<CcReal*>(args[2].addr);
      CcInt *attrNo = static_cast<CcInt*>(args[3].addr);
      if (threshold->IsDefined() && attrNo->IsDefined()){
        local.addr = new DeriveGroupsLI<M>(args[0], threshold->GetValue(), 
                                           attrNo->GetValue());
      }
      else {
        cout << "undefined argument(s)" << endl;
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->getNextTuple() : 0;
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
struct derivegroupsInfo : OperatorInfo {
  derivegroupsInfo() {
    name      = "derivegroups";
    signature = "stream(tuple(..., mT, ...)) x attrname x real -> stream(tuple("
                "..., mT, ..., int)),   where T in {label(s), place(s)}";
    syntax    = "_ derivegroups [ _ , _ ]";
    meaning   = "Finds groups of similar symbolic trajectories inside a tuple "
                "stream. The granularity of the groups is specified by a "
                "threshold.";
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
  
  ValueMapping tolabelsVMs[] = {tolabelsVM<FText>, tolabelsVM<CcString>, 0};
  AddOperator(tolabelsInfo(), tolabelsVMs, tolabelsSelect, tolabelsTM);
  
  ValueMapping toplacesVMs[] = {toplacesVM_P, toplacesVM_T, 0};
  AddOperator(toplacesInfo(), toplacesVMs, toplacesSelect, toplacesTM);

  ValueMapping containsVMs[] = {containsSingleVM<Labels, Label>,
    containsSingleVM<Places, Place>, containsMultiVM<Labels, Labels>, 
    containsMultiVM<Places, Places>, 0};
  AddOperator(containsInfo(), containsVMs, containsSelect, containsTM);
  
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
  
  ValueMapping distanceVMs[] = {distanceVM<Label>, distanceVM<Labels>,
    distanceVM<Place>, distanceVM<Places>, distanceVM<MLabel>,
    distanceVM<MLabels>, distanceVM<MPlace>, distanceVM<MPlaces>, 0};
  AddOperator(distanceInfo(), distanceVMs, distanceSelect, distanceTM);

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
    passesSymbolicVM<MPlace, Place>, passesSymbolicVM<MPlaces, Place>,
    passesSymbolicVM<MPlaces, Places>, 0};
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
  
  ValueMapping getunitSymbolicVMs[] = {getunitSymbolicVM<MLabel, ULabel>,
    getunitSymbolicVM<MLabels, ULabels>, getunitSymbolicVM<MPlace, UPlace>, 
    getunitSymbolicVM<MPlaces, UPlaces>, 0};
  AddOperator(getunitSymbolicInfo(), getunitSymbolicVMs, symbolicSimpleSelect,
              getunitSymbolicTM);
  
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
      
  AddOperator(topatternInfo(), topatternVM, topatternTM);

  AddOperator(toclassifierInfo(), toclassifierVM, toclassifierTM);

  AddOperator(&matches);
  matches.SetUsesArgsInTypeMapping();
  
  AddOperator(&tmatches);
  tmatches.SetUsesArgsInTypeMapping();
  
  ValueMapping createunitrtreeVMs[] = {createunitrtreeVM<MLabel>,
    createunitrtreeVM<MLabels>, createunitrtreeVM<MPlace>, 
    createunitrtreeVM<MPlaces>, 0};
  AddOperator(createunitrtreeInfo(), createunitrtreeVMs, createunitrtreeSelect,
              createunitrtreeTM);
  
  ValueMapping indexmatchesVMs[] = {indexmatchesVM<MLabel, FText>, 
    indexmatchesVM<MLabels, FText>, indexmatchesVM<MPlace, FText>, 
    indexmatchesVM<MPlaces, FText>, indexmatchesVM<MLabel, PatPersistent>,
    indexmatchesVM<MLabels, PatPersistent>, 
    indexmatchesVM<MPlace, PatPersistent>,
    indexmatchesVM<MPlaces, PatPersistent>, 0};
  AddOperator(indexmatchesInfo(), indexmatchesVMs, indexmatchesSelect,
              indexmatchesTM);

  ValueMapping filtermatchesVMs[] = {filtermatchesVM<MLabel, FText>,
    filtermatchesVM<MLabels, FText>, filtermatchesVM<MPlace, FText>,
    filtermatchesVM<MPlaces, FText>, filtermatchesVM<MLabel, PatPersistent>,
    filtermatchesVM<MLabels, PatPersistent>, 
    filtermatchesVM<MPlace, PatPersistent>,
    filtermatchesVM<MPlaces, PatPersistent>, 0};
  AddOperator(filtermatchesInfo(), filtermatchesVMs, filtermatchesSelect,
              filtermatchesTM);
  
  ValueMapping rewriteVMs[] = {rewriteVM<MLabel, FText>, 
    rewriteVM<MLabels, FText>, rewriteVM<MPlace, FText>, 
    rewriteVM<MPlaces, FText>, rewriteVM<MLabel, PatPersistent>,
    rewriteVM<MLabels, PatPersistent>, rewriteVM<MPlace, PatPersistent>,
    rewriteVM<MPlaces, PatPersistent>, 0};
  AddOperator(rewriteInfo(), rewriteVMs, rewriteSelect, rewriteTM);
  
  ValueMapping multirewriteVMs[] = {multirewriteVM<MLabel>,
    multirewriteVM<MLabels>, multirewriteVM<MPlace>, multirewriteVM<MPlaces>,0};
  AddOperator(multirewriteInfo(), multirewriteVMs, multirewriteSelect,
              multirewriteTM);

  ValueMapping classifyVMs[] = {classifyVM<MLabel>, classifyVM<MLabels>,
    classifyVM<MPlace>, classifyVM<MPlaces>, 0};
  AddOperator(classifyInfo(), classifyVMs, classifySelect, classifyTM);

  ValueMapping indexclassifyVMs[] = {indexclassifyVM<MLabel>,
    indexclassifyVM<MLabels>, indexclassifyVM<MPlace>, indexclassifyVM<MPlaces>,
    0};
  AddOperator(indexclassifyInfo(), indexclassifyVMs, indexclassifySelect,
              indexclassifyTM);
  
  ValueMapping concatVMs[] = {concatVM<MLabel>, concatVM<MLabels>,
    concatVM<MPlace>, concatVM<MPlaces>, 0};
  AddOperator(concatInfo(), concatVMs, symbolicSimpleSelect, concatTM);

  ValueMapping compressVMs[] = {compressVM_1<MLabel>, compressVM_1<MLabels>,
    compressVM_1<MPlace>, compressVM_1<MPlaces>, compressVM_Str<MLabel>,
    compressVM_Str<MLabels>, compressVM_Str<MPlace>, compressVM_Str<MPlaces>,0};
  AddOperator(compressInfo(), compressVMs, compressSelect, compressTM);

  ValueMapping fillgapsVMs[] = {fillgapsVM_1<MLabel>, fillgapsVM_1<MLabels>,
    fillgapsVM_1<MPlace>, fillgapsVM_1<MPlaces>, fillgapsVM_Str<MLabel>, 
    fillgapsVM_Str<MLabels>, fillgapsVM_Str<MPlace>, fillgapsVM_Str<MPlaces>,0};
  AddOperator(fillgapsInfo(), fillgapsVMs, fillgapsSelect, fillgapsTM);

  AddOperator(createmlInfo(), createmlVM, createmlTM);

  AddOperator(createmlrelInfo(), createmlrelVM, createmlrelTM);

  ValueMapping createtrieVMs[] = {createtrieVM<MLabel>, createtrieVM<MLabels>,
                                createtrieVM<MPlace>, createtrieVM<MPlaces>, 0};
  AddOperator(createtrieInfo(), createtrieVMs, createtrieSelect, createtrieTM);
  
//   ValueMapping derivegroupsVMs[] = {derivegroupsVM<MLabel>,
//     derivegroupsVM<MLabels>, derivegroupsVM<MPlace>, 
//     derivegroupsVM<MPlaces>,0};
//   AddOperator(derivegroupsInfo(), derivegroupsVMs, derivegroupsSelect,
//               derivegroupsTM);
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
