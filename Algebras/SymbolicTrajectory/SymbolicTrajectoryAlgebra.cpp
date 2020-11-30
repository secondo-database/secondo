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

// #include "Algorithms.h"
// #include "RestoreTraj.h"
#include "PatternMining.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;
using namespace temporalalgebra;
using namespace datetime;

namespace stj {
  
/*
\subsection{Type Constructors}

*/
GenTC<PatPersistent> patternTC;



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
    res->Set(true, first->Distance(*second));
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
\section{Operator ~distancesym~}

distance: T x T -> real,   where T in {mplace(s), mlabel(s)}

\subsection{Type Mapping}

*/
ListExpr distancesymTM(ListExpr args) {
  if (nl->ListLength(args) != 3) {
    return NList::typeError("Expecting three arguments.");
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if ((MLabel::checkType(first)  && MLabel::checkType(second))  || 
      (MLabels::checkType(first) && MLabels::checkType(second)) || 
      (MPlace::checkType(first)  && MPlace::checkType(second))  || 
      (MPlaces::checkType(first) && MPlaces::checkType(second))) {
    if (CcString::checkType(nl->Third(args))) {
      return nl->SymbolAtom(CcReal::BasicType());
    }
  }
  return NList::typeError("Expecting T x T x string, where T in {mplace(s), "
                          "mlabel(s)}");
}

/*
\subsection{Selection Function}

*/
int distancesymSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args)))  return 0; 
  if (MLabels::checkType(nl->First(args))) return 1;
  if (MPlace::checkType(nl->First(args)))  return 2; 
  if (MPlaces::checkType(nl->First(args))) return 3;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int distancesymVM(Word* args, Word& result, int message, Word& local, 
                  Supplier s) {
  T *first = static_cast<T*>(args[0].addr);
  T *second = static_cast<T*>(args[1].addr);
  CcString *distfuncc = static_cast<CcString*>(args[2].addr);
  result = qp->ResultStorage(s);
  CcReal *res = static_cast<CcReal*>(result.addr);
  if (first->IsDefined() && second->IsDefined() && distfuncc->IsDefined()) {
    DistanceFunSym distfun = Tools::getDistanceFunSym(distfuncc->GetValue());
    if (distfun != ERROR) {
      res->Set(true, first->DistanceSym(*second, distfun));
    }
    else {
      cout << "\'" + distfuncc->GetValue() + "\' is not a valid distance "
              "function" << endl;
      res->SetDefined(false);
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
struct distancesymInfo : OperatorInfo {
  distancesymInfo() {
    name      = "distancesym";
    signature = "T x T x string -> real,   where T in {mlabel(s), mplace(s)}";
    syntax    = "distance(_ , _ , _);";
    meaning   = "Computes a distance between two symbolic trajectories. "
                "Currently, the following distance functions are available:\n"
                "\"EQUALLABELS\": returns 0 if the sequences of labels are "
                "identical; 1 otherwise\n"
                "\"PREFIX\": returns 0 if the sequences of labels are identical"
                "; 2 if they have no common prefix; 1/p otherwise, where p is "
                "the length of the common prefix\n"
                "\"SUFFIX\": returns 0 if the sequences of labels are identical"
                "; 2 if they have no common suffix; 1/s otherwise, where s is "
                "the length of the common suffix\n"
                "\"PREFIXSUFFIX\": returns 0 if the sequences of labels are "
                "identical; 2 if they have no common prefix and no common "
                "suffix; 1/(p+s) otherwise, where p is the length of the common"
                " prefix and s is the length of the common suffix";
  }
};

/*
\section{Operator ~hybriddistance~}

distance: T x mpoint x T x mpoint -> real,   where T in {mplace, mlabel}

\subsection{Type Mapping}

*/
ListExpr hybriddistanceTM(ListExpr args) {
  if (!nl->HasLength(args, 4)) {
    return NList::typeError("Expecting four arguments.");
  }
  ListExpr first = nl->First(args);
  ListExpr third = nl->Third(args);
  if ((MLabel::checkType(first)  && MLabel::checkType(third)) || 
      (MPlace::checkType(first)  && MPlace::checkType(third)) ||
      (MLabels::checkType(first) && MLabels::checkType(third))) {
    if (MPoint::checkType(nl->Second(args)) && 
        MPoint::checkType(nl->Fourth(args))) {
      return nl->SymbolAtom(CcReal::BasicType());
    }
  }
  return NList::typeError("Expecting T x mpoint x T x mpoint,   "
                          "where T in {mlabel, mplace, mlabels}");
}

/*
\subsection{Selection Function}

*/
int hybriddistanceSelect(ListExpr args) {
  ListExpr first = nl->First(args);
  if (MLabel::checkType(first))  return (nl->HasLength(args, 6) ? 0 : 3);
  if (MPlace::checkType(first))  return (nl->HasLength(args, 6) ? 1 : 4);
  if (MLabels::checkType(first)) return (nl->HasLength(args, 6) ? 2 : 5);
  return -1;
}

/*
\subsection{Instance for storing parameters}

*/
HybridDistanceParameters hdp;

/*
\subsection{Value Mapping}

*/
template<class T, bool hasGeoid>
int hybriddistanceVM(Word* args, Word& result, int message, Word& local, 
                     Supplier s) {
  result = qp->ResultStorage(s);
  T *sym1 = static_cast<T*>(args[0].addr);
  T *sym2 = static_cast<T*>(args[2].addr);
  MPoint *mp1 = static_cast<MPoint*>(args[1].addr);
  MPoint *mp2 = static_cast<MPoint*>(args[3].addr);
  CcReal *res = static_cast<CcReal*>(result.addr);
  if (sym1->IsDefined() && sym2->IsDefined() && mp1->IsDefined() && 
      mp2->IsDefined()) {
    double symdist = sym1->Distance(*sym2, hdp.distFun, hdp.labelFun);
    if (symdist <= hdp.threshold) {
      double geodist = -1.0;
      if (hdp.geoDistFun >= 0 && hdp.geoDistFun < 3) {
        geodist = mp1->DistanceStartEnd(*mp2, hdp.geoDistFun, hdp.geoid) 
                  / hdp.scaleFactor;
      }
      else if (hdp.geoDistFun == 3) {
        geodist = mp1->FrechetDistance(mp2, hdp.geoid) / hdp.scaleFactor;
      }
      if (geodist < 0.0) { // error case
//         cout << "GeoDist NEGATIVE" << endl;
        res->SetDefined(false);
      }
      else if (geodist >= 1.0) {
//         cout << "symDist = " << symdist << " ===> GeoDist set to 1" << endl;
        res->Set(true, 1.0);
      }
      else {
//         cout << "symDist = " << symdist << " ===> GeoDist = " << geodist 
//              << endl;
        res->Set(true, geodist);
      }
    }
    else {
//       cout << "symDist = " << symdist << endl;
      res->Set(true, symdist);
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
struct hybriddistanceInfo : OperatorInfo {
  hybriddistanceInfo() {
    name      = "hybriddistance";
    signature = "T x mpoint x T x mpoint x real x real -> real,  "
                "where T in {mlabel, mplace, mlabels}";
    syntax    = "hybriddistance( _ , _ , _ , _ );";
    meaning   = "Computes a distance between two trajectories. First, "
                "a distance between the symbolic representations is computed. "
                "If it is below the threshold, the discrete Fréchet distance "
                "is returned. Otherwise, the result equals the symbolic "
                "distance divided by the scale factor. Distance function and "
                "threshold can be changed via the sethybriddistanceparam "
                "operator.";
  }
};

/*
\section{Operator ~gethybriddistanceparams~}

gethybriddistanceparams: -> stream(tuple(Name: string, InputType: string, 
                                         Value: string))

\subsection{Type Mapping}

*/
ListExpr gethybriddistanceparamsTM(ListExpr args) {
  if (nl->HasLength(args, 0)) {
    ListExpr attrList = nl->FiveElemList(
                       nl->TwoElemList(nl->SymbolAtom("Name"),
                                       nl->SymbolAtom(CcString::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("InputType"),
                                       nl->SymbolAtom(CcString::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("DefaultValue"),
                                       nl->SymbolAtom(CcString::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("CurrentValue"),
                                       nl->SymbolAtom(CcString::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("Description"),
                                       nl->SymbolAtom(FText::BasicType())));
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                                          attrList));
  }
  return listutils::typeError("No argument expected.");
}

/*
\subsection{Value Mapping}

*/
int gethybriddistanceparamsVM(Word* args, Word& result, int message, 
                              Word& local, Supplier s) {
//   HybridDistanceParameters *hdp = (HybridDistanceParameters*)local.addr;
  switch (message) {
    case OPEN: {
//       if (hdp) {
//         delete hdp;
//         local.addr = 0;
//       }
//       hdp = new HybridDistanceParameters();
//       local.addr = hdp;
      return 0;
    }
    case REQUEST: {
//       if (!local.addr) {
//         result.addr = 0;
//         return CANCEL;
//       }
//       hdp = (HybridDistanceParameters*)local.addr;
      result.addr = hdp.getNextTuple();
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
//       if (local.addr) {
//         hdp = (HybridDistanceParameters*)local.addr;
//         delete hdp;
//         local.addr = 0;
//       }
      hdp.memberNo = 0;
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct gethybriddistanceparamsInfo : OperatorInfo {
  gethybriddistanceparamsInfo() {
    name      = "gethybriddistanceparams";
    signature = "-> stream(tuple(Name: string, InputType: string, DefaultValue:"
                "string, CurrentValue: string, Description: text))";
    syntax    = "gethybriddistanceparams();";
    meaning   = "Returns the name, input type, default value, current value, "
                "and description of every parameter applied for the "
                "hybriddistance operator.";
  }
};

/*
\section{Operator ~sethybriddistanceparam~}

sethybriddistanceparam: string x T -> bool,  where T corresponds to the type
                                             of the mentioned parameter

\subsection{Type Mapping}

*/
ListExpr sethybriddistanceparamTM(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError("Two arguments expected.");
  }
  if (!nl->HasLength(nl->First(args),2) || !nl->HasLength(nl->Second(args),2)) {
    return listutils::typeError("Argument error.");
  }
  string memberName;
  if (CcString::checkType(nl->First(nl->First(args)))) {
    memberName = nl->StringValue(nl->Second(nl->First(args)));
  }
  else if (FText::checkType(nl->First(nl->First(args)))) {
    memberName = nl->TextValue(nl->Second(nl->First(args)));
  }
  else {
    return listutils::typeError("First argument must be a string or a text.");
  }
  if (!HybridDistanceParameters::isCorrectType(memberName,
                                              nl->First(nl->Second(args)))) {
    return listutils::typeError("Invalid parameter / type combination.");
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
\subsection{Selection Function}

*/
int sethybriddistanceparamSelect(ListExpr args) {
  if (FText::checkType(nl->First(args)))    return 0;
  if (CcString::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class T>
int sethybriddistanceparamVM(Word* args, Word& result, int message, Word& local,
                             Supplier s) {
  result = qp->ResultStorage(s);
  T *memberName = static_cast<T*>(args[0].addr);
  CcBool *res = static_cast<CcBool*>(result.addr);
  res->SetDefined(false);
  if (!memberName->IsDefined()) {
    return 0;
  }
  else {
    std::string name(memberName->GetValue());
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (name == "labelfun") {
      if ((static_cast<CcInt*>(args[1].addr))->IsDefined()) {
        int value = (static_cast<CcInt*>(args[1].addr))->GetValue();
        res->Set(true, hdp.setLabelFun(value));
      }
    }
    else if (name == "distfun") {
      if ((static_cast<CcInt*>(args[1].addr))->IsDefined()) {
        int value = (static_cast<CcInt*>(args[1].addr))->GetValue();
        res->Set(true, hdp.setDistFun(value));
      }
    }
    else if (name == "geodistfun") {
      if ((static_cast<CcInt*>(args[1].addr))->IsDefined()) {
        int value = (static_cast<CcInt*>(args[1].addr))->GetValue();
        res->Set(true, hdp.setGeoDistFun(value));
      }
    }
    else if (name == "threshold") {
      if ((static_cast<CcReal*>(args[1].addr))->IsDefined()) {
        double value = static_cast<CcReal*>(args[1].addr)->GetValue();
        res->Set(true, hdp.setThreshold(value));
      }
    }
    else if (name == "scalefactor") {
      if ((static_cast<CcReal*>(args[1].addr))->IsDefined()) {
        double value = static_cast<CcReal*>(args[1].addr)->GetValue();
        res->Set(true, hdp.setScaleFactor(value));
      }
    }
    else if (name == "geoid") {
      if ((static_cast<Geoid*>(args[1].addr))->IsDefined()) {
        res->Set(true, hdp.setGeoid(static_cast<Geoid*>(args[1].addr)));
      }
    }
    else {
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
const string sethybriddistanceparamSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> string x T -> bool </text--->"
  "<text> Sets one of the parameters for the hybriddistance operator.\n"
  "<text> query sethybriddistanceparam(\"Threshold\", 1909.0) </text--->) )";

ValueMapping sethybriddistanceparamVMs[] = {sethybriddistanceparamVM<FText>,
                                            sethybriddistanceparamVM<CcString>};

Operator sethybriddistanceparam("sethybriddistanceparam", 
                       sethybriddistanceparamSpec, 2, sethybriddistanceparamVMs,
                       sethybriddistanceparamSelect, sethybriddistanceparamTM);

/*
\subsection{Operator ~longestcommonsubsequence~}

longestcommonsubsequence: mT x mT -> stream(T)

\subsubsection{Type Mapping}

*/
ListExpr longestcommonsubsequenceSymbolicTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    ListExpr first(nl->First(args)), second(nl->Second(args));
    if (MLabel::checkType(first) && MLabel::checkType(second)) {
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), 
                             nl->SymbolAtom(Label::BasicType()));
    }
    if (MPlace::checkType(first) && MPlace::checkType(second)) {
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), 
                             nl->SymbolAtom(Place::BasicType()));
    }
  }
  return listutils::typeError("Correct signatures: mlabel x mlabel -> stream("
    "label),   mplace x mplace -> stream(place)");
}

/*
\subsubsection{Selection Function}

*/
int longestcommonsubsequenceSymbolicSelect(ListExpr args) {
  if (MLabel::checkType(nl->First(args))) return 0;
  if (MPlace::checkType(nl->First(args))) return 1;
  return -1;
}

/*
\subsubsection{Local Info Class}

*/
template<class M, class B>
class LongestcommonsubsequenceLI {
 public:
  LongestcommonsubsequenceLI(M *m, NewPair<int, int> pos) :
                                            src(true), limits(pos), counter(0) {
    src.CopyFrom(m);
  }
  ~LongestcommonsubsequenceLI() {}
  
  B* getNextValue() {
    B *value = new B(true);
    int pos = limits.first + counter;
    if (pos > limits.second) {
      return 0;
    }
    src.GetBasic(pos, *value);
    counter++;
    return value;
  }
  
  
  M src;
  NewPair<int, int> limits;
  int counter;
};

/*
\subsubsection{Value Mapping}

*/
template<class M, class B>
int longestcommonsubsequenceSymbolicVM(Word* args, Word& result, int message,
                                       Word& local, Supplier s) {
  LongestcommonsubsequenceLI<M, B> *li =
                                  (LongestcommonsubsequenceLI<M, B>*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      M* src1 = static_cast<M*>(args[0].addr);
      M* src2 = static_cast<M*>(args[1].addr);
      if (src1->IsDefined() && src2->IsDefined()) {
        NewPair<int, int> limits = src1->LongestCommonSubsequence(*src2);
        li = new LongestcommonsubsequenceLI<M, B>(src1, limits);
      }
      local.addr = li;
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->getNextValue() : 0;
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
\subsubsection{Operator Info}

*/
struct longestcommonsubsequenceSymbolicInfo : OperatorInfo {
  longestcommonsubsequenceSymbolicInfo() {
    name      = "longestcommonsubsequence";
    signature = "mT x mT -> stream(T),  T in {label, place}";
    syntax    = "longestcommonsubsequence(_, _)";
    meaning   = "Computes the (first) longest common sequence of labels/places "
                "of the sources with the help of a dynamic programming "
                "algorithm.";
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
          case ST_FALSE: {
            b->Set(true, false);
            break;
          }
          case ST_TRUE: {
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
          case ST_FALSE: {
            b->Set(true, false);
            break;
          }
          case ST_TRUE: {
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
\section{Operator ~createtupleindex~}

\subsection{Type Mapping}

*/
template<class PosType, class PosType2>
ListExpr createtupleindexTM(ListExpr args) {
  string err = "Operator expects a stream of tuples where at least one "
               "attribute is a symbolic trajectory. Optionally, the user can "
               "specify the name of the main attribute.";
  if (!nl->HasLength(args, 1) && !nl->HasLength(args, 2)) {
    return listutils::typeError(err + " (" 
     + stringutils::int2str(nl->ListLength(args)) + " arguments instead of 1)");
  }
  if (!listutils::isTupleStream(nl->First(args))) {
    return listutils::typeError(err + " (no tuple stream received)");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string attrName;
  int pos = -1;
  if (nl->HasLength(args, 1)) {
    string symAttrNames[] = {"mlabel", "mlabels", "mplace", "mplaces"};
    int found = 0;
    for (int i = 0; i < 4; i++) {
      found += listutils::findType(attrList, nl->SymbolAtom(symAttrNames[i]),
                                   attrName);
      if (found != 0 && pos == -1) {
        pos = found;
      }
    }
    if (found == 0) {
      return listutils::typeError(err + " (no symbolic attribute found)");
    }
  }
  else {
    attrName = nl->SymbolValue(nl->Second(args));
    ListExpr type;
    pos = listutils::findAttribute(attrList, attrName, type);
    if (pos == 0 || !Tools::isSymbolicType(type)) {
      return listutils::typeError(err + " (" + attrName + " is not the name of "
             + "a symbolic attribute)");
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->OneElemList(nl->IntAtom(pos - 1)),
                    nl->SymbolAtom(TupleIndex<PosType, PosType2>::BasicType()));
}

/*
\subsection{Value Mapping}

*/
template<class PosType, class PosType2>
int createtupleindexVM(Word* args, Word& result, int message, Word& local, 
                       Supplier s) {
  result = qp->ResultStorage(s);
  Stream<Tuple> stream = static_cast<Stream<Tuple> >(args[0].addr);
  CcInt *attrno = static_cast<CcInt*>(args[2].addr);
  TupleIndex<PosType, PosType2> * ti 
                    = static_cast<TupleIndex<PosType, PosType2>* >(result.addr);
  int counter = 0;
  stream.open();
  Tuple* tuple = stream.request();
  if (tuple) {
    ti->initialize(tuple->GetTupleType(), attrno->GetIntval());
  }
  while (tuple) {
    if (!ti->addTuple(tuple)) {
      ti->deleteIndexes();
      return 0;
    }
    counter++;
    if (counter % 1000 == 0) {
      cout << counter << " tuples processed" << endl;
    }
    tuple->DeleteIfAllowed();
    tuple = stream.request();
  }
  stream.close();
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createtupleindexInfo : OperatorInfo {
  createtupleindexInfo() {
    name      = "createtupleindex";
    signature = "stream(tuple(X)) --> bool";
    syntax    = "_ createtupleindex";
    meaning   = "Creates a multiple index for all moving attributes of the "
                "tuple stream.";
  }
};

/*
\section{Operator ~bulkloadtupleindex~}

\subsection{Type Mapping}

*/
template<class PosType, class PosType2>
ListExpr bulkloadtupleindexTM(ListExpr args) {
  string err = (PosType::BasicType() == "unitpos" ? "Operator expects a "
         "relation and the name of an attribute of a moving type, e.g., mlabel."
         : "Operator expects a relation");
  int numOfArgs = (PosType::BasicType() == "unitpos" ? 2 : 1);
  if (!nl->HasLength(args, numOfArgs)) {
    return listutils::typeError(err + " (" + 
      stringutils::int2str(nl->ListLength(args)) + " arguments instead of " +
      stringutils::int2str(numOfArgs) + ")");
  }
  if (!listutils::isRelDescription(nl->First(args))) {
    return listutils::typeError(err + " (no relation received)");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string attrName;
  int pos = -1;
//   if (nl->HasLength(args, 1)) {
//     string symAttrNames[] = {"mlabel", "mlabels", "mplace", "mplaces"};
//     int found = 0;
//     for (int i = 0; i < 4; i++) {
//       found += listutils::findType(attrList, nl->SymbolAtom(symAttrNames[i]),
//                                    attrName);
//       if (found != 0 && pos == -1) {
//         pos = found;
//       }
//     }
//     if (found == 0) {
//       return listutils::typeError(err + " (no symbolic attribute found)");
//     }
//   }
  if (numOfArgs == 2) { // attribute name given
    attrName = nl->SymbolValue(nl->Second(args));
    ListExpr type;
    pos = listutils::findAttribute(attrList, attrName, type);
    if (pos == 0 || !Tools::isSymbolicType(type)) {
      return listutils::typeError(err + " (" + attrName + " is not the name of "
             + "a symbolic attribute)");
    }
  }
  ListExpr result = (numOfArgs == 1 ? 
    nl->SymbolAtom(TupleIndex<PosType, PosType2>::BasicType()) :
    nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                      nl->OneElemList(nl->IntAtom(pos - 1)),
                   nl->SymbolAtom(TupleIndex<PosType, PosType2>::BasicType())));
  return result;
}

/*
\subsection{Value Mapping}

*/
template<class PosType, class PosType2>
int bulkloadtupleindexVM(Word* args, Word& result, int message, Word& local, 
                         Supplier s) {
  result = qp->ResultStorage(s);
  Relation *rel = static_cast<Relation*>(args[0].addr);
  int attrPos = -1;
  if (PosType::BasicType() == "unitpos") {
    CcInt* attrno = static_cast<CcInt*>(args[2].addr);
    attrPos = attrno->GetIntval();
  }
  TupleIndex<PosType, PosType2>* ti
                     = static_cast<TupleIndex<PosType, PosType2>*>(result.addr);
  if (rel->GetNoTuples() == 0) {
    return 0;
  }
  TupleType *tt = rel->GetTupleType();
  ti->initialize(tt, attrPos);
  int majorValueNo;
  vector<pair<int, string> > relevantAttrs = 
                 Tools::getRelevantAttrs(tt, attrPos, majorValueNo);
  Supplier s0 = qp->GetSon(s, 0);
  ListExpr ttList = nl->Second(qp->GetType(s0));
  int relevantAttrCount = 0;
  for (int a = 1; a <= tt->GetNoAttributes(); a++) {
    if (Tools::isMovingAttr(ttList, a)) {
      string typeName = relevantAttrs[relevantAttrCount].second;
      ti->collectSortInsert(rel, a - 1, typeName, qp->GetMemorySize(s));
      relevantAttrCount++;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
const string bulkloadtupleindexSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> relation(tuple(X)) x attrname --> tupleindex </text--->"
  "<text> _ bulkloadtupleindex[attr] </text--->"
  "<text> Creates a tupleindex for all moving attributes of the \n"
  "relation, given the name of an attribute. </text--->"
  "<text> Dotraj bulkloadtupleindex[Trajectory]</text--->) )";

struct bulkloadtupleindexInfo : OperatorInfo {
  bulkloadtupleindexInfo() {
    name      = "bulkloadtupleindex";
    signature = "relation(tuple(X)) x ATTRNAME --> tupleindex";
    syntax    = "_ bulkloadtupleindex[ _ ]";
    meaning   = "Creates a multiple index for all moving attributes of the "
                "relation, given the name of an attribute (applicable with "
                "indextmatches).";
  }
};

Operator bulkloadtupleindex("bulkloadtupleindex", bulkloadtupleindexSpec, 
            bulkloadtupleindexVM<UnitPos, UnitPos>, Operator::SimpleSelect, 
            bulkloadtupleindexTM<UnitPos, UnitPos>);

const string bulkloadtupleindex2Spec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> relation(tuple(X)) --> tupleindex2 </text--->"
  "<text> _ bulkloadtupleindex[attr] </text--->"
  "<text> Creates a tupleindex2 for all moving attributes of the \n"
  "relation. </text--->"
  "<text> Dotraj bulkloadtupleindex2</text--->) )";

struct bulkloadtupleindex2Info : OperatorInfo {
  bulkloadtupleindex2Info() {
    name      = "bulkloadtupleindex2";
    signature = "relation(tuple(X)) --> tupleindex2";
    syntax    = "_ bulkloadtupleindex2";
    meaning   = "Creates a tupleindex2 for all moving attributes of the "
                "relation (applicable with indextmatches2).";
  }
};


Operator bulkloadtupleindex2("bulkloadtupleindex2", bulkloadtupleindex2Spec, 
            bulkloadtupleindexVM<NewInterval, UnitPos>, Operator::SimpleSelect, 
            bulkloadtupleindexTM<NewInterval, UnitPos>);

/*
\section{Operator ~tmatches~}

\subsection{Type Mapping}

*/
ListExpr tmatchesTM(ListExpr args) {
  string err = "the expected syntax is: tuple(X) x attrname x (text | pattern)";
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  ListExpr attrs;
  if (!Tuple::checkType(nl->First(nl->First(args)))) {
    return listutils::typeError(err + " (first argument is not a tuple)");
  }
  attrs = nl->Second(nl->First(nl->First(args)));
  if (!listutils::isSymbol(nl->First(nl->Second(args))) ||
      (!FText::checkType(nl->First(nl->Third(args))) && 
        !PatPersistent::checkType(nl->First(nl->Third(args))))) {
    return listutils::typeError(err + " (error in 2nd or 3rd argument)");
  }
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
    Supplier s0 = qp->GetSon(s, 0);
    ListExpr ttype = qp->GetType(s0);
    p = Pattern::getPattern(pat->toText(), false, tuple, ttype);
    if (p) {
      ExtBool result = p->tmatches(tuple, attrno->GetIntval(), ttype);
      if (result == ST_TRUE) {
        res->Set(true, true); 
      }
      else if (result == ST_FALSE) {
        res->Set(true, false);  
      }
      else {
        res->SetDefined(false);
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
\section{Operator ~indextmatches~}

\subsection{Type Mapping}

*/
ListExpr indextmatchesTM(ListExpr args) {
  string err = "the expected syntax is: tupleindex x rel x attrname x "
               "(text | pattern)";
  if (!nl->HasLength(args, 4)) {
    return listutils::typeError(err + " (4 arguments expected)");
  }
  if (!TupleIndex<UnitPos, UnitPos>::checkType(nl->First(nl->First(args)))) {
    return listutils::typeError(err + " (first argument is not a tuple index)");
  }
  if (!Relation::checkType(nl->First(nl->Second(args)))) {
    return listutils::typeError(err + " (second argument is not a relation)");
  }
  if (!listutils::isSymbol(nl->First(nl->Third(args))) ||
          (!FText::checkType(nl->First(nl->Fourth(args))) && 
           !PatPersistent::checkType(nl->First(nl->Fourth(args))))) {
    return listutils::typeError(err + " (error in 3rd or 4th argument)");
  }
  ListExpr tList = nl->Second(nl->First(nl->Second(args)));
  ListExpr attrs = nl->Second(tList);
  string name = nl->SymbolValue(nl->First(nl->Third(args)));
  ListExpr type;
  int index = listutils::findAttribute(attrs, name, type);
  if (!index) {
    return listutils::typeError("Attribute " + name + " not found in relation");
  }
  if (!MLabel::checkType(type) && !MLabels::checkType(type) &&
      !MPlace::checkType(type) && !MPlaces::checkType(type)) {
    return listutils::typeError("Attribute " + name + " is not a symbolic "
                              "trajectory (MLabel, MLabels, MPlace, MPlaces)");
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->IntAtom(index - 1)), 
                      nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), tList));
}

/*
\subsection{Selection Function}

*/
int indextmatchesSelect(ListExpr args) {
  return FText::checkType(nl->Fourth(args)) ? 0 : 1;
}

/*
\subsection{Value Mapping}

*/
template<class P>
int indextmatchesVM(Word* args, Word& result, int message, Word& local, 
                    Supplier s) {
  TMatchIndexLI *li = (TMatchIndexLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      Relation *rel = static_cast<Relation*>(args[1].addr);
      CcInt *attrno = static_cast<CcInt*>(args[4].addr);
      TupleIndex<UnitPos, UnitPos> *ti
                     = static_cast<TupleIndex<UnitPos, UnitPos>*>(args[0].addr);
      FText* pText = static_cast<FText*>(args[3].addr);
      Pattern *p = 0;
      if (pText->IsDefined() && attrno->IsDefined() && rel->GetNoTuples() > 0) {
        Supplier s0 = qp->GetSon(s, 1);
        ListExpr ttList = nl->Second(qp->GetType(s0));
//         cout << "ttype is " << nl->ToString(ttList) << endl;
        Tuple *firstTuple = rel->GetTuple(1, false);
        TupleType *tt = firstTuple->GetTupleType();
        p = Pattern::getPattern(pText->GetValue(), false, firstTuple, ttList);
        if (p) {
          vector<pair<int, string> > relevantAttrs;
          int majorValueNo = -1;
          if (p->isCompatible(tt, attrno->GetIntval(), relevantAttrs, 
                              majorValueNo)) {
            DataType mtype = Tools::getDataType(tt, attrno->GetIntval());
            li = new TMatchIndexLI(rel, ttList, ti, attrno->GetIntval(), p, 
                                   majorValueNo, mtype);
            if (!li->initialize(true)) {
              delete li;
              li = 0;
              local.addr = 0;
              cout << "initialization failed" << endl;
            }
          }
        }
        else {
          cout << "invalid pattern" << endl;
        }
        firstTuple->DeleteIfAllowed();
      }
      local.addr = li;
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
const string indextmatchesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> tuple(X) x attrname x (pattern | text) -> bool </text--->"
  "<text> tupleindex rel indextmatches [attr, p] </text--->"
  "<text> Checks whether the moving type attributes of the relation match the\n"
  "pattern. The given attribute name has to be the name of a symbolic\n"
  "trajectory attribute. It is treated as a main attribute."
  "<text>query Part bulkloadtupleindex[ML] Part indextmatches[ML, "
  "'* (_ _ superset{\"BKA\"}) *'] count </text--->) )";

ValueMapping indextmatchesVMs[] = {indextmatchesVM<FText>, 
                                   indextmatchesVM<PatPersistent>};

Operator indextmatches("indextmatches", indextmatchesSpec, 2, indextmatchesVMs,
                       indextmatchesSelect, indextmatchesTM);

/*
\section{Operator ~indextmatches2~}

\subsection{Type Mapping}

*/
ListExpr indextmatches2TM(ListExpr args) {
  string err = "the expected syntax is: tupleindex x rel x (text | pattern)";
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(err + " (3 arguments expected)");
  }
  if (!TupleIndex<NewInterval, UnitPos>::checkType(nl->First(nl->First(args)))){
    return listutils::typeError(err + " (1st argument is not a tupleindex2)");
  }
  if (!Relation::checkType(nl->First(nl->Second(args)))) {
    return listutils::typeError(err + " (2nd argument is not a relation)");
  }
  if (!FText::checkType(nl->First(nl->Third(args))) && 
      !PatPersistent::checkType(nl->Third(args))) {
    return listutils::typeError(err + " (3rd argument is not a text/pattern)");
  }
  ListExpr tList = nl->Second(nl->First(nl->Second(args)));
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), tList);
}

/*
\subsection{Selection Function}

*/
int indextmatches2Select(ListExpr args) {
  return FText::checkType(nl->Third(args)) ? 0 : 1;
}

/*
\subsection{Value Mapping}

*/
template<class P>
int indextmatches2VM(Word* args, Word& result, int message, Word& local, 
                     Supplier s) {
  TMatchIndexLI *li = (TMatchIndexLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      Relation *rel = static_cast<Relation*>(args[1].addr);
      TupleIndex<NewInterval, UnitPos> *ti 
                 = static_cast<TupleIndex<NewInterval, UnitPos>*>(args[0].addr);
      FText* pText = static_cast<FText*>(args[2].addr);
      Pattern *p = 0;
      if (pText->IsDefined() && rel->GetNoTuples() > 0) {
        Supplier s0 = qp->GetSon(s, 1);
        ListExpr ttList = nl->Second(qp->GetType(s0));
//         cout << "ttype is " << nl->ToString(ttList) << endl;
        Tuple *firstTuple = rel->GetTuple(1, false);
        TupleType *tt = firstTuple->GetTupleType();
        p = Pattern::getPattern(pText->GetValue(), false, firstTuple, ttList);
        if (p) {
          vector<pair<int, string> > relevantAttrs;
          int majorValueNo;
          if (p->isCompatible(tt, -1, relevantAttrs, majorValueNo)) {
            li = new TMatchIndexLI(rel, ttList, ti, p);
            if (!li->initialize(false)) {
              delete li;
              li = 0;
              local.addr = 0;
              cout << "initialization failed" << endl;
            }
          }
        }
        else {
          cout << "invalid pattern" << endl;
        }
        firstTuple->DeleteIfAllowed();
      }
      local.addr = li;
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
const string indextmatches2Spec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> tuple(X) x attrname x (pattern | text) -> bool </text--->"
  "<text> tupleindex rel indextmatches2 [p] </text--->"
  "<text> Checks whether the moving type attributes of the relation match the\n"
  "pattern. A main attribute is not specified, in contrast to indextmatches."
  "<text>query Part bulkloadtupleindex[ML] Part indextmatches2["
  "'* (_ _ superset{\"BKA\"}) *'] count </text--->) )";

ValueMapping indextmatches2VMs[] = {indextmatches2VM<FText>, 
                                   indextmatches2VM<PatPersistent>};

Operator indextmatches2("indextmatches2", indextmatches2Spec, 2, 
                     indextmatches2VMs, indextmatches2Select, indextmatches2TM);


/*
\section{Operator ~indexrewrite~}

\subsection{Type Mapping}

*/
ListExpr indexrewriteTM(ListExpr args) {
  string err = "the expected syntax is: tupleindex x rel x attrname x "
               "(text | pattern)";
  if (!nl->HasMinLength(args,1))
    return listutils::typeError("Operator requires 1 Argument at least!");
  if (!TupleIndex<UnitPos, UnitPos>::checkType(nl->First(args))) {
    return listutils::typeError(err + " (first argument is not a tupleindex)");
  }
  if (!Relation::checkType(nl->Second(args))) {
    return listutils::typeError(err + " (second argument is not a relation)");
  }
  if (!listutils::isSymbol(nl->Third(args)) ||
          (!FText::checkType(nl->Fourth(args)) && 
           !PatPersistent::checkType(nl->Fourth(args)))) {
    return listutils::typeError(err + " (error in 3rd or 4th argument)");
  }
  ListExpr attrs = nl->Second(nl->Second(nl->Second(args)));
  string name = nl->SymbolValue(nl->Third(args));
  ListExpr type;
  int index = listutils::findAttribute(attrs, name, type);
  if (!index) {
    return listutils::typeError("Attribute " + name + " not found in relation");
  }
  if (!MLabel::checkType(type) && !MLabels::checkType(type) &&
      !MPlace::checkType(type) && !MPlaces::checkType(type)) {
    return listutils::typeError("Attribute " + name + " is not a symbolic "
                              "trajectory (MLabel, MLabels, MPlace, MPlaces)");
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->IntAtom(index - 1)), 
                       nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), type));
}

/*
\subsection{Selection Function}

*/
int indexrewriteSelect(ListExpr args) {
  ListExpr attrs = nl->Second(nl->Second(nl->Second(args)));
  string name = nl->SymbolValue(nl->Third(args));
  ListExpr type;
  listutils::findAttribute(attrs, name, type);
  if (MLabel::checkType(type)) return 0;
  if (MLabels::checkType(type)) return 1;
  if (MPlace::checkType(type)) return 2;
  if (MPlaces::checkType(type)) return 3;
  return -1;
}

/*
\subsection{Value Mapping}

*/
template<class M>
int indexrewriteVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  IndexRewriteLI<M> *li = (IndexRewriteLI<M>*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      Relation *rel = static_cast<Relation*>(args[1].addr);
      CcInt *attrno = static_cast<CcInt*>(args[4].addr);
      TupleIndex<UnitPos, UnitPos> *ti
                     = static_cast<TupleIndex<UnitPos, UnitPos>*>(args[0].addr);
      FText* pText = static_cast<FText*>(args[3].addr);
      Pattern *p = 0;
      if (pText->IsDefined() && attrno->IsDefined() && rel->GetNoTuples() > 0) {
        Supplier s0 = qp->GetSon(s, 1);
        ListExpr ttList = nl->Second(qp->GetType(s0));
//         cout << "ttype is " << nl->ToString(ttList) << endl;
        Tuple *firstTuple = rel->GetTuple(1, false);
        TupleType *tt = firstTuple->GetTupleType();
        p = Pattern::getPattern(pText->GetValue(), false, firstTuple, ttList);
        if (p) {
          vector<pair<int, string> > relevantAttrs;
          int majorValueNo = -1;
          if (p->isCompatible(tt, attrno->GetIntval(), relevantAttrs, 
                              majorValueNo)) {
            DataType mtype = Tools::getDataType(tt, attrno->GetIntval());
            li = new IndexRewriteLI<M>(rel, ttList, ti, attrno->GetIntval(), p, 
                                       majorValueNo, mtype);
            if (!li->initialize(true, true)) {
              delete li;
              li = 0;
              local.addr = 0;
              cout << "initialization failed" << endl;
            }
          }
        }
        else {
          cout << "invalid pattern" << endl;
        }
        firstTuple->DeleteIfAllowed();
      }
      local.addr = li;
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
struct indexrewriteInfo : OperatorInfo {
  indexrewriteInfo() {
    name      = "indexrewrite";
    signature = "tupleindex x rel x attrname x (text | pattern) -> stream(T),"
                " where T is the type of attrname";
    syntax    = "_ _ indexrewrite[_, _]";
    meaning   = "Rewrites all attribute values according to the pattern.";
  }
};

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
      Pattern *p = 0;
      P *pText = static_cast<P*>(args[4].addr);
      CcInt *attr = static_cast<CcInt*>(args[5].addr);
      InvertedFile *inv = static_cast<InvertedFile*>(args[2].addr);
      R_Tree<1, NewPair<TupleId, int> > *rt = 
                  static_cast<R_Tree<1, NewPair<TupleId, int> >*>(args[3].addr);
      Relation *rel = static_cast<Relation*>(args[0].addr);
      if (pText->IsDefined() && attr->IsDefined()) {
        p = Pattern::getPattern(pText->toText(), false);
        if (p) {
          if (p->isValid(M::BasicType())) {
            local.addr = new IndexMatchesLI(rel, inv, rt, attr->GetIntval(), p,
                                            Tools::getDataType(M::BasicType()));
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
    signature = "rel(tuple(X)) x IDENT x invfile x rtree x text -> "
                "stream(tuple(X))";
    syntax    = "_ indexmatches [ _ , _ , _ , _ ]";
    meaning   = "Filters a relation containing a mlabel attribute, applying a "
                "twofold index (trie and 1-dim rtree) and passing only those "
                "trajectories matching the pattern on to the output stream.";
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
          if (p->initAssignOpTrees() && p->initEasyCondOpTrees(false)) {
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
      R_Tree<1, NewPair<TupleId, UnitPos> > *rt = 
              static_cast<R_Tree<1, NewPair<TupleId, UnitPos> >*>(args[3].addr);
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
  InvertedFileT<UnitPos, UnitPos>* inv 
                                = (InvertedFileT<UnitPos, UnitPos>*)result.addr;
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
  int attrno = ((CcInt*)args[2].addr)->GetIntval() - 1;
  int64_t dummy;
  for (int i = 0; i < rel->GetNoTuples(); i++) {
    tuple = rel->GetTuple(i + 1, false);
    src = (M*)(tuple->GetAttribute(attrno));
    TupleIndex<UnitPos, UnitPos>::insertIntoTrie(inv, i + 1, src, 
    Tools::getDataType(tuple->GetTupleType(), attrno), cache, trieCache, dummy);
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
\section{Operator ~createMaxspeedRaster~}

\subsection{Type Mapping}

*/
// ListExpr createMaxspeedRasterTM(ListExpr args) {
//   const std::string error_message = "expects the signature sint x nrel(... "
//    "(WayInfo (arel (tuple ((WayTagKey text) (WayTagValue text)))))) x rtree";
//   if (!nl->HasLength(args, 3)) {
//     return listutils::typeError(error_message);
//   }
//   if (!raster2::sint::checkType(nl->First(args))) {
//     return listutils::typeError("First argument must be an sint");
//   }
//   if (!RTree2TID::checkType(nl->Third(args))) {
//     return listutils::typeError("Third argument must be an rtree");
//   }
//   if (NestedRelation::checkType(nl->Second(args))) {
//     if (nl->HasLength(nl->Second(args), 2)) {
//       if (nl->HasLength(nl->Second(nl->Second(args)), 2)) {
//         ListExpr nrelAttrs = nl->Second(nl->Second(nl->Second(args)));
//         ListExpr arelAttrs = nl->Second(nl->Second(nl->Second(nl->Nth(
//                                     nl->ListLength(nrelAttrs), nrelAttrs))));
//         if (nl->ToString(nl->First(nl->First(arelAttrs))) == "WayTagKey"
//          && nl->ToString(nl->First(nl->Second(arelAttrs))) == "WayTagValue"
//          && FText::checkType(nl->Second(nl->First(arelAttrs)))
//          && FText::checkType(nl->Second(nl->Second(arelAttrs)))) {
//           return nl->SymbolAtom(raster2::sint::BasicType());
//         }
//       }
//     }
//   }
//   return listutils::typeError("Second argument must be an nrel(... (WayInfo "
//                    "(arel (tuple ((WayTagKey text) (WayTagValue text))))))");
// }

/*
\subsection{Value Mapping}

*/
// int createMaxspeedRasterVM(Word* args, Word& result, int message,Word& local,
//                            Supplier s) {
//   result = qp->ResultStorage(s);
//   raster2::sint *hgt = static_cast<raster2::sint*>(args[0].addr);
//   NestedRelation *nrel = static_cast<NestedRelation*>(args[1].addr);
//   RTree2TID *rtree = static_cast<RTree2TID*>(args[2].addr);
//   MaxspeedRaster mr(hgt, nrel, rtree);
//   raster2::sint *res = static_cast<raster2::sint*>(result.addr);
//   res->setGrid(hgt->getGrid());
//   raster2::sint::storage_type& rs = hgt->getStorage();
//   for (raster2::sint::iter_type it = rs.begin(),e = rs.end(); it != e; ++it){
//     raster2::RasterIndex<2> pos = it.getIndex();
//     int maxspeed = mr.getMaxspeed(pos);
//     res->set(pos, maxspeed);
//   }
//   return 0;
// }

/*
\subsection{Operator Info}

*/
// struct createMaxspeedRasterInfo : OperatorInfo {
//   createMaxspeedRasterInfo() {
//     name      = "createMaxspeedRaster";
//     signature = raster2::sint::BasicType() + " x nrel(... (WayInfo (arel 
//                 (tuple((WayTagKey text) (WayTagValue text)))))) x rtree" +  "
//                 -> " + raster2::sint::BasicType();
//     syntax    = "createMaxspeedRaster( _ , _ , _ )";
//     meaning   = "Creates an sint raster where every tile holds the maximum"
//                 "speed (kph) permitted on the roads inside the tile.";
//   }
// };

/*
\section{Operator ~createTileAreas~}

\subsection{Type Mapping}

*/
// ListExpr createTileAreasTM(ListExpr args) {
//   if (!nl->HasLength(args, 1)) {
//     return listutils::typeError("One argument expected");
//   }
//   if (!raster2::sint::checkType(nl->First(args))) {
//     return listutils::typeError("First argument must be an sint");
//   }
//   return nl->SymbolAtom(Tileareas::BasicType());
// }

/*
\subsection{Value Mapping}

*/
// int createTileAreasVM(Word* args, Word& result, int message, Word& local, 
//                       Supplier s) {
//   result = qp->ResultStorage(s);
//   raster2::sint *hgt = static_cast<raster2::sint*>(args[0].addr);
//   Tileareas *res = static_cast<Tileareas*>(result.addr);
//   res->retrieveAreas(hgt);
//   res->recordRoadCourses(hgt);
//   return 0;
// }

/*
\subsection{Operator Info}

*/
// struct createTileAreasInfo : OperatorInfo {
//   createTileAreasInfo() {
//     name      = "createTileAreas";
//     signature = raster2::sint::BasicType() + " -> " + Tileareas::BasicType();
//     syntax    = "createTileAreas( _ )";
//     meaning   = "Stores all areas with tiles having the same value and all "
//                 "possible transitions from one area to another.";
//   }
// };

/*
\section{Operator ~restoreTraj~}

\subsection{Type Mapping}

*/
// ListExpr restoreTrajTM(ListExpr args) {
//   if (!nl->HasLength(args, 10)) {
//     return listutils::typeError("Ten arguments expected.");
//   }
//   if (!BTree::checkType(nl->Second(args))) {
//     return listutils::typeError("Second argument must be a btree");
//   }
//   if (!RTree2TID::checkType(nl->Third(args))) {
//     return listutils::typeError("Third argument must be an rtree");
//   }
//   if (!raster2::sint::checkType(nl->Fourth(args))) { // elevation raster file
//     return listutils::typeError("Fourth argument must be an sint");
//   }
//   if (!nl->Equal(nl->SymbolAtom(Hash::BasicType()), 
//        nl->First(nl->Fifth(args)))
//       || !CcInt::checkType(nl->Third(nl->Fifth(args)))) {
//     return listutils::typeError("Fifth argument must be a hash file");
//   }
//   if (!raster2::sint::checkType(nl->Sixth(args))) { // maxspeed raster file
//     return listutils::typeError("Sixth argument must be an sint");
//   }
//   if (!Tileareas::checkType(nl->Seventh(args))) { // tileareas
//     return listutils::typeError("Seventh argument must be a tileareas");
//   }
//   for (int i = 8; i <= 10; i++) {
//     if (!MLabel::checkType(nl->Nth(i, args))) {
//       std::stringstream sstr;
//       sstr << "Argument " << i << " must be an mlabel";
//       return listutils::typeError(sstr.str());
//     }
//   }
//   if (Relation::checkType(nl->First(args))) { // extended edges relation
//     if (Tuple::checkType(nl->First(nl->Rest(nl->First(args))))) {
//       ListExpr attrList =
//                nl->First(nl->Rest(nl->First(nl->Rest(nl->First(args)))));
//       if (nl->ListLength(attrList) != 5) {
//         return listutils::typeError("Edges relation must have 5 attributes");
//       }
//       if (!LongInt::checkType(nl->Second(nl->First(attrList)))) {
//         return listutils::typeError("Edges: type error in first attribute");
//       }
//       if (!FText::checkType(nl->Second(nl->Second(attrList)))) {
//         return listutils::typeError("Edges: type error in second attribute");
//       }
//       if (!SimpleLine::checkType(nl->Second(nl->Third(attrList)))) {
//         return listutils::typeError("Edges: type error in third attribute");
//       }
//       if (!CcString::checkType(nl->Second(nl->Fourth(attrList)))) {
//         return listutils::typeError("Edges: type error in fourth attribute");
//       }
//       if (!CcInt::checkType(nl->Second(nl->Fifth(attrList)))) {
//         return listutils::typeError("Edges: type error in fifth attribute");
//       }
//       return nl->TwoElemList(listutils::basicSymbol<Stream<Rectangle<2> > >
//          (),
//                              listutils::basicSymbol<Rectangle<2> >());
//     }
//   }
//   return listutils::typeError("Argument types must be rel(tuple(longint, "
//       text, "
//                             "sline, string, int)) x btree x rtree x sint x "
//                        "hash x sint x tileareas x mlabel x mlabel x mlabel");
// }

/*
\subsection{Value Mapping}

*/
// int restoreTrajVM(Word* args, Word& result, int message, Word& local, 
//                   Supplier s) {
//   Relation *edgesRel = static_cast<Relation*>(args[0].addr);
//   BTree *heightBtree = static_cast<BTree*>(args[1].addr);
//   RTree2TID *segmentsRtree = static_cast<RTree2TID*>(args[2].addr);
//   raster2::sint *raster = static_cast<raster2::sint*>(args[3].addr);
//   Hash *rhash = static_cast<Hash*>(args[4].addr);
//   raster2::sint *maxspeedRaster = static_cast<raster2::sint*>(args[5].addr);
//   Tileareas *ta = static_cast<Tileareas*>(args[6].addr);
//   MLabel *direction = static_cast<MLabel*>(args[7].addr);
//   MLabel *height = static_cast<MLabel*>(args[8].addr);
//   MLabel *speed = static_cast<MLabel*>(args[9].addr);
//   RestoreTrajLI *li = static_cast<RestoreTrajLI*>(local.addr);
//   switch (message) {
//     case OPEN: {
//       if (li) {
//         delete li;
//         local.addr = 0;
//       }
//       li = new RestoreTrajLI(edgesRel, heightBtree, segmentsRtree, raster,
//                        rhash, maxspeedRaster, ta, height, direction, speed);
//       local.addr = li;
//       return 0;
//     }
//     case REQUEST: {
//       result.addr = li ? li->nextCandidate() : 0;
//       return result.addr ? YIELD : CANCEL;
//     }
//     case CLOSE: {
//       if (local.addr) {
//         li = (RestoreTrajLI*)local.addr;
//         delete li;
//         local.addr = 0;
//       }
//       return 0;
//     }
//   }
// 
//   return 0;
// }

/*
\subsection{Operator Info}

*/
// struct restoreTrajInfo : OperatorInfo {
//   restoreTrajInfo() {
//     name      = "restoreTraj";
//     signature = "rel(tuple(longint,text,sline,string, int)) x btree x rtree "
//                 "x sint x hash x sint x mlabel x mlabel x mlabel";
//     syntax    = "restoreTraj( _ , _ , _ , _ , _ , _ , _ , _ , _)";
//     meaning   = "Restores the original trajectory (mpoint) from symbolic "
//                 "direction, height, and speed information as well as a road "
//                 "network with elevation data.";
//   }
// };

/*
\section{Operator ~getPatterns~}

\subsection{Type Mapping}

*/
ListExpr getPatternsTM(ListExpr args) {
  if (!nl->HasLength(args, 6) && !nl->HasLength(args, 7)) {
    return listutils::typeError("Six or seven arguments expected");
  }
  if (!Relation::checkType(nl->First(args))) {
    return listutils::typeError("1st argument is not a relation");
  }
  if (!listutils::isSymbol(nl->Second(args))) {
    return listutils::typeError("2nd argument is not an attribute name");
  }
  if (!listutils::isSymbol(nl->Third(args))) {
    return listutils::typeError("3rd argument is not an attribute name");
  }
  ListExpr attrs = nl->Second(nl->Second(nl->First(args)));
  string attrnameTextual = nl->SymbolValue(nl->Second(args));
  string attrnameSpatial = nl->SymbolValue(nl->Third(args));
  ListExpr type;
  int indexTextual = listutils::findAttribute(attrs, attrnameTextual, type);
  if (indexTextual == 0) {
    return listutils::typeError("Attribute " + attrnameTextual + " not found");
  }
  int indexSpatial = listutils::findAttribute(attrs, attrnameSpatial, type);
  if (indexSpatial == 0) {
    return listutils::typeError("Attribute " + attrnameSpatial + " not found");
  }
  if (!CcReal::checkType(nl->Fourth(args))) {
    return listutils::typeError("4th argument is not a real number");
  }
  if (!CcInt::checkType(nl->Fifth(args))) {
    return listutils::typeError("5th argument is not an integer");
  }
  if (!CcInt::checkType(nl->Sixth(args))) {
    return listutils::typeError("6th argument is not an integer");
  }
  if (nl->HasLength(args, 7)) {
    if (!Geoid::checkType(nl->Seventh(args))) {
      return listutils::typeError("7th argument is not a geoid");
    }
  }
  ListExpr outputAttrs = nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("Pattern"),
                                       nl->SymbolAtom(FText::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("Support"),
                                       nl->SymbolAtom(CcReal::BasicType())));
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbol::APPEND()),
           nl->TwoElemList(nl->IntAtom(indexTextual - 1),
                           nl->IntAtom(indexSpatial - 1)),
           nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                           outputAttrs)));
}

/*
\subsection{Value Mapping}

*/
template<class T>
int getPatternsVM(Word* args, Word& result, int message, Word& local, 
                  Supplier s) {
  T* li = (T*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      Geoid *geoid = 0;
      int geoidSpecified = (qp->GetNoSons(s) == 9 ? 1 : 0);
      Relation *rel = static_cast<Relation*>(args[0].addr);
      CcReal *suppmin = static_cast<CcReal*>(args[3].addr);
      CcInt *atomsmin = static_cast<CcInt*>(args[4].addr);
      CcInt *atomsmax = static_cast<CcInt*>(args[5].addr);
      if (geoidSpecified == 1) {
        geoid = static_cast<Geoid*>(args[6].addr);
        if (!geoid->IsDefined()) {
          return 0;
        }
      }
      CcInt *posTextual = static_cast<CcInt*>(args[6+geoidSpecified].addr);
      CcInt *posSpatial = static_cast<CcInt*>(args[7+geoidSpecified].addr);
      if (!suppmin->IsDefined() || !atomsmin->IsDefined() 
       || !atomsmax->IsDefined()) {
        return 0;
      }
      if (suppmin->GetValue() > 0 && suppmin->GetValue() <= 1 
                         && atomsmin->GetValue() > 0 && atomsmax->GetValue() > 0
                         && atomsmin->GetValue() <= atomsmax->GetValue()) {
        local.addr = new T(rel, 
          NewPair<int, int>(posTextual->GetValue(), posSpatial->GetValue()),
          suppmin->GetValue(), atomsmin->GetValue(), atomsmax->GetValue(),
          geoid, qp->GetMemorySize(s));
      }
      else {
        cout << "the minimum support has to be in (0,1], and the minimum number"
                " of atoms must be at least 1" << endl;
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? GetPatternsLI::getNextResult(li->agg,li->tupleType) :0;
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
struct getPatternsInfo : OperatorInfo {
  getPatternsInfo() {
    name      = "getPatterns";
    signature = "rel(tuple(X)) x ATTR x ATTR x real x int --> "
                "stream(tuple(Pattern: text, Support: real))";
    syntax    = "_ getPatterns[ _ , _ , _ , _ , _ ]";
    meaning   = "Computes patterns for spatio-textual attributes of movement "
                "data (mpoint, mlabel). The numeric parameters represent the "
                "patterns' minimum support and the minimum and maximum number "
                "of atoms for each pattern, respectively.";
  }
};

const string getPatternsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> rel(tuple(X)) x ATTR x ATTR x real x int x int --> \n"
  "stream(tuple(Pattern: text, Support: real))</text--->"
  "<text> _ getPatterns[ _ , _ , _ , _ , _ ] </text--->"
  "<text> Computes patterns for spatio-textual attributes of movement data \n"
  "(mpoint, mlabel). The numeric parameters represent the patterns' minimum \n"
  "support and the minimum and maximum number of atoms for each pattern, \n"
  "respectively.</text--->"
  "<text> query Dotraj feed extend[X: [const mpoint value undef]] consume \""
  "getPatterns[Trajectory, X, 0.5, 1, 5] count </text--->) )";

Operator getPatterns("getPatterns", getPatternsSpec, 
           getPatternsVM<GetPatternsLI>, Operator::SimpleSelect, getPatternsTM);

/*
\section{Operator ~createfptree~}

\subsection{Type Mapping}

Used for FP-tree and Projected Databases

*/
template<class T>
ListExpr createMiningStructureTM(ListExpr args) {
  if (!nl->HasLength(args, 4) && !nl->HasLength(args, 5)) {
    return listutils::typeError("Four or five arguments expected");
  }
  if (!Relation::checkType(nl->First(args))) {
    return listutils::typeError("1st argument is not a relation");
  }
  if (!listutils::isSymbol(nl->Second(args))) {
    return listutils::typeError("2nd argument is not an attribute name");
  }
  if (!listutils::isSymbol(nl->Third(args))) {
    return listutils::typeError("3rd argument is not an attribute name");
  }
  ListExpr attrs = nl->Second(nl->Second(nl->First(args)));
  string attrnameTextual = nl->SymbolValue(nl->Second(args));
  string attrnameSpatial = nl->SymbolValue(nl->Third(args));
  ListExpr type;
  int indexTextual = listutils::findAttribute(attrs, attrnameTextual, type);
  if (indexTextual == 0) {
    return listutils::typeError("Attribute " + attrnameTextual + " not found");
  }
  int indexSpatial = listutils::findAttribute(attrs, attrnameSpatial, type);
  if (indexSpatial == 0) {
    return listutils::typeError("Attribute " + attrnameSpatial + " not found");
  }
  if (!CcReal::checkType(nl->Fourth(args))) {
    return listutils::typeError("4th argument is not a real number");
  }
  if (nl->HasLength(args, 5)) {
    if (!Geoid::checkType(nl->Seventh(args))) {
      return listutils::typeError("5th argument is not a geoid");
    }
  }
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbol::APPEND()),
           nl->TwoElemList(nl->IntAtom(indexTextual - 1),
                           nl->IntAtom(indexSpatial - 1)),
           nl->SymbolAtom(T::BasicType()));
}

/*
\subsection{Value Mapping}

*/
template<class T>
int createMiningStructureVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  result = qp->ResultStorage(s);
  T *resultPtr = (T*)result.addr;
  resultPtr->clear();
  Geoid *geoid = 0;
  int geoidSpecified = (qp->GetNoSons(s) == 7 ? 1 : 0);
  Relation *rel = static_cast<Relation*>(args[0].addr);
  CcReal *suppmin = static_cast<CcReal*>(args[3].addr);
  if (geoidSpecified == 1) {
    geoid = static_cast<Geoid*>(args[4].addr);
    if (!geoid->IsDefined()) {
      return 0;
    }
  }
  CcInt *posTextual = static_cast<CcInt*>(args[4+geoidSpecified].addr);
  CcInt *posSpatial = static_cast<CcInt*>(args[5+geoidSpecified].addr);
  if (!suppmin->IsDefined()) {
    return 0;
  }
  if (suppmin->GetValue() <= 0 || suppmin->GetValue() > 1) {
    cout << "the minimum support has to be in (0,1]" << endl;
  }
  else {
    RelAgg *agg = new RelAgg();
    agg->scanRelation(rel, NewPair<int, int>(posTextual->GetValue(), 
                                             posSpatial->GetValue()), geoid);
    agg->filter(suppmin->GetValue(), qp->GetMemorySize(s));
    resultPtr->initialize(suppmin->GetValue(), agg);
    resultPtr->construct();
  }
  return 0;
}

/*
 \subsection{Operator Info}

*/
const string createfptreeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> rel(tuple(X)) x ATTR x ATTR x real --> fptree</text--->"
  "<text> _ createfptree[ _ , _ , _ ] </text--->"
  "<text> Computes an FP-tree from a relation, two attribute names, and a \n"
  "real number (minimum support).</text--->"
  "<text> query Dotraj feed extend[X: [const mpoint value undef]] consume \n"
  "createfptree[Trajectory, X, 0.5] getTypeNL\n"
  "</text--->) )";

Operator createfptree("createfptree", createfptreeSpec, 
                      createMiningStructureVM<FPTree>, Operator::SimpleSelect,
                      createMiningStructureTM<FPTree>);

/*
\section{Operator ~minefptree~}

\subsection{Type Mapping}

*/
template<class T>
ListExpr mineStructureTM(ListExpr args) {
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError("Three arguments expected");
  }
  if (!T::checkType(nl->First(args))) {
    return listutils::typeError("1st argument is not a " + T::BasicType());
  }
  if (!CcInt::checkType(nl->Second(args))) {
    return listutils::typeError("2nd argument is not an integer");
  }
  if (!CcInt::checkType(nl->Third(args))) {
    return listutils::typeError("3rd argument is not an integer");
  }
  ListExpr outputAttrs = nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("Pattern"),
                                       nl->SymbolAtom(FText::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("Support"),
                                       nl->SymbolAtom(CcReal::BasicType())));
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
              nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), outputAttrs));
}

/*
\subsection{Value Mapping}

*/
template<class T, class L>
int mineStructureVM(Word* args, Word& result, int message, Word& local, 
                    Supplier s) {
  L* li = (L*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      T *structure = static_cast<T*>(args[0].addr);
      CcInt *amin = static_cast<CcInt*>(args[1].addr);
      CcInt *amax = static_cast<CcInt*>(args[2].addr);
      if (!amin->IsDefined() || !amax->IsDefined()) {
        return 0;
      }
      if (amin->GetValue() > 0 && amax->GetValue() > 0
       && amin->GetValue() <= amax->GetValue()) {
        local.addr = new L(structure, amin->GetValue(), amax->GetValue());
      }
      else {
        cout << "condition 0 < minNoAtoms <= maxNoAtoms not fulfilled" << endl;
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
const string minefptreeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> fptree x int x int --> stream(tuple(Pattern: text, Support: real\n"
  "))</text--->"
  "<text> _ minefptree[ _ , _ ] </text--->"
  "<text> Retrieves the frequent patterns from an FP-tree.</text--->"
  "<text> query Dotraj feed extend[X: [const mpoint value undef]] consume \n"
  "createfptree[Trajectory, X, 0.5] minefptree[1, 5] count\n"
  "</text--->) )";

Operator minefptree("minefptree", minefptreeSpec, 
                    mineStructureVM<FPTree, MineFPTreeLI>, 
                    Operator::SimpleSelect, mineStructureTM<FPTree>);

/*
\section{Operator ~createprojecteddb~}

\subsection{Type Mapping}

See ~createStructureTM~

\subsection{Operator Info}

*/
const string createprojecteddbSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> rel x ATTR x ATTR x real ( x geoid) --> projecteddb </text--->"
  "<text> _ createprojecteddb [ _ , _ , _ , _ ] </text--->"
  "<text> Computes the auxiliary structure for PrefixSpan.</text--->"
  "<text> query Dotraj feed extend[X: [const mpoint value undef]] consume \n"
  "createprojecteddb[Trajectory, X, 0.5] getTypeNL</text--->) )";


Operator createprojecteddb("createprojecteddb", createprojecteddbSpec,
                   createMiningStructureVM<ProjectedDB>, Operator::SimpleSelect,
                           createMiningStructureTM<ProjectedDB>);

/*
\section{Operator ~prefixSpan~}

\subsection{Type Mapping}

See ~getPatternsTM~

\subsection{Value Mapping}

See ~getPatternsVM

 \subsection{Operator Info}

*/
const string prefixSpanSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> projecteddb x int x int --> stream(tuple(Pattern: text, \n"
  "Support: real))</text--->"
  "<text> _ prefixSpan[ _ , _ ] </text--->"
  "<text> Retrieves the frequent patterns from a projecteddb.</text--->"
  "<text> query Dotraj feed extend[X: [const mpoint value undef]] consume \n"
  "createprojecteddb[Trajectory, X, 0.5] prefixSpan[1, 2] count</text--->) )";

Operator prefixSpan("prefixSpan", prefixSpanSpec, 
                    mineStructureVM<ProjectedDB, PrefixSpanLI>,
                    Operator::SimpleSelect, mineStructureTM<ProjectedDB>);

/*
\section{Operator ~createverticaldb~}

\subsection{Type Mapping}

See ~createStructureTM~

\subsection{Operator Info}

*/
const string createverticaldbSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> rel x ATTR x ATTR x real ( x geoid) --> verticaldb </text--->"
  "<text> _ createverticaldb [ _ , _ , _ , _ ] </text--->"
  "<text> Computes the auxiliary structure for Spade.</text--->"
  "<text> query Dotraj feed extend[X: [const mpoint value undef]] consume \n"
  "createverticaldb[Trajectory, X, 0.5] getTypeNL</text--->) )";


Operator createverticaldb("createverticaldb", createverticaldbSpec,
                   createMiningStructureVM<VerticalDB>, Operator::SimpleSelect,
                           createMiningStructureTM<VerticalDB>);

/*
\section{Operator ~spade~}

\subsection{Type Mapping}

See ~getPatternsTM~

\subsection{Value Mapping}

See ~getPatternsVM

 \subsection{Operator Info}

*/
const string spadeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> verticaldb x int x int --> stream(tuple(Pattern: text, \n"
  "Support: real))</text--->"
  "<text> _ spade[ _ , _ ] </text--->"
  "<text> Retrieves the frequent patterns from a verticaldb.</text--->"
  "<text> query Dotraj feed extend[X: [const mpoint value undef]] consume \n"
  "createverticaldb[Trajectory, X, 0.5] spade[1, 2] count</text--->) )";

Operator spade("spade", spadeSpec, mineStructureVM<VerticalDB, SpadeLI>,
               Operator::SimpleSelect, mineStructureTM<VerticalDB>);

/*
\section{Operator ~getlabels~}

\subsection{Type Mapping}

*/
ListExpr getlabelsTM(ListExpr args) {
  string err = "Operator expects a symbolic trajectory.";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(err + " (" 
     + stringutils::int2str(nl->ListLength(args)) + " arguments instead of 2)");
  }
  if (!Tools::isSymbolicType(nl->First(args))) {
    return listutils::typeError(err + " (wrong type)");
  }
  return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                         nl->SymbolAtom(FText::BasicType()));
}

/*
\subsection{Value Mappings}

*/
template<class T>
int getlabelsVM(Word* args, Word& result, int message, Word& local, Supplier s){
  GetLabelsLI<T> *li = (GetLabelsLI<T>*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      T* src = static_cast<T*>(args[0].addr);
      if (!src->IsDefined()) {
        local.addr = 0;
      }
      else {
        local.addr = new GetLabelsLI<T>(*src);
      }
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

ValueMapping getlabelsVMs[] = {getlabelsVM<MLabel>, getlabelsVM<MLabels>, 
                               getlabelsVM<MPlace>, getlabelsVM<MPlaces>};

/*
\subsection{Operator Info}

*/
const string getlabelsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> stream(tuple(X)) x ATTR -> set(text) </text--->"
  "<text> getlabels ( _ ) </text--->"
  "<text> Collects and outputs all labels into the stream.</text--->"
  "<text> query getlabels(Dotraj feed extract[Trajectory])</text--->) )";

Operator getlabels("getlabels", getlabelsSpec, 4, getlabelsVMs,
                   distancesymSelect, getlabelsTM);

/*
\section{Operator ~createlexicon~}

\subsection{Type Mapping}

*/
ListExpr createlexiconTM(ListExpr args) {
  string err = "Operator expects a stream of tuples and the name of an "
               "mlabel(s) or mplace(s) attribute.";
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError(err + " (" 
     + stringutils::int2str(nl->ListLength(args)) + " arguments instead of 2)");
  }
  if (!listutils::isTupleStream(nl->First(args))) {
    return listutils::typeError(err + " (no tuple stream received)");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string attrName;
  int pos = -1;
  attrName = nl->SymbolValue(nl->Second(args));
  ListExpr type;
  pos = listutils::findAttribute(attrList, attrName, type);
  if (pos == 0 || !Tools::isSymbolicType(type)) {
    return listutils::typeError(err + " (" + attrName + " is not the name of "
            + "a symbolic attribute)");
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->OneElemList(nl->IntAtom(pos - 1)),
                           nl->SymbolAtom(InvertedFile::BasicType()));
}

/*
\subsection{Value Mapping}

*/
template<class T>
int createlexiconVM(Word* args, Word& result, int message, Word& local, 
                    Supplier s) {
  result = qp->ResultStorage(s);
  Stream<Tuple> stream = static_cast<Stream<Tuple> >(args[0].addr);
  int attrno = (static_cast<CcInt*>(args[2].addr))->GetValue();
  InvertedFile *inv = static_cast<InvertedFile*>(result.addr);
  inv->setParams(false, 1, "");
  vector<string> labels;
  multiset<string> allLabels;
  set<string> justInserted;
  stream.open();
  Tuple* tuple = stream.request();
  int tupleCounter = 0;
  while (tuple) {
    tupleCounter++;
    T *src = (T*)(tuple->GetAttribute(attrno));
    src->InsertLabels(labels);
    for (auto it : labels) {
      if (justInserted.find(it) == justInserted.end()) { // not present yet
        justInserted.insert(it);
        allLabels.insert(it);
      }
    }
    labels.clear();
    justInserted.clear();
    tuple->DeleteIfAllowed();
    tuple = stream.request();
  }
  stream.close();
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
  string curLabel = *(allLabels.begin());
  int freq(0), pos(0);
  for (auto it : allLabels) {
    if (it == curLabel) {
      freq++;
    }
    else { // next label
      inv->insertString(pos, curLabel, freq, 0, cache, trieCache);
      curLabel = it;
      freq = 1;
      pos++;
    }
  }
  inv->insertString(pos, curLabel, freq, 0, cache, trieCache);
  inv->insertString(tupleCounter, "zzzzzzzzzzzzzzzzzzzzzzzzzzzzz", 0, 0, cache,
                    trieCache);
  // TODO: find out why final entry cannot be accessed
  delete trieCache;
  delete cache;
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createlexiconSpec : OperatorInfo {
  createlexiconSpec() {
    name      = "createlexicon";
    signature = "stream(tuple(X)) x ATTR --> invfile";
    syntax    = "_ createlexicon [ _ ]";
    meaning   = "Creates a lexicon of all occurring labels, stores their "
                "position according to lexicographic order and the number of "
                "tuples they occur in.";
  }
};

ValueMapping createlexiconVMs[] = {createlexiconVM<MLabel>, 
  createlexiconVM<MLabels>, createlexiconVM<MPlace>, createlexiconVM<MPlaces>};

Operator createlexicon(createlexiconSpec(), createlexiconVMs, 
                       derivegroupsSelect, createlexiconTM);

/*
\section{Operator ~frequencyvector~}

\subsection{Type Mapping}

*/
ListExpr frequencyvectorTM(ListExpr args) {
  const string errMsg = "Expecting mT x inv (x bool), T in {label(s),place(s)}";
  if (!nl->HasLength(args, 2) && !nl->HasLength(args, 3)) {
    return listutils::typeError(errMsg);
  }
  ListExpr arg1 = nl->First(args);
  if (!MLabel::checkType(arg1) && !MLabels::checkType(arg1) &&
      !MPlace::checkType(arg1) && !MPlaces::checkType(arg1)) {
    return listutils::typeError(errMsg);
  }
  if (!InvertedFile::checkType(nl->Second(args))) {
    return listutils::typeError(errMsg);
  }
  if (nl->HasLength(args, 3)) {
    if (!CcBool::checkType(nl->Third(args))) {
      return listutils::typeError(errMsg);
    }
  }
  return nl->TwoElemList(nl->SymbolAtom(Vector::BasicType()),
                         nl->SymbolAtom(CcReal::BasicType()));
}

/*
\subsection{Value Mapping}

*/
template<class T>
int frequencyvectorVM(Word* args, Word& result, int message, Word& local, 
                      Supplier s) {
  T* src = static_cast<T*>(args[0].addr);
  InvertedFile* inv = static_cast<InvertedFile*>(args[1].addr);
  result = qp->ResultStorage(s);
  collection::Collection* res = 
                              static_cast<collection::Collection*>(result.addr);
  res->Clear();
  if (!src->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  bool useIdf = false;
  if (qp->GetNoSons(s) == 3) {
    CcBool *ccb = static_cast<CcBool*>(args[2].addr);
    if (!ccb->IsDefined()) {
      res->SetDefined(false);
      return 0;
    }
    useIdf = ccb->GetValue();
  }
  vector<double> fv(inv->getNoEntries() - 1, 0.0);
  src->FrequencyVector(*inv, fv, useIdf);
  for (unsigned int i = 0; i < fv.size(); i++) {
    CcReal *elem = new CcReal(true, fv[i]);
    res->Insert(elem, 1);
    elem->DeleteIfAllowed();
  }
  return  0;
}

/*
\subsection{Operator Info}

*/
struct frequencyvectorSpec : OperatorInfo {
  frequencyvectorSpec() {
    name      = "frequencyvector";
    signature = "mT x invfile (x bool) -> vector(real),  where T in {label(s), "
                "place(s)}";
    syntax    = "frequencyvector(_, _)";
    meaning   = "Computes the frequency vector of the labels of a symbolic "
                "trajectory, according to the labels stored in the trie. The "
                "boolean value indicates whether idf (inverted document "
                "frequency) is applied (false by default).";
  }
};

ValueMapping frequencyvectorVMs[] = {frequencyvectorVM<MLabel>, 
  frequencyvectorVM<MLabels>, frequencyvectorVM<MPlace>, 
  frequencyvectorVM<MPlaces>};

Operator frequencyvector(frequencyvectorSpec(), frequencyvectorVMs, 
                         distancesymSelect, frequencyvectorTM);

/*
\section{Operator ~cosinesim~}

\subsection{Type Mapping}

*/
ListExpr cosinesimTM(ListExpr args) {
  const string errMsg = "Expecting vector(int) x vector(int)";
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError(errMsg);
  }
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if (!collection::Collection::KindCheck(nl->First(args), errorInfo) ||
      !collection::Collection::KindCheck(nl->Second(args), errorInfo)) {
    return listutils::typeError(errMsg);
  }
  string ctype1 = nl->SymbolValue(nl->First(nl->First(args)));
  string ctype2 = nl->SymbolValue(nl->First(nl->Second(args)));
  if (ctype1 != Vector::BasicType() || ctype2 != Vector::BasicType()) {
    return listutils::typeError(errMsg + " two vectors required");
  }
  string etype1 = nl->SymbolValue(nl->Second(nl->First(args)));
  string etype2 = nl->SymbolValue(nl->Second(nl->Second(args)));
  if (etype1 != CcReal::BasicType() || etype2 != CcReal::BasicType()) {
    return listutils::typeError(errMsg + " real type required");
  }
  return nl->SymbolAtom(CcReal::BasicType());
}

/*
\subsection{Value Mapping}

*/
int cosinesimVM(Word* args, Word& result, int message, Word& local, Supplier s){
  collection::Collection* v1 = 
                             static_cast<collection::Collection*>(args[0].addr);
  collection::Collection* v2 = 
                             static_cast<collection::Collection*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcReal* res = (CcReal*)result.addr;
  if (!v1->IsDefined() || !v2->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  if (v1->GetNoComponents() != v2->GetNoComponents()) {
    res->SetDefined(false);
    return 0;
  }
  res->Set(true, cosineSimilarity(*v1, *v2));
  return  0;
}

/*
\subsection{Operator Info}

*/
struct cosinesimSpec : OperatorInfo {
  cosinesimSpec() {
    name      = "cosinesim";
    signature = "vector(real) x vector(real) -> real";
    syntax    = "cosinesim(_, _)";
    meaning   = "Computes the cosine similarity for two frequency vectors.";
  }
};

Operator cosinesim(cosinesimSpec(), cosinesimVM, cosinesimTM);

/*
\section{Operator ~jaccardsim~}

\subsection{Value Mapping (for a single MLabel)}

*/
int jaccardsimVM(Word* args, Word& result, int message, Word& local, 
                 Supplier s) {
  collection::Collection* v1 = 
                             static_cast<collection::Collection*>(args[0].addr);
  collection::Collection* v2 = 
                             static_cast<collection::Collection*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcReal* res = (CcReal*)result.addr;
  if (!v1->IsDefined() || !v2->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  if (v1->GetNoComponents() != v2->GetNoComponents()) {
    res->SetDefined(false);
    return 0;
  }
  res->Set(true, jaccardSimilarity(*v1, *v2));
  return  0;
}

/*
\subsection{Operator Info}

*/
struct jaccardsimSpec : OperatorInfo {
  jaccardsimSpec() {
    name      = "jaccardsim";
    signature = "vector(real) x vector(real) -> real";
    syntax    = "jaccardsim(_, _)";
    meaning   = "Computes the Jaccard similarity for two frequency vectors.";
  }
};

Operator jaccardsim(jaccardsimSpec(), jaccardsimVM, cosinesimTM);

/*
\section{Class ~SymbolicTrajectoryAlgebra~}

*/
  
class SymbolicTrajectoryAlgebra : public Algebra {
 public:
  SymbolicTrajectoryAlgebra() : Algebra() {

  AddTypeConstructor(&patternTC);
  AddTypeConstructor(&tupleindexTC);
  AddTypeConstructor(&tupleindex2TC);
  AddTypeConstructor(&classifierTC);
  AddTypeConstructor(&fptreeTC);
  AddTypeConstructor(&projecteddbTC);
  AddTypeConstructor(&verticaldbTC);
  
//   AddTypeConstructor(&tileareasTC);
  
  ValueMapping distanceVMs[] = {distanceVM<Label>, distanceVM<Labels>,
    distanceVM<Place>, distanceVM<Places>, distanceVM<MLabel>,
    distanceVM<MLabels>, distanceVM<MPlace>, distanceVM<MPlaces>, 0};
  AddOperator(distanceInfo(), distanceVMs, distanceSelect, distanceTM);
  
  ValueMapping distancesymVMs[] = {distancesymVM<MLabel>, 
      distancesymVM<MLabels>, distancesymVM<MPlace>, distancesymVM<MPlaces>, 0};
  AddOperator(distancesymInfo(), distancesymVMs, distancesymSelect, 
              distancesymTM);
  
  ValueMapping hybriddistanceVMs[] = {hybriddistanceVM<MLabel, false>,
              hybriddistanceVM<MPlace, false>, hybriddistanceVM<MLabels, false>,
              hybriddistanceVM<MLabel, true>, hybriddistanceVM<MPlace, true>,
              hybriddistanceVM<MLabels, false>, 0};
  AddOperator(hybriddistanceInfo(), hybriddistanceVMs, hybriddistanceSelect, 
              hybriddistanceTM);
  
  AddOperator(gethybriddistanceparamsInfo(), gethybriddistanceparamsVM,
              gethybriddistanceparamsTM);
  
  AddOperator(&sethybriddistanceparam);
  sethybriddistanceparam.SetUsesArgsInTypeMapping();
  
  ValueMapping longestcommonsubsequenceSymbolicVMs[] = 
    {longestcommonsubsequenceSymbolicVM<MLabel, Label>,
     longestcommonsubsequenceSymbolicVM<MPlace, Place>, 0};
  AddOperator(longestcommonsubsequenceSymbolicInfo(), 
              longestcommonsubsequenceSymbolicVMs, 
              longestcommonsubsequenceSymbolicSelect,
              longestcommonsubsequenceSymbolicTM);
      
  AddOperator(topatternInfo(), topatternVM, topatternTM);

  AddOperator(toclassifierInfo(), toclassifierVM, toclassifierTM);

  AddOperator(&matches);
  matches.SetUsesArgsInTypeMapping();
  
  AddOperator(createtupleindexInfo(), createtupleindexVM<UnitPos, UnitPos>,
              createtupleindexTM<UnitPos, UnitPos>);
  
  AddOperator(&bulkloadtupleindex);
  bulkloadtupleindex.SetUsesMemory();
  
  AddOperator(&bulkloadtupleindex2);
  bulkloadtupleindex2.SetUsesMemory();
   
  AddOperator(&tmatches);
  tmatches.SetUsesArgsInTypeMapping();
  
  AddOperator(&indextmatches);
  indextmatches.SetUsesArgsInTypeMapping();
  
  AddOperator(&indextmatches2);
  indextmatches2.SetUsesArgsInTypeMapping();
  
  ValueMapping indexrewriteVMs[] = {indexrewriteVM<MLabel>, 
    indexrewriteVM<MLabels>, indexrewriteVM<MPlace>, indexrewriteVM<MPlaces>,0};
  AddOperator(indexrewriteInfo(), indexrewriteVMs, indexrewriteSelect, 
              indexrewriteTM);
  
  ValueMapping createunitrtreeVMs[] = {createunitrtreeVM<MLabel>,
    createunitrtreeVM<MLabels>, createunitrtreeVM<MPlace>, 
    createunitrtreeVM<MPlaces>, 0};
  AddOperator(createunitrtreeInfo(), createunitrtreeVMs, createunitrtreeSelect,
              createunitrtreeTM);
  
//   ValueMapping indexmatchesVMs[] = {indexmatchesVM<MLabel, FText>, 
//     indexmatchesVM<MLabels, FText>, indexmatchesVM<MPlace, FText>, 
//     indexmatchesVM<MPlaces, FText>, indexmatchesVM<MLabel, PatPersistent>,
//     indexmatchesVM<MLabels, PatPersistent>, 
//     indexmatchesVM<MPlace, PatPersistent>,
//     indexmatchesVM<MPlaces, PatPersistent>, 0};
//   AddOperator(indexmatchesInfo(), indexmatchesVMs, indexmatchesSelect,
//               indexmatchesTM);

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

  AddOperator(createmlInfo(), createmlVM, createmlTM);

  AddOperator(createmlrelInfo(), createmlrelVM, createmlrelTM);

  ValueMapping createtrieVMs[] = {createtrieVM<MLabel>, createtrieVM<MLabels>,
                                createtrieVM<MPlace>, createtrieVM<MPlaces>, 0};
  AddOperator(createtrieInfo(), createtrieVMs, createtrieSelect, createtrieTM);
  
//   AddOperator(createMaxspeedRasterInfo(), createMaxspeedRasterVM,
//               createMaxspeedRasterTM);
  
//   AddOperator(createTileAreasInfo(), createTileAreasVM, createTileAreasTM);
  
//   AddOperator(restoreTrajInfo(), restoreTrajVM, restoreTrajTM);
  
//   ValueMapping derivegroupsVMs[] = {derivegroupsVM<MLabel>,
//     derivegroupsVM<MLabels>, derivegroupsVM<MPlace>, 
//     derivegroupsVM<MPlaces>,0};
//   AddOperator(derivegroupsInfo(), derivegroupsVMs, derivegroupsSelect,
//               derivegroupsTM);

  AddOperator(&getPatterns);
  getPatterns.SetUsesMemory();
  
  AddOperator(&createfptree);
  createfptree.SetUsesMemory();
  
  AddOperator(&minefptree);
  minefptree.SetUsesMemory();
  
  AddOperator(&createprojecteddb);
  createprojecteddb.SetUsesMemory();
  
  AddOperator(&prefixSpan);
  prefixSpan.SetUsesMemory();
  
  AddOperator(&createverticaldb);
  createverticaldb.SetUsesMemory();
  
  AddOperator(&spade);
  spade.SetUsesMemory();
  
  AddOperator(&getlabels);
  
  AddOperator(&createlexicon);
  
  AddOperator(&frequencyvector);
  
  AddOperator(&cosinesim);
  AddOperator(&jaccardsim);
  
  
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
