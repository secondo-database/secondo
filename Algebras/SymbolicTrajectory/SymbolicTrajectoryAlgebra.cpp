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
#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "TemporalExtAlgebra.h"
#include "FTextAlgebra.h"
#include "DateTime.h"
#include "CharTransform.h"
#include "Stream.h"
#include "SecParser.h"
#include "SymbolicTrajectoryAlgebra.h"
#include "TemporalUnitAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

#include <string>
#include <vector>

using namespace std;

namespace stj {

Label::Label(const bool def, const string& val) : CcString(def, val) {}

Label::Label(Label& rhs) : CcString(rhs.IsDefined(), rhs.GetValue()) {}

Label::Label(const bool def) : CcString(def) {}

Label::~Label() {}

string Label::GetValue() const {
  return CcString::GetValue();
}

void Label::Set(const bool defined, const string &value) {
  CcString::Set(defined, value);
}

Word Label::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word result = SetWord(Address(0));
  correct = false;
  NList list(instance);
  if (list.isAtom()) {
    if (list.isString()) {
      string text = list.str();
      correct = true;
      result.addr = new Label(true, text);
    }
    else {
      correct = false;
      cmsg.inFunError("Expecting a string!");
    }
  }
  else {
    correct = false;
    cmsg.inFunError("Expecting one string atom!");
  }
  return result;
}

ListExpr Label::Out(ListExpr typeInfo, Word value) {
  Label* label = static_cast<Label*>(value.addr);
  NList element (label->GetValue(), true);
  return element.listExpr();
}

Word Label::Create(const ListExpr typeInfo) {
  return (SetWord( new Label( true ) ));
}

void Label::Delete(const ListExpr typeInfo, Word& w) {
  delete static_cast<Label*>( w.addr );
  w.addr = 0;
}

void Label::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<Label*>( w.addr );
  w.addr = 0;
}

Word Label::Clone(const ListExpr typeInfo, const Word& w) {
  Label* label = static_cast<Label*>( w.addr );
  return SetWord( new Label(*label) );
}

int Label::SizeOfObj() {
  return sizeof(Label);
}

bool Label::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual( type, Label::BasicType() ));
}

ListExpr Label::Property() {
  return (nl->TwoElemList(
      nl->FiveElemList(nl->StringAtom("Signature"),
         nl->StringAtom("Example Type List"),
         nl->StringAtom("List Rep"),
         nl->StringAtom("Example List"),
         nl->StringAtom("Remarks")),
      nl->FiveElemList(nl->StringAtom("-> DATA"),
         nl->StringAtom(Label::BasicType()),
         nl->StringAtom("<x>"),
         nl->StringAtom("\"at home\""),
         nl->StringAtom("x must be of type string (max. 48 characters)."))));
}

const bool Label::checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
}

void Label::CopyFrom(const Attribute* right) {
  CcString::CopyFrom(right);
}

int Label::Compare(const Label* arg) const {
  return this->GetValue().compare(arg->GetValue());
}

TypeConstructor labelTC(
  Label::BasicType(),                          // name of the type in SECONDO
  Label::Property,                // property function describing signature
  Label::Out, Label::In,         // Out and In functions
  0, 0,                            // SaveToList, RestoreFromList functions
  Label::Create, Label::Delete,  // object creation and deletion
  0, 0,                            // object open, save
  Label::Close, Label::Clone,    // close, and clone
  0,                               // cast function
  Label::SizeOfObj,               // sizeof function
  Label::KindCheck );             // kind checking function


/*

4 Type Constructor ~ilabel~

Type ~ilabel~ represents an (instant, value)-pair of labels.

The list representation of an ~ilabel~ is

----    ( t string-value )
----

For example:

----    ( (instant 1.0) "My Label" )
----

4.1 function Describing the Signature of the Type Constructor

*/
ListExpr ILabel::IntimeLabelProperty() {
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(ilabel) "),
                             nl->StringAtom("(instant string-value)"),
                             nl->StringAtom("((instant 0.5) \"at home\")"))));
}

/*
4.2 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool ILabel::CheckIntimeLabel(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Intime<Label>::BasicType()));
}

/*
4.3 Creation of the type constructor ~ilabel~

*/
TypeConstructor intimelabel(
        ILabel::BasicType(),  //name
        ILabel::IntimeLabelProperty,   //property function describing signature
        OutIntime<CcString, OutCcString>,
        InIntime<CcString, InCcString>,     //Out and In functions
        0,
        0,  //SaveToList and RestoreFromList functions
        CreateIntime<CcString>,
        DeleteIntime<CcString>, //object creation and deletion
        0,
        0,  // object open and save
        CloseIntime<CcString>,
        CloneIntime<CcString>,  //object close and clone
        CastIntime<CcString>,   //cast function
        SizeOfIntime<CcString>, //sizeof function
        ILabel::CheckIntimeLabel       //kind checking function
);

/*
5 Type Constructor ~ulabel~

Type ~ulabel~ represents an (tinterval, stringvalue)-pair.

5.1 List Representation

The list representation of an ~ulabel~ is

----    ( timeinterval string-value )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   ["]My Label["] )
----

*/

ULabel::ULabel(const Interval<Instant>& interval, const Label& label)
                                                               : UString(true) {
  timeInterval = interval;
  constValue.CopyFrom(&label);
}

/*
5.2 function Describing the Signature of the Type Constructor

*/
ListExpr ULabel::ULabelProperty() {
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom("-> UNIT"),
                     nl->StringAtom("(ulabel) "),
                     nl->StringAtom("(timeInterval string) "),
                     nl->StringAtom("((i1 i2 FALSE FALSE) \"at home\")"))));
}

/*
5.3 Kind Checking Function

*/
bool ULabel::CheckULabel(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, ULabel::BasicType()));
}

/*
5.4 Creation of the type constructor ~ulabel~

*/
TypeConstructor unitlabel(
        ULabel::BasicType(), // ULabel::BasicType(),  //name
        ULabel::ULabelProperty,    //property function describing signature
        OutConstTemporalUnit<CcString, OutCcString>,
        InConstTemporalUnit<CcString, InCcString>,  //Out and In functions
        0,
        0,  //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcString>,
        DeleteConstTemporalUnit<CcString>,  //object creation and deletion
        0,
        0,  // object open and save
        CloseConstTemporalUnit<CcString>,
        CloneConstTemporalUnit<CcString>,   //object close and clone
        CastConstTemporalUnit<CcString>,    //cast function
        SizeOfConstTemporalUnit<CcString>,  //sizeof function
        ULabel::CheckULabel    //kind checking function
);

/*
\subsection{Type Constructor ~mlabel~}

Type ~mlabel~ represents a moving string.

\subsubsection{List Representation}

The list representation of a ~mlabel~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ulabel~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) ["]Label 1["] )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) ["]Label 2["] )
        )
----

*/
MLabel::MLabel(MString* ms): MString(1), index(0) {
  ULabel ul(1);
  for (int i = 0; i < ms->GetNoComponents(); i++) {
    ms->Get(i, ul);
    Add(ul);
  }
  SetDefined(ms->IsDefined());
}

MLabel::MLabel(MLabel* ml): MString(0), index(ml->index) {
  ULabel ul(1);
  for (int i = 0; i < ml->GetNoComponents(); i++) {
    ml->Get(i, ul);
    Add(ul);
  }
  SetDefined(ml->IsDefined());
}

ListExpr MLabel::MLabelProperty() {
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom("-> MAPPING"),
                     nl->StringAtom("(mlabel) "),
                     nl->StringAtom("( u1 ... un)"),
                     nl->StringAtom("(((i1 i2 TRUE TRUE) \"at home\") ...)"))));
}

/*
\subsubsection{Kind Checking Function}

This function checks whether the type constructor is applied correctly.

*/
bool MLabel::CheckMLabel(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, MLabel::BasicType()));
}

Word MLabel::Create(const ListExpr typeInfo) {
  MLabel* ml = new MLabel(0);
  return (SetWord(ml));
}

void MLabel::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<MLabel*>(w.addr);
  w.addr = 0;
}

MLabel* MLabel::Clone() const {
  MLabel* result = new MLabel(1);
  *result = *this;
//   cout << "result has " << result->GetNoComponents() << " units" << endl;
  result->index.copyFrom(this->index);
//   cout << result->index.getNodeRefSize() << " nodes copied" << endl;
  return result;
}

int MLabel::Compare(const Attribute * arg) const {
  if(!IsDefined() && !arg->IsDefined()) {
    return 0;
  }
  if(!IsDefined()) {
    return -1;
  }
  if(!arg->IsDefined()) {
    return 1;
  }
  const MLabel* ml = (const MLabel*)(arg);
  if (GetNoComponents() != ml->GetNoComponents()) {
    return (GetNoComponents() > ml->GetNoComponents() ? 1 : -1);
  }
  ULabel ul1(1), ul2(1);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ul1);
    ml->Get(i, ul2);
    if (ul1.constValue.GetValue().compare(ul2.constValue.GetValue())) {
      return ul1.constValue.GetValue().compare(ul2.constValue.GetValue());
    }
    if (ul1.timeInterval != ul2.timeInterval) {
      if (ul1.timeInterval.start < ul2.timeInterval.start) {
        return 1;
      }
      else if (ul1.timeInterval.start > ul2.timeInterval.start) {
        return -1;
      }
      if (ul1.timeInterval.end < ul2.timeInterval.end) {
        return -1;
      }
      else if (ul1.timeInterval.end > ul2.timeInterval.end) {
        return 1;
      }
      if (ul1.timeInterval.lc < ul2.timeInterval.lc) {
        return -1;
      }
      else if (ul1.timeInterval.lc > ul2.timeInterval.lc) {
        return 1;
      }
      if (ul1.timeInterval.rc < ul2.timeInterval.rc) {
        return -1;
      }
      else if (ul1.timeInterval.rc > ul2.timeInterval.rc) {
        return 1;
      }
    }
  }
  return 0;
}

void MLabel::CopyFrom(const Attribute* right) {
  *this = *((MLabel*)right);
  const MLabel* source = (const MLabel*)right;
  index.copyFrom(source->index);
}

int MLabel::NumOfFLOBs() const {
  return 4; // one for the time intervals, three for the index
}

Flob* MLabel::GetFLOB(const int i) {
  assert((0 <= i) && (i < 4));
  switch (i) {
    case 0: return &units;
    case 1: return index.getNodeRefsPtr();
    case 2: return index.getNodeLinksPtr();
    default: return index.getLabelIndexPtr();
  }
}



/*
\subsubsection{Function ~compress~}

If there are subsequent ULabels with the same Label, this function squeezes
them to one ULabel.

*/
MLabel* MLabel::compress() {
  MLabel* newML = new MLabel(1);
  if(!IsDefined()){
    newML->SetDefined(false);
    return newML;
  }
  ULabel ul(1);
  for (size_t i = 0; i < (size_t)this->GetNoComponents(); i++) {
    this->Get(i, ul);
    newML->MergeAdd(ul);
  }
// if (newML->GetNoComponents() < this->GetNoComponents()) {
//   cout << "MLabel was compressed from " << this->GetNoComponents() << " to "
//        << newML->GetNoComponents() << " components." << endl;
// }
// else {
//   cout << "MLabel could not be compressed, still has "
//        << newML->GetNoComponents() << " components." << endl;
// }
  return newML;
}

/*
\subsubsection{Function ~create~}

Creates an MLabel of a certain size for testing purposes. The labels will be
contain only numbers between 1 and size[*]rate; rate being the number of
different labels divided by the size.

*/
void MLabel::createML(int size, bool text, double rate = 1.0) {
  index.initRoot();
  index.cleanDbArrays();
  if ((size > 0) && (rate > 0) && (rate <= 1)) {
    int max = size * rate;
    MLabel* newML = new MLabel(1);
    ULabel ul(1);
    DateTime* start = new DateTime(instanttype);
    DateTime* halfHour = new DateTime(0, 1800000, durationtype); // duration
    start->Set(2012, 1, 1);
    Instant* end = new Instant(*start);
    end->Add(halfHour);
    SecInterval* iv = new SecInterval(*start, *end, true, false);
    if (text) {
      vector<string> trajectory = createTrajectory(size);
      for (int i = 0; i < size; i++) {
        ul.constValue.Set(true, trajectory[i]);
        iv->Set(*start, *end, true, false);
        ul.timeInterval = *iv;
        newML->Add(ul);
        start->Add(halfHour);
        end->Add(halfHour);
      }
    }
    else {
      for (int i = 0; i < size; i++) {
        ul.constValue.Set(true, int2String(max - (i % max)));
        start->Add(halfHour);
        end->Add(halfHour);
        iv->Set(*start, *end, true, false);
        ul.timeInterval = *iv;
        newML->Add(ul);
      }
    }
    *this = *newML;
    delete halfHour;
    delete start;
    delete end;
    delete iv;
    delete newML;
  }
  else {
    cout << "Invalid parameters for creation." << endl;
  }
}

/*
\subsection{Function ~convertFromMString~}

*/
void MLabel::convertFromMString(MString* source) {
  Clear();
  UString us(0);
  ULabel ul(0);
  ul.SetDefined(true);
  for (int i = 0; i < source->GetNoComponents(); i++) {
    source->Get(i, us);
    ul.timeInterval = us.timeInterval;
    ul.constValue = us.constValue;
    Add(ul);
  }
  SetDefined(source->IsDefined());
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

template<class T>
T* rewrite(MLabel const &ml,
           const pair<vector<unsigned int>, vector<unsigned int> > &seq,
           vector<Assign> assigns, map<string, int> varPosInSeq) {
  T* result = new T(1);
  if (!checkRewriteSeq(seq, ml.GetNoComponents(), false) ||
    (seq.first.empty() && seq.second.empty())) {
    result->SetDefined(false);
    return 0;
  }
  result->SetDefined(true);
  Word qResult;
  int vPos(-1), seqPos(0);
  string label("");
  Instant start(instanttype), end(instanttype);
  SecInterval iv(1);
  bool lc(false), rc(false);
  ULabel ul(1), uls(1);
//   cout << "First: ";
//   for (unsigned int i = 0; i < seq.first.size(); i++) {
//     cout << seq.first[i] << " ";
//   }
//   cout << endl << "Second: ";
//   for (unsigned int i = 0; i < seq.second.size(); i++) {
//     cout << seq.second[i] << " ";
//   }
//   cout << endl;
  for (unsigned int i = 0; i < assigns.size(); i++) {
    for (int j = 0; j < 6; j++) {
      if (!assigns[i].getText(j).empty()) {
        for (int k = 0; k < assigns[i].getRightSize(j); k++) {
          vPos = varPosInSeq[assigns[i].getRightVar(j, k)];
          switch (assigns[i].getRightKey(j, k)) {
            case 0: { // label
              ml.Get(seq.second[vPos], ul);
              assigns[i].setLabelPtr(k, ul.constValue.GetValue());
              break;
            }
            case 1: { // time
              ml.Get(seq.second[vPos], ul);
              iv.start = ul.timeInterval.start;
              iv.lc = ul.timeInterval.lc;
              ml.Get(seq.second[vPos + 1] - 1, ul);
              iv.end = ul.timeInterval.end;
              iv.rc = ul.timeInterval.rc;
              assigns[i].setTimePtr(k, iv);
              break;
            }
            case 2: { // start
              ml.Get(seq.second[vPos], ul);
              if (j == 2) {
                assigns[i].setStartPtr(k, ul.timeInterval.start);
              }
              else {
                assigns[i].setEndPtr(k, ul.timeInterval.start);
              }
              break;
            }
            case 3: { // end
              ml.Get(seq.second[vPos + 1] - 1, ul);
              if (j == 2) {
                assigns[i].setStartPtr(k, ul.timeInterval.end);
              }
              else {
                assigns[i].setEndPtr(k, ul.timeInterval.end);
              }
              break;
            }
            case 4: { // leftclosed
              ml.Get(seq.second[vPos], ul);
              if (j == 4) {
                assigns[i].setLeftclosedPtr(k, ul.timeInterval.lc);
              }
              else {
                assigns[i].setRightclosedPtr(k, ul.timeInterval.lc);
              }
              break;
            }
            case 5: { // rightclosed
              ml.Get(seq.second[vPos + 1] - 1, ul);
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
              return 0;
            }
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
    if (assigns[i].getPatternPos() > -1) { // variable occurs in p
      if (seq.first[seqPos] + 1 == seq.first[seqPos + 1]) { // 1 source ul
        ml.Get(seq.first[seqPos], uls);
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
          return 0;
        }
        result->MergeAdd(uls);
      }
      else { // arbitrary many source uls
        for (size_t m = seq.first[seqPos]; m < seq.first[seqPos + 1]; m++) {
          ml.Get(m, uls);
          if (!assigns[i].getText(0).empty()) {
            uls.constValue.Set(true, label);
          }
          if ((m == seq.first[seqPos]) && // first unit label
            (!assigns[i].getText(1).empty() || !assigns[i].getText(2).empty())){
            uls.timeInterval.start = start;
            if (!uls.timeInterval.IsValid()) {
              uls.timeInterval.Print(cout);
              cout << " is an invalid interval" << endl;
              result->SetDefined(false);
              return 0;
            }
          }
          if ((m == seq.first[seqPos + 1] - 1) && // last unit label
            (!assigns[i].getText(1).empty() || !assigns[i].getText(3).empty())){
            uls.timeInterval.end = end;
            if (!uls.timeInterval.IsValid()) {
              uls.timeInterval.Print(cout);
              cout << " is an invalid interval" << endl;
              result->SetDefined(false);
              return 0;
            }
          }
          if ((m == seq.first[seqPos]) && !assigns[i].getText(4).empty()) {
            uls.timeInterval.lc = lc;
          }
          if ((m == seq.first[seqPos+1] - 1) && !assigns[i].getText(5).empty()){
            uls.timeInterval.rc = rc;
          }
          result->MergeAdd(uls);
        }
      }
      seqPos = seqPos + 2;
    }
    else { // variable does not occur in p
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
      result->MergeAdd(uls);
    }
  }
  if (!result->IsValid()) {
    result->SetDefined(false);
  }
  return result;
}

/*
\subsection{Function ~rewrite~}

Rewrites a moving label using another moving label and a vector.

*/
void MLabel::rewrite(MLabel const &ml,
                   const pair<vector<unsigned int>, vector<unsigned int> > &seq,
                     vector<Assign> assigns, map<string, int> varPosInSeq) {
  if (!checkRewriteSeq(seq, ml.GetNoComponents(), false) ||
    (seq.first.empty() && seq.second.empty())) {
    SetDefined(false);
    return;
  }
  Word qResult;
  int vPos(-1), seqPos(0);
  string label("");
  Instant start(instanttype), end(instanttype);
  SecInterval iv(1);
  bool lc(false), rc(false);
  ULabel ul(1), uls(1);
//   cout << "First: ";
//   for (unsigned int i = 0; i < seq.first.size(); i++) {
//     cout << seq.first[i] << " ";
//   }
//   cout << endl << "Second: ";
//   for (unsigned int i = 0; i < seq.second.size(); i++) {
//     cout << seq.second[i] << " ";
//   }
//   cout << endl;
  for (unsigned int i = 0; i < assigns.size(); i++) {
    for (int j = 0; j < 6; j++) {
      if (!assigns[i].getText(j).empty()) {
        for (int k = 0; k < assigns[i].getRightSize(j); k++) {
          vPos = varPosInSeq[assigns[i].getRightVar(j, k)];
          switch (assigns[i].getRightKey(j, k)) {
            case 0: { // label
              ml.Get(seq.second[vPos], ul);
              assigns[i].setLabelPtr(k, ul.constValue.GetValue());
              break;
            }
            case 1: { // time
              ml.Get(seq.second[vPos], ul);
              iv.start = ul.timeInterval.start;
              iv.lc = ul.timeInterval.lc;
              ml.Get(seq.second[vPos + 1] - 1, ul);
              iv.end = ul.timeInterval.end;
              iv.rc = ul.timeInterval.rc;
              assigns[i].setTimePtr(k, iv);
              break;
            }
            case 2: { // start
              ml.Get(seq.second[vPos], ul);
              if (j == 2) {
                assigns[i].setStartPtr(k, ul.timeInterval.start);
              }
              else {
                assigns[i].setEndPtr(k, ul.timeInterval.start);
              }
              break;
            }
            case 3: { // end
              ml.Get(seq.second[vPos + 1] - 1, ul);
              if (j == 2) {
                assigns[i].setStartPtr(k, ul.timeInterval.end);
              }
              else {
                assigns[i].setEndPtr(k, ul.timeInterval.end);
              }
              break;
            }
            case 4: { // leftclosed
              ml.Get(seq.second[vPos], ul);
              if (j == 4) {
                assigns[i].setLeftclosedPtr(k, ul.timeInterval.lc);
              }
              else {
                assigns[i].setRightclosedPtr(k, ul.timeInterval.lc);
              }
              break;
            }
            case 5: { // rightclosed
              ml.Get(seq.second[vPos + 1] - 1, ul);
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
              return;
            }
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
    if (assigns[i].getPatternPos() > -1) { // variable occurs in p
      if (seq.first[seqPos] + 1 == seq.first[seqPos + 1]) { // 1 source ul
        ml.Get(seq.first[seqPos], uls);
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
          SetDefined(false);
          return;
        }
        this->MergeAdd(uls);
      }
      else { // arbitrary many source uls
        for (size_t m = seq.first[seqPos]; m < seq.first[seqPos + 1]; m++) {
          ml.Get(m, uls);
          if (!assigns[i].getText(0).empty()) {
            uls.constValue.Set(true, label);
          }
          if ((m == seq.first[seqPos]) && // first unit label
            (!assigns[i].getText(1).empty() || !assigns[i].getText(2).empty())){
            uls.timeInterval.start = start;
            if (!uls.timeInterval.IsValid()) {
              uls.timeInterval.Print(cout);
              cout << " is an invalid interval" << endl;
              SetDefined(false);
              return;
            }
          }
          if ((m == seq.first[seqPos + 1] - 1) && // last unit label
            (!assigns[i].getText(1).empty() || !assigns[i].getText(3).empty())){
            uls.timeInterval.end = end;
            if (!uls.timeInterval.IsValid()) {
              uls.timeInterval.Print(cout);
              cout << " is an invalid interval" << endl;
              SetDefined(false);
              return;
            }
          }
          if ((m == seq.first[seqPos]) && !assigns[i].getText(4).empty()) {
            uls.timeInterval.lc = lc;
          }
          if ((m == seq.first[seqPos+1] - 1) && !assigns[i].getText(5).empty()){
            uls.timeInterval.rc = rc;
          }
          MergeAdd(uls);
        }
      }
      seqPos = seqPos + 2;
    }
    else { // variable does not occur in p
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
      MergeAdd(uls);
    }
  }
  SetDefined(IsValid());
}

/*
\subsubsection{Creation of the type constructor ~mlabel~}

*/
TypeConstructor movinglabel(
    MLabel::BasicType(), // name
    MLabel::MLabelProperty,    //property function describing signature
    //Out and In functions
    OutMapping<MLabel, UString, OutConstTemporalUnit<CcString, OutCcString> >,
    InMapping<MLabel, UString, InConstTemporalUnit<CcString, InCcString> >,
    0,
    0,  //SaveToList and RestoreFromList functions
    CreateMapping<MLabel>,
    DeleteMapping<MLabel>,     //object creation and deletion
    OpenAttribute<MLabel>,
    SaveAttribute<MLabel>,  // object open and save
    CloseMapping<MLabel>,
    CloneMapping<MLabel>,  //object close and clone
    CastMapping<MLabel>,   //cast function
    SizeOfMapping<MLabel>, //sizeof function
    MLabel::CheckMLabel    //kind checking function
);


template <class Mapping, class Alpha>
int MappingAtInstantExt(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
  result = qp->ResultStorage(s);
  Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;
  ((Mapping*)args[0].addr)->AtInstant(*((Instant*)args[1].addr), *pResult);
  return 0;
}

const string TemporalSpecAtInstantExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T in {label},\n"
    "mT x instant  -> iT</text--->"
    "<text>_ atinstant _ </text--->"
    "<text>get the Intime value corresponding to the instant.</text--->"
    "<text>mlabel1 atinstant instant1</text--->"
    ") )";
    
int MovingExtSimpleSelect(ListExpr args) {
  ListExpr arg1 = nl->First(args);
  if (nl->SymbolValue(arg1) == MLabel::BasicType()) {
    return 0;
  }
  return -1; // This point should never be reached
}

ListExpr MovingInstantExtTypeMapIntime(ListExpr args) {
  if (nl->ListLength(args) == 2) {
    ListExpr arg1 = nl->First(args),
    arg2 = nl->Second(args);
    if(nl->IsEqual(arg2, Instant::BasicType())) {
      if(nl->IsEqual(arg1, MLabel::BasicType()))
        return nl->SymbolAtom(Intime<Label>::BasicType());
    }
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ValueMapping temporalatinstantextmap[] = {
    MappingAtInstantExt<MString, CcString> };
//    MappingAtInstantExt<MLabel, Label> };

Operator temporalatinstantext(
    "atinstant",
    TemporalSpecAtInstantExt,
    5,
    temporalatinstantextmap,
    MovingExtSimpleSelect,
    MovingInstantExtTypeMapIntime);

int CompareLabels(const void *a, const void *b) {
  const Label *label1 = (const Label*)a;
  const Label *label2 = (const Label*)b;
  if (label1->GetValue().compare(label2->GetValue()) > 0) {
    return 1;
  }
  if (label1->GetValue().compare(label2->GetValue()) < 0) {
    return -1;
  }
  return 0;
}

enum LabelsState {partial, complete};

class Labels : public Attribute {
  public:
    Labels(const int n, const Label *Lb = 0);
    ~Labels();

    Labels(const Labels& src);
    Labels& operator=(const Labels& src);

    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);
    int Compare(const Attribute*) const;
    bool Adjacent(const Attribute*) const;
    Labels *Clone() const;
    size_t Sizeof() const;
    ostream& Print(ostream& os) const;

    void Append( const Label &lb );
    void Complete();
    bool Correct();
    void Destroy();
    int GetNoLabels() const;
    Label GetLabel(int i) const;
    string GetState() const;
    const bool IsEmpty() const;
    void CopyFrom(const Attribute* right);
    size_t HashValue() const;

    friend ostream& operator <<( ostream& os, const Labels& p );

    static Word     In(const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct);
    static ListExpr Out(ListExpr typeInfo, Word value);
    static Word     Create(const ListExpr typeInfo);
    static void     Delete(const ListExpr typeInfo, Word& w);
    static void     Close(const ListExpr typeInfo, Word& w);
    static bool     Save(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value);
    static bool     Open(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value);
    static Word     Clone(const ListExpr typeInfo, const Word& w);
    static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
    static int      SizeOfObj();
    static ListExpr Property();
    static void*    Cast(void* addr);
    static const    string BasicType() {
      return "labels";
    }
    static const    bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }
    DbArray<Label> GetDbArray() {return labels;}
    void Sort() {labels.Sort(CompareLabels);}
    void Clean() {
      if (labels.Size()) {
        labels.clean();
      }
      state = partial;
    }

  private:
    Labels() {} // this constructor is reserved for the cast function.
    DbArray<Label> labels;
    LabelsState state;
};

/*
2.3.18 Print functions

*/
ostream& operator<<(ostream& os, const Label& lb) {
  os << "(" << lb << ")";
  return os;
}

ostream& operator<<(ostream& os, const Labels& lbs) {
  os << " State: " << lbs.GetState() << "<";
  for(int i = 0; i < lbs.GetNoLabels(); i++)
    os << lbs.GetLabel(i) << " ";
  os << ">";
  return os;
}

/*
2.3.1 Constructors.

This first constructor creates a new Labels object.

*/
Labels::Labels(const int n, const Label *lb) :
  Attribute(true),
  labels(n),
  state(partial) {
  SetDefined(true);
  if(n > 0) {
    for(int i = 0; i < n; i++) {
      Append(lb[i]);
    }
    Complete();
  }
}

/*
2.3.2 Copy Constructor

*/
Labels::Labels(const Labels& src):
  Attribute(src.IsDefined()),
  labels(src.labels.Size()),state(src.state) {
  labels.copyFrom(src.labels);
}

/*
2.3.2 Destructor.

*/
Labels::~Labels() {}

Labels& Labels::operator=(const Labels& src) {
  this->state = src.state;
  labels.copyFrom(src.labels);
  return *this;
}

/*
2.3.3 NumOfFLOBs.

*/
int Labels::NumOfFLOBs() const {
  return 1;
}

/*
2.3.4 GetFLOB

*/
Flob *Labels::GetFLOB(const int i) {
  assert( i >= 0 && i < NumOfFLOBs() );
  return &labels;
}

/*
2.3.5 Compare

Not yet implemented. 

*/
int Labels::Compare(const Attribute* arg) const {
  return 0;
}

/*
2.3.6 HashValue

Because Compare returns alway 0, we can only return a constant hash value.

*/
size_t Labels::HashValue() const {
  return  1;
}

/*
2.3.5 Adjacent

Not yet implemented. 

*/
bool Labels::Adjacent(const Attribute*) const {
  return 0;
}

/*
2.3.7 Clone

Returns a new created element labels (clone) which is a
copy of ~this~.

*/
Labels *Labels::Clone() const {
  assert(state == complete);
  Labels *lbs = new Labels(*this);
  return lbs;
}

void Labels::CopyFrom(const Attribute* right) {
  *this = *((Labels*) right);
}

/*
2.3.8 Sizeof

*/
size_t Labels::Sizeof() const {
  return sizeof( *this );
}

/*
2.3.8 Print

*/
ostream& Labels::Print( ostream& os ) const {
  return (os << *this);
}

/*
2.3.9 Append

Appends a label ~lb~ at the end of the DBArray labels.

*Precondition* ~state == partial~.

*/
void Labels::Append(const Label& lb) {
  assert(state == partial);
  labels.Append( lb );
}

/*
2.3.10 Complete

Turns the element labels into the ~complete~ state.

*Precondition* ~state == partial~.

*/
void Labels::Complete() {
  assert(state == partial);
  state = complete;
}

/*
2.3.11 Correct

Not yet implemented.

*/
bool Labels::Correct() {
  return true;
}

/*
2.3.13 Destroy

Turns the element labels into the ~closed~ state destroying the
labels DBArray.

*Precondition* ~state == complete~.

*/
void Labels::Destroy() {
  state = complete;
  // assert( state == complete );
  labels.destroy();
}

/*
2.3.14 NoLabels

Returns the number of labels of the DBArray labels.

*Precondition* ~state == complete~.

*/
int Labels::GetNoLabels() const {
  return labels.Size();
}

/*
2.3.15 GetLabel

Returns a label indexed by ~i~.

*Precondition* ~state == complete \&\& 0 <= i < noLabels~.

*/
Label Labels::GetLabel(int i) const {
  assert(state == complete);
  assert(0 <= i && i < GetNoLabels());
  Label lb(true);
  labels.Get(i, &lb);
  return lb;
}

/*
2.3.16 GetState

Returns the state of the element labels in string format.

*/
string Labels::GetState() const {
  switch(state) {
    case partial:
      return "partial";
    case complete:
      return "complete";
  }
  return "";
}

/*
2.3.18 IsEmpty

Returns if the labels is empty or not.

*/
const bool Labels::IsEmpty() const {
  assert(state == complete);
  return GetNoLabels() == 0;
}

/*
3 Labels Algebra.

3.1 List Representation

The list representation of a labels is

----    ( (<recordId>) label_1 label_2 ... label_n )
----

3.2 ~In~ and ~Out~ Functions

*/

ListExpr Labels::Out(ListExpr typeInfo, Word value) {
  Labels* labels = static_cast<Labels*>(value.addr);
  if (!labels->IsDefined()) {
    return (NList(Symbol::UNDEFINED())).listExpr();
  }
  if (labels->IsEmpty()) {
    return (NList()).listExpr();
  }
  else {
    NList element("", true);
    for (int i = 0; i < labels->GetNoLabels(); i++) {
      element.append(NList((labels->GetLabel(i)).GetValue(), true));
    }
    return element.listExpr();    
  }
}

Word Labels::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word result = SetWord(Address(0));
  correct = false;
  NList list(instance); 
  Labels* labels = new Labels(0);
  labels->SetDefined(true);
  while (!list.isEmpty()) {
    if (!list.isAtom() && list.first().isAtom() && list.first().isString()) {
      Label label(true, list.first().str());
      labels->Append(label);
      correct = true;
    }
    else {
      correct = false;
      cmsg.inFunError("Expecting a list of string atoms!");
      delete labels;
      return SetWord(Address(0));
    }
    list.rest();
  }
  labels->labels.Sort(CompareLabels);
  labels->Complete();  
  result.addr = labels;
  return result;
}

/*
3.3 Function Describing the Signature of the Type Constructor

*/
ListExpr Labels::Property() {
  return (nl->TwoElemList(
         nl->FiveElemList(nl->StringAtom("Signature"),
                          nl->StringAtom("Example Type List"),
                          nl->StringAtom("List Rep"),
                          nl->StringAtom("Example List"),
                          nl->StringAtom("Remarks")),
         nl->FiveElemList(nl->StringAtom("->" + Kind::DATA() ),
                          nl->StringAtom(Labels::BasicType()),
                          nl->StringAtom("(<label>*) where <label> is "
                          "a string"),
                          nl->StringAtom("( \"at home\" \"at work\" \"cinema\" "
                          "\"at home\" )"),
                          nl->StringAtom("Each label must be of "
                          "type string."))));
}

/*
3.4 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~labels~ does not have arguments, this is trivial.

*/
bool Labels::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Labels::BasicType()));
}

/*

3.5 ~Create~-function

*/
Word Labels::Create(const ListExpr typeInfo) {
  Labels* labels = new Labels(0);
  return (SetWord(labels));
}

/*
3.6 ~Delete~-function

*/
void Labels::Delete(const ListExpr typeInfo, Word& w) {
  Labels* labels = (Labels*)w.addr;
  labels->Destroy();
  delete labels;
}

/*
3.6 ~Open~-function

*/
bool Labels::Open(SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value) {
  Labels *lbs = (Labels*)Attribute::Open(valueRecord, offset, typeInfo);
  value.setAddr(lbs);
  return true;
}

/*
3.7 ~Save~-function

*/
bool Labels::Save(SmiRecord& valueRecord, size_t& offset,
             const ListExpr typeInfo, Word& value) {
  Labels *lbs = (Labels *)value.addr;
  Attribute::Save(valueRecord, offset, typeInfo, lbs);
  return true;
}

/*
3.8 ~Close~-function

*/
void Labels::Close(const ListExpr typeInfo, Word& w) {
  Labels* labels = (Labels*)w.addr;
  delete labels;
}

/*
3.9 ~Clone~-function

*/
Word Labels::Clone(const ListExpr typeInfo, const Word& w) {
  return SetWord(((Labels*)w.addr)->Clone());
}

/*
3.9 ~SizeOf~-function

*/
int Labels::SizeOfObj() {
  return sizeof(Labels);
}

/*
3.10 ~Cast~-function

*/
void* Labels::Cast(void* addr) {
  return (new (addr)Labels);
}

/*
3.11 Creation of the Type Constructor Instance

*/
TypeConstructor labelsTC(
        Labels::BasicType(),               //name
        Labels::Property,                  //property function
        Labels::Out,   Labels::In,        //Out and In functions
        0,              0,                  //SaveTo and RestoreFrom functions
        Labels::Create,  Labels::Delete,  //object creation and deletion
        Labels::Open,    Labels::Save,    //object open and save
        Labels::Close,   Labels::Clone,   //object close and clone
        Labels::Cast,                      //cast function
        Labels::SizeOfObj,                 //sizeof function
        Labels::KindCheck );               //kind checking function

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
  string source = static_cast<T*>(args[0].addr)->GetValue();
  result = qp->ResultStorage(s);
  Label* res = new Label(true, source);
  result.addr = res;
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
  MString *source = static_cast<MString*>(args[0].addr);
  result = qp->ResultStorage(s);
  MLabel* res = new MLabel(0);
  res->convertFromMString(source);
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

\subsection{Type Mapping}

*/
ListExpr containsTM(ListExpr args) {
  const string errMsg = "Expecting a labels and a string.";
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  if (Labels::checkType(nl->First(args))
   && CcString::checkType(nl->Second(args))) {
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Value Mapping}

*/
int containsVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  Labels *labels = static_cast<Labels*>(args[0].addr);
  CcString* ccstr = static_cast<CcString*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* ccbool = static_cast<CcBool*>(result.addr);
  Label *label = new Label(true, ccstr->GetValue());
  int pos;
  bool res = labels->GetDbArray().Find(label, CompareLabels, pos);
  ccbool->Set(true, res);
  return 0;
}

/*
\subsection{Operator Info}

*/
struct containsInfo : OperatorInfo {
  containsInfo() {
    name      = "contains";
    signature = "labels x string -> bool";
    syntax    = "_ contains _;";
    meaning   = "Checks whether a Labels object contains a string.";
  }
};


/*
\section{Pattern}
*/

/*
\subsection{Function ~toString~}
Writes all pattern information into a string.

*/
string Pattern::toString() const {
  stringstream str;
  if (description != "") {
    str << "=====description=====" << endl << description << endl;
  }
  str << "==pattern elements==" << endl;
  for (int i = 0; i < (int)elems.size(); i++) {
    str << "[" << i << "] " << elems[i].getV() << " | "
        << setToString(elems[i].getI()) << " | "
        << setToString(elems[i].getL()) << " | "
        << (elems[i].getW() ? (elems[i].getW() == STAR ? "*" : "+") :"")
        << endl;
  }
  str << "=====conditions======" << endl;
  for (int i = 0; i < (int)conds.size(); i++) {
    str << "[" << i << "] " << conds[i].toString() << endl;
  }
  str << "=results/assignments=" << endl;
  for (int i = 0; i < (int)assigns.size(); i++) {
    str << "[" << i << "] " << assigns[i].getV() << " | ";
    for (int j = 0; j < 4; j++) {
      str << assigns[i].getText(j) << "; ";
      for (int k = 0; k < assigns[i].getRightSize(j); k++) {
        str << "(" << assigns[i].getVarKey(j, k).first << ", "
            << assigns[i].getVarKey(j, k).second << ") ";
      }
      str << " # ";
    }
    str << endl;
  }
  str << endl;
  return str.str();
}



/*
\subsection{Function ~GetText()~}

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
  delete static_cast<Pattern*>(w.addr);
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
  Pattern* p = (Pattern*)value.addr;
  int size = p->elems.size();
  valueRecord.Write(&size, sizeof(int), offset);
  offset += sizeof(int);
  for (int i = 0; i < size; i++) {
    valueRecord.Write(&p->elems[i], sizeof(UPat), offset);
    offset += sizeof(UPat);
  }
  size = p->assigns.size();
  valueRecord.Write(&size, sizeof(int), offset);
  offset += sizeof(int);
  for (int i = 0; i < size; i++) {
    valueRecord.Write(&p->assigns[i], sizeof(Assign), offset);
    offset += sizeof(Assign);
  }
  size = p->conds.size();
  valueRecord.Write(&size, sizeof(int), offset);
  offset += sizeof(int);
  for (int i = 0; i < size; i++) {
    valueRecord.Write(&p->conds[i], sizeof(Condition), offset);
    offset += sizeof(Condition);
  }
  size = p->text.length();
  valueRecord.Write(&size, sizeof(int), offset);
  offset += sizeof(int);
  for (int i = 0; i < size; i++) {
    valueRecord.Write(&p->text[i], sizeof(char), offset); // text
    offset += sizeof(char);
  }
  size = p->elems.size();
  valueRecord.Write(&size, sizeof(int), offset); // delta.size
  offset += sizeof(int);
  int mapsize, setsize;
  set<int>::iterator k;
  for (int i = 0; i < size; i++) {
    mapsize = p->delta[i].size();
    valueRecord.Write(&mapsize, sizeof(int), offset); //mapsize
    offset += sizeof(int);
    for (int j = 0; j < mapsize; j++) {
      setsize = p->delta[i][j].size();
      valueRecord.Write(&setsize, sizeof(int), offset);//setsize
      offset += sizeof(int);
      for (k = p->delta[i][j].begin(); k != p->delta[i][j].end(); k++) {
        valueRecord.Write(&(*k), sizeof(int), offset);
        offset += sizeof(int);
      }
    }
  }
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
\subsection{Function ~verifyPattern~}

Loops through the pattern elements and checks whether every specified interval
is valid and whether the variables are unique.

*/
bool Pattern::verifyPattern() const {
  set<string>::iterator it;
  SecInterval iv;
  set<string> vars, ivs;
  if (!this) {
    cout << "Error: Pattern not initialized." << endl;
    return false;
  }
  for (int i = 0; i < (int)elems.size(); i++) {
    ivs = elems[i].getI();
    for (it = ivs.begin(); it != ivs.end(); it++){
      if ((*it).at(0) >= 65 && (*it).at(0) <= 122
        && !checkSemanticDate(*it, iv, false)) {
        return false;
      }
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

bool Pattern::parseNFA() {
  IntNfa* intNfa = 0;
  cout << "RegEx = \'" << regEx << "\'" << endl;
  if (parsePatternRegEx(regEx.c_str(), &intNfa) != 0) {
    return false;
  }
  intNfa->nfa.makeDeterministic();
  intNfa->nfa.minimize();
  intNfa->nfa.bringStartStateToTop();
//   intNfa->nfa.print(cout);
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
  cout << "==========================================" << endl;
  printNfa();
  cout << "==========================================" << endl;
  return true;
}

/*
\subsection{Function ~matches~}

Checks the pattern and the condition and (if no problem occurs) invokes the NFA
construction and the matching procedure.

*/
ExtBool Pattern::matches(MLabel &ml) {
  if (!isVerified()) {
    if (!verifyPattern()) {
      cout << "Error: Invalid pattern." << endl;
      return FALSE;
    }
  }
/*  cout << nfa2String() << endl;*/
  Match *match = new Match(elems.size() + 1);
  match->copyFromPattern(this);
  ExtBool result = UNDEF;
  if (match->initEasyCondOpTrees()) {
    result = match->matches(ml);
  }
  ml.index.removeTrie();
  match->deleteEasyCondOpTrees();
  delete match;
  return result;
}

/*
\subsection{Function ~getRewriteSeqs~}

Performs a match and returns the set of matching sequences for the operator
~rewrite~.

*/
set<pair<vector<unsigned int>, vector<unsigned int> > > Pattern::
                                            getRewriteSeqs(MLabel &ml) {
  set<pair<vector<unsigned int>, vector<unsigned int> > > result;
  Match *match = new Match(elems.size() + 1);
  match->copyFromPattern(this);
  match->setAssVars(this->getAssVars());
  match->setVarPos(this->getVarPos());
//   cout << nfa2String() << endl;
  match->initEasyCondOpTrees();
  if (!match->matches(ml, true)) {
    match->deleteEasyCondOpTrees();
    delete match;
    return result;
  }
  match->deleteEasyCondOpTrees();
  match->computeResultVars(this->assigns);
  match->buildSequences();
//   match->printSequences(300);
  match->filterSequences(ml);
//   match->printRewriteSeqs(50);
  match->deleteCondOpTrees();
  result = match->getRewriteSeqs();
  delete match;
  return result;
}

map<string, int> Pattern::getVarPosInSeq() {
  set<string>::iterator it;
  int pos = 0;
  map<string, int> result;
  for (it = assignedVars.begin(); it != assignedVars.end(); it++) {
    result[*it] = pos;
    pos = pos + 2;
  }
//   cout << endl << "varPosInSeq=";
//   map<string, int>::iterator ite;
//   for (ite = result.begin(); ite != result.end(); ite++) {
//     cout << (*ite).first << "|" << (*ite).second << "--";
//   }
//   cout << endl;
  return result;
}

Match::Match(IndexLI* li, TupleId tId) {
  p = li->p;
  f = p->getSize();
  match = new set<unsigned int>[f];
  cardsets = new set<unsigned int>[f];
  seqOrder = new int[f];
  numOfLabels = li->getMLsize(tId);
  for (int i = 0; i < f; i++) {
    match[i].insert(li->matches[tId-1][i].begin(), li->matches[tId-1][i].end());
  }
}

/*
\subsection{Function ~computeResultVars~}

Computes a mapping containing the positions of the pattern elements the result
variables belong to.

*/
void Match::computeResultVars(vector<Assign> assigns) {
  resultVars.clear();
  for (int i = 0; i < (int)assigns.size(); i++) {
    resultVars[i] = assigns[i].getPatternPos();
  }
}

/*
\subsection{Function ~filterSequences~}

Searches for sequences which fulfill all conditions and stores their relevant
parts for rewriting.

*/
void Match::filterSequences(MLabel const &ml) {
  set<multiset<unsigned int> >::iterator it;
  for (it = sequences.begin(); it != sequences.end(); it++) {
    for (unsigned int i = 0; i < p->conds.size(); i++) {
      if (!evaluateCond(ml, p->conds[i], *it)) {
        i = p->conds.size(); // continue with next sequence
      }
      else if (i == p->conds.size() - 1) { // all conditions are fulfilled
        buildRewriteSeq(*it);
      }
    }
    if (p->conds.empty()) {
      buildRewriteSeq(*it);
    }
  }
}

void Match::buildRewriteSeq(multiset<unsigned int> sequence) {
  vector<unsigned int> seq(sequence.begin(), sequence.end());
  vector<unsigned int> rewriteSeq, assignedSeq;
  pair<vector<unsigned int>, vector<unsigned int> > completeSeq;
  for (unsigned int j = 0; j < resultVars.size(); j++) {
    if (resultVars[j] > -1) {
      rewriteSeq.push_back(seq[resultVars[j]]); // begin
      if (resultVars[j] < (int)(f - 1)) {
        rewriteSeq.push_back(seq[resultVars[j] + 1]); // end
      }
      else { // last state
        rewriteSeq.push_back(numOfLabels);
      }
    }
  }
  set<string>::iterator it; // for assigned variables (... := X...)
  for (it = assignedVars.begin(); it != assignedVars.end(); it++) {
    assignedSeq.push_back(seq[varPos[*it]]);
    if (varPos[*it] < f - 1) {
      assignedSeq.push_back(seq[varPos[*it] + 1]);
    }
    else { // last state
      assignedSeq.push_back(numOfLabels);
    }
  }
  completeSeq.first = rewriteSeq;
  completeSeq.second = assignedSeq;
  rewriteSeqs.insert(completeSeq);
}

/*
\subsection{Function ~buildNFA~}

Reads the pattern and generates the delta function

*/
void Pattern::buildNFA() {
  int f = elems.size();
  int prev[3] = {-1, -1, -1}; // prevStar, prevNotStar, secondPrevNotStar
  for (int i = 0; i < f; i++) {
    delta[i][i].insert(i + 1); // state i, read pattern i => new state i+1
    if (!elems[i].getW() || !elems[i].getI().empty() // last pattern or
     || !elems[i].getL().empty() || (i == f - 1)) { // any pattern except +,*
      if ((prev[0] == i - 1) || (i == f - 1)) { // '...* #(1 a)...'
        for (int j = prev[1] + 1; j < i; j++) {
          delta[j][i].insert(i + 1); // '* * * #(1 a ) ...'
          for (int k = j; k <= i; k++) {
            delta[j][k].insert(j);
            for (int m = j; m <= i; m++) {
              delta[j][k].insert(m); // step 1
            }
            if ((elems[i].getW() == STAR) && (i == f - 1)) { // end
              delta[j][k].insert(f);
            }
          }
        }
        if (prev[1] >= 0) { // match before current pattern
          for (int j = prev[1] + 1; j <= i; j++) {
            delta[prev[1]][prev[1]].insert(j); // step 2
          }
          if ((elems[i].getW() == STAR) && (i == f - 1)) { // end
            delta[prev[1]][prev[1]].insert(f);
          }
        }
        if (prev[2] < prev[1] - 1) { // '* ... * (1 a) * ... * #(2 b) ...'
          for (int j = prev[2] + 1; j < prev[1]; j++){
            for (int k = prev[1] + 1; k <= i; k++) {
              delta[j][prev[1]].insert(k); // step 3
            }
            if ((elems[i].getW() == STAR) && (i == f - 1)) { // end
              delta[j][prev[1]].insert(f);
            }
          }
        }
      }
      if (elems[i].getW() == PLUS) {
        delta[i][i].insert(i);
      }
      prev[2] = prev[1];
      prev[1] = i;
    }
    else if (elems[i].getW() == STAR) { // reading '*'
      prev[0] = i;
    }
    else if (elems[i].getW() == PLUS) { // reading '+'
      delta[i][i].insert(i);
      prev[2] = prev[1];
      prev[1] = i;
    }
  }
  if (elems[f - 1].getW()) { // '... #*' or '... #+'
    delta[f - 1][f - 1].insert(f - 1);
  }
  setVerified(true);
}

/*
\subsection{Function ~match~}

Loops through the MLabel calling updateStates() for every ULabel. True is
returned if and only if the final state is an element of currentStates after
the loop. If ~rewrite~ is true (which happens in case of the operator ~rewrite~)
the matching procedure ends after the unit pattern test.

*/
ExtBool Match::matches(MLabel &ml, bool rewrite) {
  numOfLabels = (size_t)ml.GetNoComponents();
  MatchElem** matching = 0;
  if (ml.hasIndex()) { // use index
    ml.index.initRoot();
    set<size_t> positions;
    positions.insert(0);
    for (int i = 0; i < f; i++) {
      positions = updatePositionsIndex(ml, i, positions);
      if (positions.empty()) {
        return FALSE;
      }
    }
    if (!positions.count(ml.GetNoComponents())) { // final ml position inactive?
      return FALSE;
    }
  }
  else { // no index => process whole mlabel
    set<int> states;
    states.insert(0);
    if (p->getConds().empty() && !rewrite) {
      for (size_t i = 0; i < numOfLabels; i++) {
        ml.Get(i, ul);
        ulId = i;
        updateStates2(ml);
        if (!updateStates(ml, i, states)) {
  //  TODO: return FALSE;
        }
        if (currentStates.empty()) {
          return FALSE;
        }
      }
    }
    else {
      matching = create2DimArray<MatchElem>(ml.GetNoComponents(), p->getSize());
      for (size_t i = 0; i < numOfLabels; i++) {
        ml.Get(i, ul);
        ulId = i;
        updateStates2(ml);
        if (!updateStates(ml, i, states/*, matching*/)) {
  //  TODO: return FALSE;
        }
        if (currentStates.empty()) {
          delete2DimArray<MatchElem>(matching, ml.GetNoComponents());
          return FALSE;
        }
      }
    }
//     printCards();
    if (!numOfLabels) { // empty MLabel
      int pos = 0;
      while (p->elems[pos].getW() == STAR) {
        currentStates.insert(pos + 1);
        pos++;
      }
    }
    if (!currentStates.count(f)) { // is the final state inactive?
      delete2DimArray<MatchElem>(matching, ml.GetNoComponents());
      return FALSE;
    }
  }
  if (rewrite) {
    if (!numOfLabels) {
      cout << "no rewriting for an empty MLabel." << endl;
      delete2DimArray<MatchElem>(matching, ml.GetNoComponents());
      return FALSE;
    }
    computeCardsets();
    delete2DimArray<MatchElem>(matching, ml.GetNoComponents());
    if (!initCondOpTrees()) {
      return UNDEF;
    }
    return TRUE;
  }
  if (p->conds.size()) {
    computeCardsets();
    delete2DimArray<MatchElem>(matching, ml.GetNoComponents());
    if (!initCondOpTrees(true)) {
      return UNDEF;
    }
    if (!conditionsMatch(ml)) {
      deleteCondOpTrees();
      return FALSE;
    }
    deleteCondOpTrees();
  }
  return TRUE;
}

/*
\subsection{Function ~printNfa~}

*/
void Pattern::printNfa() {
  map<int, int>::iterator it;
  for (unsigned int i = 0; i < nfa.size(); i++) {
    cout << (finalStates.count(i) ? " * " : "   ") << "state " << i << ":  ";
    for (it = nfa[i].begin(); it != nfa[i].end(); it++) {
      cout << "---" << it->first << "---> " << it->second << "    ";
    }
    cout << endl << endl;
  }
}

/*
\subsection{Function ~nfa2String~}

Returns a string displaying the information stored in the NFA.

*/
string Pattern::nfa2String() const {
  stringstream nfa;
  set<int>::iterator k;
  for (int i = 0; i < (int)elems.size(); i++) {
    for (int j = i; j < (int)elems.size(); j++) {
      map<int, set<int> > tempmap = delta[i];
      set<int> tempset = tempmap[j];
      if (tempset.size() > 0) {
        nfa << "state " << i << " | upat #" << j << " | new states {";
        if (tempset.size() == 1) {
          nfa << *(tempset.begin());
        }
        else {
          for (k = tempset.begin();
               k != tempset.end(); k++) {
            nfa << *k << " ";
          }
        }
        nfa << "}" << endl;
      }
    }
  }
  return nfa.str();
}

/*
\subsection{Function ~printCurrentStates~}

Prints the set of currently active states.

*/
void Match::printCurrentStates() {
  if (!currentStates.empty()) {
    set<int>::iterator it = currentStates.begin();
    cout << "after ULabel # " << ulId << ", active states are {" << *it;
    it++;
    while (it != currentStates.end()) {
      cout << ", " << *it;
      it++;
    }
    cout << "}" << endl;
  }
  else {
    cout << "after ULabel # " << ulId << ", there is no active state" << endl;
  }
}

/*
\subsection{Function ~printCards~}

Prints the ulabels matched by every unit pattern. Subsequently, the possible
cardinalities for every unit pattern are displayed.

*/
void Match::printCards() {
  set<unsigned int>::iterator it;
  for (int j = 0; j < f; j++) {
    cout << "upat " << j << " matches ulabels ";
    for (it = match[j].begin(); it != match[j].end(); it++) {
      cout << *it << ", ";
    }
    cout << endl;
  }
  for (int i = 0; i < f; i++) {
    cout << i << " | ";
    for (it = cardsets[i].begin(); it != cardsets[i].end(); it++) {
      cout << *it << ",";
    }
    cout << endl;
  }
}

/*
\subsection{Function ~printSequences~}

Displays the possible cardinality sequences. As the number of sequences may
be very high, only the first ~max~ sequences are printed.

*/
void Match::printSequences(unsigned int max) {
  set<multiset<unsigned int> >::iterator it1;
  set<unsigned int>::iterator it2;
  unsigned int seqCount = 0;
  it1 = sequences.begin();
  while ((seqCount < max) && (it1 != sequences.end())) {
    cout << "seq_" << (seqCount < 9 ? "0" : "") << seqCount  + 1 << " | ";
    for (it2 = (*it1).begin(); it2 != (*it1).end(); it2++) {
      cout << (*it2) << ", ";
    }
    cout << endl;
    it1++;
    seqCount++;
  }
  cout << "there are " << sequences.size() << " possible sequences" << endl;
}

/*
\subsection{Function ~printRewriteSeqs~}

Displays the sequences for rewriting. As the number of sequences may be very
high, only the first ~max~ sequences are printed.

*/
void Match::printRewriteSeqs(unsigned int max) {
  set<pair<vector<unsigned int>, vector<unsigned int> > >::iterator it;
  unsigned int seqCount = 0;
  it = rewriteSeqs.begin();
  while ((seqCount < max) && (it != rewriteSeqs.end())) {
    cout << "result_" << (seqCount < 9 ? "0" : "") << seqCount  + 1 << " | ";
    for (unsigned int i = 0; i < (*it).first.size(); i++) {
      cout << (*it).first[i] << ", ";
    }
    cout << endl;
    it++;
    seqCount++;
  }
  cout << "there are " << rewriteSeqs.size() << " result sequences" << endl;
}

/*
\subsection{Function ~updateStates~}

Applies the NFA.

*/
bool Match::updateStates(MLabel const &ml, size_t ulId, set<int> &states) {
//   cout << "old states: ";
  set<int>::iterator its;
  set<short int>::iterator iti;
  map<int, int> transitions;
  for (its = states.begin(); its != states.end(); its++) {
//     cout << *its << " ";
    map<int, int> trans = p->getTransitions(*its);
    transitions.insert(trans.begin(), trans.end());
  }
  if (transitions.empty()) {
    return false;
  }
  states.clear();
  map<int, int>::iterator itm;
//   cout << "|||| new states: ";
  for (itm = transitions.begin(); itm != transitions.end(); itm++) {
//     cout << itm->second << " ";
    if (labelsMatch(ul.constValue.GetValue(), p->elems[itm->first].getL())
     && timesMatch(&ul.timeInterval, p->elems[itm->first].getI())
     && easyCondsMatch(ml, p->elems[itm->first])) {
      states.insert(itm->second);

    }
  }
//   cout << endl;
  return !states.empty();
}

/*
\subsection{Function ~updateStates~}

Further functions are invoked to decide which transition can be applied to
which current state. The set of current states is updated.

*/
void Match::updateStates2(MLabel const &ml) {
  set<int> newStates;
  set<int>::iterator i, k;
  map<int, set<int> >::iterator j;
  for (i = currentStates.begin(); (i != currentStates.end() && *i < f); i++) {
    for (j = p->delta[*i].begin(); j != p->delta[*i].end(); j++) {
      if (labelsMatch(ul.constValue.GetValue(), p->elems[j->first].getL())
       && timesMatch(&ul.timeInterval, p->elems[j->first].getI())
       && easyCondsMatch(ml, p->elems[j->first])) {
        if (!p->elems[j->first].getW() || !p->elems[j->first].getI().empty()
         || !p->elems[j->first].getL().empty()) {//(_ a), ((1 _)), () or similar
          match[j->first].insert(ulId);
        }
        newStates.insert(j->second.begin(), j->second.end());
      }
    }
  }
  currentStates = newStates;  
}

/*
\subsection{Function ~updatePositionsIndex~}

*/
set<size_t> Match::updatePositionsIndex(MLabel &ml, int pPos,set<size_t> mlpos){
  set<size_t> result, foundpos;
  set<size_t>::iterator j;
  set<string> labels = p->elems[pPos].getL();;
  set<string>::iterator i = labels.begin();
  Wildcard w = p->elems[pPos].getW();
  if (*(mlpos.rbegin()) == (size_t)ml.GetNoComponents()) { // erase final state
    mlpos.erase(*(mlpos.rbegin()));
  }
  if (w == NO) { // () or (...)
    while (i != labels.end()) {
      foundpos = ml.index.find(*i);
      for (j = foundpos.begin(); j != foundpos.end(); j++) {
        ml.Get(*j, ul);
        if (mlpos.count(*j)
         && timesMatch(&ul.timeInterval, p->elems[pPos].getI())
         && easyCondsMatch(ml, p->elems[pPos])) {
          result.insert(*j + 1);
          match[pPos].insert(*j);
        }
      }
      i++;
    }
    if (labels.empty()) {
      for (j = mlpos.begin(); j != mlpos.end(); j++) {
        ml.Get(*j, ul);
        if (timesMatch(&ul.timeInterval, p->elems[pPos].getI())
          && easyCondsMatch(ml, p->elems[pPos])) {
          result.insert(*j + 1);
          match[pPos].insert(*j);
        }
      }
    }
  }
  else if (labels.empty() && p->elems[pPos].getI().empty()) { // +, * or (())
    j = result.begin();
    for (size_t k = (w == STAR ? *(mlpos.begin()) : *(mlpos.begin()) + 1);
           (int)k <= ml.GetNoComponents(); k++) {
      result.insert(j, k); // efficient insertion
      j++;
    }
  }
  else { // ((...)), non-empty
    bool ok = true;
    size_t k;
    while (i != labels.end()) {
      foundpos = ml.index.find(*i);
      for (j = foundpos.begin(); j != foundpos.end(); j++) {
        ml.Get(*j, ul);
        if (timesMatch(&ul.timeInterval, p->elems[pPos].getI())
         && (mlpos.count(*j) || result.count(*j))) {
          result.insert(*j + 1);
        }
      }
      i++;
    }
    if (labels.empty()) {
      j = mlpos.begin();
      ok = true;
      while (j != mlpos.end()) {
        k = *j;
        while (ok && (k < (size_t)ml.GetNoComponents())) {
          ml.Get(k, ul);
          if (timesMatch(&ul.timeInterval, p->elems[pPos].getI())) {
            result.insert(k + 1);
            k++;
          }
          else {
            ok = false;
          }
        }
        do { // find next relevant position
          j++;
        } while ((j != mlpos.end()) && (*j <= k));
      }
    }
  }
  return result;
}

/*
\subsection{Function ~processDoublePars~}

Computes the set of possible cardinalities for sequence patterns in double
parentheses and stores their positions into a set.

*/
void Match::processDoublePars(int pos) {
  set<unsigned int>::iterator j;
  size_t last = -2;
  size_t count = 0;
  for (j = match[pos].begin(); j != match[pos].end(); j++) {
    if (*j == last + 1) {
      count++;
      cardsets[pos].insert(count);
    }
    else {
      count = 1;
    }
    last = *j;
  }
  if (!p->elems[pos].getI().empty() || !p->elems[pos].getL().empty()) {
    doublePars.insert(pos);
  }
}

/*
\subsection{Function ~computeCardsets~}

Computes the set of possible cardinalities for every state.

*/
void Match::computeCardsets() {
  set<unsigned int>::iterator j, k;
  int prev = -1; // previous matching position
  int numOfNonStars(0), numOfW(0);
  size_t limit;
  for (int i = 0; i < f; i++) {
    if (match[i].size()) {
      cardsets[i].insert(1);
      if (p->elems[i].getW() == PLUS) { // '... #((1 a)) ...'
        processDoublePars(i);
      }
      if (prev == i - 2) {
        if (prev > -1) { // '(1 a) +|* #(2 b)'
          for (j = match[i - 2].begin(); j != match[i - 2].end(); j++) {
            for (k = match[i].begin(); k != match[i].end(); k++) {
              if  (*k > *j) {
                cardsets[i - 1].insert(*k - *j - 1);
              }
            }
          }
        }
        else { // '+|* #(1 a)'
          for (k = match[i].begin(); k != match[i].end(); k++) {
            cardsets[0].insert(*k);
          }
        }
      }
      else if (prev < i - 2) {//'(1 a) *|+ .. *|+ #(2 b)' or '*|+ .. *|+ #(1 a)'
        limit = (prev > -1) ? *(match[i].rbegin()) - *(match[prev].begin())
                            : *(match[i].rbegin()) + 1;
        for (int j = prev + 1; j < i; j++) {
          for (size_t m = 1; m < limit; m++) {
            cardsets[j].insert(m);
          }
        }
      }
      prev = i;
    }
    else if (i == f - 1) { // no matching at the end
      if (prev == -1) { // no matching at all
        for (int m = 0; m <= i; m++) {
          for (size_t n = 1; n <= numOfLabels; n++) {
            cardsets[m].insert(n);
          }
        }
      }
      else if (prev == i - 1) {
        for (j = match[i - 1].begin(); j != match[i - 1].end(); j++) {
          cardsets[i].insert(numOfLabels - 1 - *j);
        }
      }
      else { // '... (1 a) * ... #*' or '* ... #*'
        for (int j = prev + 1; j <= i; j++) {
          for (size_t m = 1; m < numOfLabels - *(match[prev].begin()); m++){
            cardsets[j].insert(m);
          }
        }
      }
    }
    if (p->elems[i].getW() != STAR) {
      numOfNonStars++;
    }
    if (p->elems[i].getW() != NO) {
      numOfW++;
    }
  }
  correctCardsets(numOfNonStars, numOfW);
}

/*
\subsection{Function ~correctCardsets~}

Inserts and deletes missing and incorrect (respectively) zero values in the
sets of cardinality candidates. Subsequently, all elements exceeding a certain
threshold (which depends on the number of non-asterisk units in the pattern)
are erased.

*/
void Match::correctCardsets(int nonStars, int wildcards) {
  set<unsigned int>::iterator j;
  for (int i = 0; i < f; i++) {
    if (wildcards > 1) { // correct zeros for more than one star
      if (p->elems[i].getW() == STAR) {
        cardsets[i].insert(0);
      }
      else {
        cardsets[i].erase(0);
      }
    }
    int k = (p->elems[i].getW() == STAR ? 1 : 2);
    if ((j = cardsets[i].lower_bound(numOfLabels - nonStars + k))
        != cardsets[i].end()) { // delete too high candidates
      cardsets[i].erase(j, cardsets[i].end());
    }
  }
}

/*
\subsection{Function ~buildSequences~}

Derives all possible ULabel sequences from the cardinality candidates. Only
sequences with length ~numOfLabels~ are accepted. This function is necessary
for ~rewrite~. For ~matches~, see ~getNextSeq~.

*/
void Match::buildSequences() {
  sequences.clear();
  multiset<unsigned int> seq;
  vector<unsigned int> cards;
  set<unsigned int>::iterator it;
  int maxNumber = 0;
  size_t totalSize = 1;
  unsigned int j, cardSum, partSum;
  for (int i = 0; i < f; i++) { // determine id of most cardinality candidates
    if (cardsets[i].size() > cardsets[maxNumber].size()) {
      maxNumber = i;
    }
  }
  for (int i = 0; i < f; i++) {
    if (i != maxNumber) {
      totalSize *= cardsets[i].size();
    }
  }
//   cout << "totalSize = " << totalSize << endl;
  for (size_t i = 0; i < totalSize; i++) {
    j = i;
    seq.clear();
    cards.clear();
    cardSum = 0;
    partSum = 0;
    for (int state = 0; state < f; state++) {
      if (state != maxNumber) {
        it = cardsets[state].begin();
        advance(it, j % cardsets[state].size());
        cards.push_back(*it);
        cardSum += *it;
        j /= cardsets[state].size();
        if (cardSum > numOfLabels) {
          state = f; // stop if sum exceeds maximum
        }
      }
    }
    if ((cardSum <= numOfLabels)
      && cardsets[maxNumber].count(numOfLabels - cardSum)) {
      cards.insert(cards.begin() + maxNumber, numOfLabels - cardSum);
      for (int k = 0; k < (int)cards.size(); k++) {
        seq.insert(partSum);
        partSum += cards[k];
      }
      if (doublePars.empty() || checkDoublePars(seq)) {
        sequences.insert(seq);
      }
    }
  }
}

/*
\subsection{Function ~checkDoublePars~}

Checks the correctness of a sequence concerning double parentheses. Therefore,
a comparison with the contents of the respective matchings set is performed.

*/
bool Match::checkDoublePars(multiset<unsigned int> sequence) {
  vector<unsigned int> seq(sequence.begin(), sequence.end());
  unsigned int max = -1;
  for (unsigned int i = 0; i < seq.size(); i++) {
    if (doublePars.count(i)) {
      if (i < seq.size() - 1) {
        max = seq[i + 1] - 1;
      }
      else {
        max = numOfLabels - 1;
      }  
      for (unsigned int j = seq[i]; j <= max; j++) {
        if (!match[i].count(j)) {
          return false;
        }
      }
    }
  }
  return true;
}

/*
\subsection{Function ~easyCondsMatch~}

*/
bool Match::easyCondsMatch(const MLabel &ml, UPat const &up) {
  if (up.getW()) {
    return true;
  }
  set<int> condPos = p->easyCondPos[up.getV()];
  if (condPos.empty()) {
    return true;
  }
  multiset<unsigned int> seq;
  seq.insert(ulId);
  seq.insert(ulId + 1);
  for (set<int>::iterator it = condPos.begin(); it != condPos.end(); it++) {
    if (!evaluateCond(ml, p->easyConds[*it], seq)) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~conditionsMatch~}

Checks whether the specified conditions are fulfilled. The result is true if
and only if there is (at least) one cardinality sequence that matches every
condition.

*/
bool Match::conditionsMatch(MLabel const &ml) {
  bool proceed(false);
  multiset<unsigned int>::iterator it;
  if (p->conds.empty()) {
    return true;
  }
  if (!ml.GetNoComponents()) { // empty MLabel
    return evaluateEmptyML();
  }
  maxCardPos = 0;
  for (int i = 0; i < f; i++) { // determine id of max. number of cardinalities
    if (cardsets[i].size() > cardsets[maxCardPos].size()) {
      maxCardPos = i;
    }
  }
  seqMax = 1;
  for (int i = 0; i < f; i++) {
    if (i != maxCardPos) {
      seqMax *= cardsets[i].size();
    }
  }
  seqCounter = 0;
  computeSeqOrder();
  multiset<unsigned int> seq = getNextSeq();
  for (int i = 0; i < (int)p->conds.size(); i++) {
    do {
      proceed = false;
      if (!evaluateCond(ml, p->conds[i], seq)) {
        seq = getNextSeq();
        i = 0; // in case of a mismatch, return to the first condition
      }
      else {
        proceed = true;
      }
    } while (!seq.empty() && !proceed);
    if (!proceed) { // no matching sequence found
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~computeSeqOrder~}

Computes the order in which the sequences will be built. More exactly, the
sequences will first differ in the positions having variables.

*/
void Match::computeSeqOrder() {
  set<int> used;
  int k = 0;
  for (int i = 0; i < (int)p->conds.size(); i++) {
    for (int j = 0; p->conds[i].getPId(j) != -1; j++) {
      if (!used.count(p->conds[i].getPId(j))) {
        seqOrder[k] = p->conds[i].getPId(j);
        k++;
        used.insert(p->conds[i].getPId(j));
      }
    }
  }
  for (int i = 0; i < f; i++) {
    if (!used.count(i)) {
      seqOrder[k] = i;
      k++;
    }
  }
}

/*
\subsection{Function ~getNextSeq~}

Invoked during ~matches~, similar to ~buildSequences~, but with two crucial
differences: (1) Not all the possible matching sequences are built but only one
is constructed and returned -- in contrast to the operator ~rewrite~, where all
the sequences are needed. (2) The order of the sequences depends on whether a
unit pattern is referred to in the conditions.

*/
multiset<unsigned int> Match::getNextSeq() {
  multiset<unsigned int> result;
  unsigned int cardSum, partSum, j;
  set<unsigned int>::iterator it;
  multiset<unsigned>::iterator m;
  while (seqCounter < seqMax) {
    j = seqCounter;
    cardSum = 0;
    partSum = 0;
    unsigned int cards[f];
    result.clear();
    for (int i = 0; i < f; i++) {
      if (seqOrder[i] != maxCardPos) {
        it = cardsets[seqOrder[i]].begin();
        advance(it, j % cardsets[seqOrder[i]].size());
        cards[seqOrder[i]] = *it;
        cardSum += *it;
        j /= cardsets[seqOrder[i]].size();
        if (cardSum > numOfLabels) {
          i = f; // stop if sum exceeds maximum
        }
      }
    }
    if ((cardSum <= numOfLabels)
      && cardsets[maxCardPos].count(numOfLabels - cardSum)) {
      cards[maxCardPos] = numOfLabels - cardSum;
      for (int k = 0; k < f; k++) {
        result.insert(partSum);
        partSum += cards[k];
      }
      if (doublePars.empty() || checkDoublePars(result)) {
        seqCounter++;
//         cout << "seq_" << seqCounter << " | ";
//         for (m = result.begin(); m != result.end(); m++) {
//           cout << *m << ", ";
//         }
//         cout << endl;
        return result;
      }
    }
    seqCounter++;
  }
//   cout << "no more sequences" << endl;
  return result;
}

/*
\subsection{Function ~evaluateEmptyML~}

This function is invoked in case of an empty moving label (i.e., with 0
components). A match is possible for a pattern like 'X [*] Y [*]' and conditions
X.card = 0, X.card = Y.card [*] 7. Time or label constraints are invalid.

*/
bool Match::evaluateEmptyML() {
  Word res;
  for (int i = 0; i < (int)p->conds.size(); i++) {
    for (int j = 0; j < p->conds[i].getKeysSize(); j++) {
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

This function is invoked by ~conditionsMatch~ and checks whether a sequence of
possible cardinalities matches a certain condition.

*/
bool Match::evaluateCond(const MLabel &ml, Condition &cond,
                         multiset<unsigned int> sq){
  vector<size_t> seq(sq.begin(), sq.end());
  Word qResult;
  ULabel ul;
  for (int i = 0; i < cond.getKeysSize(); i++) {
    int pId = cond.getPId(i);
    size_t max;
    if ((sq.size() < 3) && (pId > 0)) {
      pId = 0;
      max = seq[0] + 1;
    }
    else {
      max = (pId == f - 1 ? numOfLabels - 1 : seq[pId + 1] - 1);
    }
    if ((max > (size_t)ml.GetNoComponents()) || max < seq[pId]) {
      // cout << cond.getVar(i) << " bound to empty sequence" << endl;
      return false;
    }
    switch (cond.getKey(i)) {
      case 0: { // label
        ml.Get(seq[pId], ul);
        cond.setLabelPtr(i, ul.constValue.GetValue());
        break;
      }
      case 1: { // time
        cond.clearTimePtr(i);
        for (size_t j = seq[pId]; j <= max; j++) {
          ml.Get(j, ul);
          cond.mergeAddTimePtr(i, ul.timeInterval);
        }
        break;
      }
      case 2: { // start
        ml.Get(seq[pId], ul);
        cond.setStartEndPtr(i, ul.timeInterval.start);
        break;
      }
      case 3: { // end
        ml.Get(max, ul);
        cond.setStartEndPtr(i, ul.timeInterval.end);
        break;
      }
      case 4: { // leftclosed
        ml.Get(seq[pId], ul);
        cond.setLeftRightclosedPtr(i, ul.timeInterval.lc);
        break;
      }
      case 5: { // rightclosed
        ml.Get(max, ul);
        cond.setLeftRightclosedPtr(i, ul.timeInterval.rc);
        break;
      }
      case 6: { // card
        cond.setCardPtr(i, max + 1 - seq[pId]);
        break;
      }
      default: { // labels
        cond.cleanLabelsPtr(i);
        for (size_t j = seq[pId]; j <= max; j++) {
          ml.Get(j, ul);
          Label *label = new Label(true, ul.constValue.GetValue());
          cond.appendToLabelsPtr(i, *label);
        }
        cond.completeLabelsPtr(i);
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
  for (int j = 0; j < (int)vars.size(); j++) {
    result << "  [[" << j << "]] " << vars[j] << "." << keys[j] << " in #"
           << pIds[j] << endl;
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

void Condition::completeLabelsPtr(unsigned int pos) {
  if (pos < pointers.size()) {
    ((Labels*)pointers[pos])->Complete();
    ((Labels*)pointers[pos])->Sort();
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
\subsection{Function ~buildMultiNFA~}

*/
vector<map<int, set<int> > > Match::buildMultiNFA(vector<Pattern*> pats) {
  int start = 0;
  vector<map<int, set<int> > > delta;
  map<int, set<int> > emptyMapping;
  for (unsigned int p = 0; p < pats.size(); p++) {
    int f = start + pats[p]->getSize();
    int prev[3] = {start - 1, start - 1, start - 1}; // prevStar, prevNotStar,
    delta.push_back(emptyMapping);                   // secondPrevNotStar
    for (int i = start; i < f; i++) {
      delta.push_back(emptyMapping);
      delta[i][i - start].insert(i + 1);//state i,read pattern i =>new state i+1
      UPat u = pats[p]->getPat(i - start);//next line: any pattern except +,*
      if (!u.getW() || !u.getI().empty() || !u.getL().empty() || (i == f - 1)) {
        if ((prev[0] == i - 1) || (i == f - 1)) { //last pat or'...* #(1 a)...'
          for (int j = prev[1] + 1; j < i; j++) {
            delta[j][i - start].insert(i + 1); // '* * * #(1 a ) ...'
            for (int k = j; k <= i; k++) {
              delta[j][k - start].insert(j);
              for (int m = j; m <= i; m++) {
                delta[j][k - start].insert(m); // step 1
              }
              if ((u.getW() == STAR) && (i == f - 1)) { // end
                delta[j][k - start].insert(f);
              }
            }
          }
          if (prev[1] >= start) { // match before current pattern
            for (int j = prev[1] + 1; j <= i; j++) {
              delta[prev[1]][prev[1] - start].insert(j); // step 2
            }
            if ((u.getW() == STAR) && (i == f - 1)) { // end
              delta[prev[1]][prev[1] - start].insert(f);
            }
          }
          if (prev[2] < prev[1] - 1) { // '* ... * (1 a) * ... * #(2 b) ...'
            for (int j = prev[2] + 1; j < prev[1]; j++){
              for (int k = prev[1] + 1; k <= i; k++) {
                delta[j][prev[1] - start].insert(k); // step 3
              }
              if ((u.getW() == STAR) && (i == f - 1)) { // end
                delta[j][prev[1] - start].insert(f);
              }
            }
          }
        }
        prev[2] = prev[1];
        prev[1] = i;
      }
      else if (u.getW() == STAR) { // reading '*'
        prev[0] = i;
      }
      else if (u.getW() == PLUS) { // reading '+'
        delta[i][i - start].insert(i);
        prev[2] = prev[1];
        prev[1] = i;
      }
    }
    if (pats[p]->getPat(f - 1 - start).getW()) { // '... #*' or '... #+'
      delta[f - 1][f - 1 - start].insert(f - 1);
    }
    start += pats[p]->getSize() + 1;
  }
  return delta;
}

/*
\subsection{Function ~printMultiNFA~}

Prints a multiNFA.

*/
void Match::printMultiNFA() {
  set<int>::iterator k;
  for (unsigned int i = 0; i < p->delta.size(); i++) {
    for (unsigned int j = 0; j < p->delta.size(); j++) {
      if (p->delta[i][j].size() > 0) {
        cout << "state " << i << " | upat #" << j << " | new states {";
        if (p->delta[i][j].size() == 1) {
          cout << *(p->delta[i][j].begin());
        }
        else {
          for (k = p->delta[i][j].begin();
               k != p->delta[i][j].end(); k++) {
            cout << *k << " ";
          }
        }
        cout << "}" << endl;
      }
    }
  }
}

/*
\subsection{Function ~applyMultiNFA~}

Applies a multiNFA and returns the set of numbers referring to the matching
patterns.

*/
vector<int> Match::applyMultiNFA(ClassifyLI* c, bool rewrite /*=false*/) {
  currentStates = c->initialStates;
  set<int>::iterator i;
  vector<int> result;
  map<int, set<int> >::iterator j;
  int state(0), activePat(0);
  bool found;
  for (ulId = 0; (ulId < (size_t)c->currentML->GetNoComponents()
                     && !currentStates.empty()); ulId++) {
    c->currentML->Get(ulId, ul);
    set<int> newStates;
    for (i = currentStates.begin(); (i != currentStates.end() && *i < f); i++) {
      state = *i; // find corresponding pattern
      found = false;
      if ((*i < c->pat2start[1]) || (*i == 0)) {
        activePat = 0;
        found = true;
      }
      while (!found && state >= 0) {
        activePat = c->start2pat[state];
        if (!activePat) {
          state--;
        }
        else {
          found = true;
        }
      }
      p = c->pats[activePat];
      if (!initEasyCondOpTrees()) {
        cout << "easyCondOpTrees could not be initialized" << endl;
      }
      else {
        for (j = c->delta[*i].begin(); j != c->delta[*i].end(); j++) {
          if (j->second.size()) {
            if (labelsMatch(ul.constValue.GetValue(),
                            c->pats[activePat]->getPat(j->first).getL()) &&
             timesMatch(&ul.timeInterval,
                        c->pats[activePat]->getPat(j->first).getI()) &&
             easyCondsMatch(*(c->currentML),
                            c->pats[activePat]->getPat(j->first))) {
              newStates.insert(j->second.begin(), j->second.end());
              if (!c->pats[activePat]->getConds().empty() || rewrite) {
                UPat u = c->pats[activePat]->getPat(j->first); //store matches
                if (!u.getW() || !u.getI().empty() || !u.getL().empty()){//(_ a)
                  c->matches[activePat][j->first].insert(ulId);//((1 _)),(), sim
                }
              }
            }
          }
        }
//         deleteEasyCondOpTrees(); TODO: delete them!
      }
    }
    currentStates = newStates;
  } // translate active states to matched patterns
  if (currentStates.count(c->pat2start[1] - 1) || ((c->pats.size() == 1)
                          && currentStates.count(c->pats[0]->getSize()))) {
    result.push_back(0);
  }
  for (i = currentStates.begin(); i != currentStates.end(); i++) {
    if (c->end2pat[*i]) {
      result.push_back(c->end2pat[*i]);
    }
  }
  return result;
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
      result.second = new Labels(0);
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

For a pattern with conditions, an operator tree structure is prepared. If
~overwrite~ is true, the tree and the pointers are built in any case.

*/
bool Match::initCondOpTrees(bool overwrite) {
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  vector<Attribute*> ptrs;
  for (unsigned int i = 0; i < p->conds.size(); i++) { // opTrees for conditions
    if (overwrite) {
      p->conds[i].setTreeOk(false);
    }
    if (!p->conds[i].isTreeOk()) {
      q = "query " + p->conds[i].getText();
      for (int j = 0; j < p->conds[i].getKeysSize(); j++) { // init pointers
        strAttr = Pattern::getPointer(p->conds[i].getKey(j));
        ptrs.push_back(strAttr.second);
        toReplace = p->conds[i].getVar(j)
                    + Condition::getType(p->conds[i].getKey(j));
        q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
      }
      pair<QueryProcessor*, OpTree> qp_optree = processQueryStr(q, -1);
      if (!qp_optree.first) {
        cout << "Operator tree for condition " << i << " uninitialized" << endl;
        return false;
      }
      p->conds[i].setOpTree(qp_optree);
      p->conds[i].setPointers(ptrs);
      ptrs.clear();
      p->conds[i].setTreeOk(true);
    }
  }
  return true;
}

/*
\subsection{Function ~initEasyCondOpTrees~}

For a pattern with conditions, an operator tree structure is prepared.

*/
bool Match::initEasyCondOpTrees() {
  string q(""), part, toReplace("");
  pair<string, Attribute*> strAttr;
  vector<Attribute*> ptrs;
  for (unsigned int i = 0; i < p->easyConds.size(); i++) { //opTrees for conds
    if (!p->easyConds[i].isTreeOk()) {
      q = "query " + p->easyConds[i].getText();
      for (int j = 0; j < p->easyConds[i].getKeysSize(); j++) { // init pointers
        strAttr = Pattern::getPointer(p->easyConds[i].getKey(j));
        ptrs.push_back(strAttr.second);
        toReplace = p->easyConds[i].getVar(j)
                    + Condition::getType(p->easyConds[i].getKey(j));
        q.replace(q.find(toReplace), toReplace.length(), strAttr.first);
      }
      pair<QueryProcessor*, OpTree> qp_optree = processQueryStr(q, -1);
      if (!qp_optree.first) {
        cout << "Op tree for easy condition " << i << " uninitialized" << endl;
        return false;
      }
      p->easyConds[i].setOpTree(qp_optree);
      p->easyConds[i].setPointers(ptrs);
      ptrs.clear();
      p->easyConds[i].setTreeOk(true);
    }
  }
  return true;
}

/*
\subsection{Function ~deleteCondOpTrees~}

Removes the corresponding structures.

*/
void Match::deleteCondOpTrees() {
  for (unsigned int i = 0; i < p->conds.size(); i++) {
    p->conds[i].deleteOpTree();
  }
}

/*
\subsection{Function ~deleteEasyCondOpTrees~}

Removes the corresponding structures.

*/
void Match::deleteEasyCondOpTrees() {
  for (unsigned int i = 0; i < p->easyConds.size(); i++) {
    if (p->easyConds[i].isTreeOk()) {
      p->easyConds[i].deleteOpTree();
    }
  }
}

/*
\subsection{Function ~applyConditions~}

Applies conditions from a set of patterns (for the operator ~classify~).

*/
vector<int> Match::applyConditions(ClassifyLI* c) {
  vector<int> result;
  int numOfStates = 0;
  for (unsigned int i = 0; i < c->matched.size(); i++) {
    if (c->pats[c->matched[i]]->getConds().empty()) {
      result.push_back(c->matched[i]);
    }
    else {
      p = c->pats[c->matched[i]];
      numOfStates = f;
      f = p->elems.size();
      if (!initCondOpTrees(true)) {
        cout << "Operator trees could not be initialized" << endl;
        result.clear();
        return result;
      }
      for (int j = 0; j < f; j++) {
        match[j] = c->matches[c->matched[i]][j];
        cardsets[j].clear();
      }
      numOfLabels = c->currentML->GetNoComponents();
      computeCardsets();
      if (conditionsMatch(*c->currentML)) {
        result.push_back(c->matched[i]);
      }
      deleteCondOpTrees();
      f = numOfStates;
    }
  }
  return result;
}

/*
\subsection{Function ~multiRewrite~}

Computes a multiple rewrite result, i.e., a vector of MLabels.

*/
void Match::multiRewrite(ClassifyLI* c) {
  MLabel *ml = 0;
  set<pair<vector<unsigned int>, vector<unsigned int> > >::iterator it;
  for (unsigned int i = 0; i < c->matched.size(); i++) {
    rewriteSeqs.clear();
    p = c->pats[c->matched[i]];
    f = p->elems.size();
    for (int j = 0; j < f; j++) {
      if ((int)c->matches.size() > 0) {
        match[j] = c->matches[c->matched[i]][j];
      }
      else {
        match[j].clear();
      }
      cardsets[j].clear();
    }
    numOfLabels = c->currentML->GetNoComponents();
    assignedVars = c->pats[c->matched[i]]->getAssVars();
    varPos = c->pats[c->matched[i]]->getVarPos();
    computeResultVars(c->pats[c->matched[i]]->getAssigns());
    computeCardsets();
    buildSequences();
    if (p->conds.size()) {
      if (!initCondOpTrees(true)) {
        return;
      }
    }
    filterSequences(*(c->currentML));
    if (!c->pats[c->matched[i]]->initAssignOpTrees()) {
      return;
    }
    while (!rewriteSeqs.empty()) {
      it = rewriteSeqs.begin();
      ml = new MLabel(0);
      ml->rewrite(*(c->currentML), *it, c->pats[c->matched[i]]->getAssigns(),
                  c->pats[c->matched[i]]->getVarPosInSeq());
      rewriteSeqs.erase(it);
      c->rewritten.push_back(ml);
      ml = 0;
    }
    deleteCondOpTrees();
    c->pats[c->matched[i]]->deleteAssignOpTrees();
  }
}

/*
\section{Operator ~the\_unit~}

the\_unit: label instant instant bool bool --> ulabel

\subsection{Type Mapping}

*/
ListExpr the_unit_Label_TM(ListExpr args) {
  if (nl->Equal(args, nl->FiveElemList(nl->SymbolAtom(Label::BasicType()),
                                       nl->SymbolAtom(Instant::BasicType()),
                                       nl->SymbolAtom(Instant::BasicType()),
                                       nl->SymbolAtom(CcBool::BasicType()),
                                       nl->SymbolAtom(CcBool::BasicType())))) {
    return nl->SymbolAtom(ULabel::BasicType());
  }
  return listutils::typeError(
    "Operator 'the_unit' expects a list with structure\n"
     "'(point point instant instant bool bool)', or \n"
     "'(ipoint ipoint bool bool)', or \n"
     "'(real real real bool instant instant bool bool)', or\n"
     "'(T instant instant bool bool)', or \n"
     "'(iT duration bool bool)'\n for T in {bool, int, string, label}.");
}

/*
\subsection{Value Mapping}

*/
int the_unit_Label_VM(Word* args, Word& result,
                        int message, Word& local, Supplier s) {
  result = (qp->ResultStorage(s));
  ULabel  *res = static_cast<ULabel*>(result.addr);
  Label *value = static_cast<Label*>(args[0].addr);
  Instant *i1  = static_cast<DateTime*>(args[1].addr);
  Instant *i2  = static_cast<DateTime*>(args[2].addr);
  CcBool  *cl  = static_cast<CcBool*>(args[3].addr);
  CcBool  *cr  = static_cast<CcBool*>(args[4].addr);
  bool clb, crb;
  if (!value->IsDefined() || !i1->IsDefined() || !i2->IsDefined() ||
      !cl->IsDefined() || !cr->IsDefined()) {
    res->SetDefined( false );
    return 0;
  }
  clb = cl->GetBoolval();
  crb = cr->GetBoolval();
  if (((*i1 == *i2) && (!clb || !crb)) || (i1->Adjacent(i2) && !(clb || crb))) {
    res->SetDefined(false); // illegal interval setting
    return 0;
  }
  if (*i1 < *i2) { // sorted instants
    Interval<Instant> interval(*i1, *i2, clb, crb);
    *res = ULabel(interval, *value);
  }
  else {
    Interval<Instant> interval(*i2, *i1, clb, crb);
    *res = ULabel(interval, *value);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct the_unit_LabelInfo : OperatorInfo {
  the_unit_LabelInfo() {
    name      = "the_unit";
    signature = "label x instant x instant x bool x bool -> ulabel";
    syntax    = "the_unit( _ _ _ _ _ )";
    meaning   = "Creates a ulabel from its components.";
  }
};

/*
\section{Operator ~makemvalue~}

makemvalue: tream (tuple ((x1 t1)...(xi ulabel)...(xn tn))) xi -> mlabel

\subsection{Type Mapping}

*/
ListExpr makemvalue_ULabelTM(ListExpr args) {
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
    ErrorReporter::ReportError("Operator makemvalue expects as first argument "
      "a tuplestream, but gets '" + argstr + "'.");
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
  if (inputtype=="") {
    return listutils::typeError("attribute not found");
  }
  if (inputtype != ULabel::BasicType()) {
    return listutils::typeError("attr type not in {ubool, uint,"
                                " ustring, ulabel, ureal, upoint");
  }
  attrname = nl->SymbolValue(second);
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  assert(j !=0);
  if (inputtype == ULabel::BasicType()) {
    attrtype = nl->SymbolAtom(MLabel::BasicType());
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
         nl->TwoElemList(nl->IntAtom(j),
         nl->StringAtom(nl->SymbolValue(attrtype))), attrtype);
}

/*
\subsection{Value Mapping}

*/
int makemvalue_ULabelVM(Word* args,Word& result,int message,
                         Word& local,Supplier s) {
  MLabel* m;
  ULabel* unit;
  Word curTupleWord;
  assert(args[2].addr != 0);
  assert(args[3].addr != 0);
  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, curTupleWord);
  result = qp->ResultStorage(s);
  m = (MLabel*)result.addr;
  m->Clear();
  m->SetDefined(true);
  m->StartBulkLoad();
  while (qp->Received(args[0].addr)) { // get all tuples
    Tuple* curTuple = (Tuple*)curTupleWord.addr;
    Attribute* curAttr = (Attribute*)curTuple->GetAttribute(attrIndex);
    if (curAttr == 0) {
      cout << endl << "ERROR in " << __PRETTY_FUNCTION__
           << ": received Nullpointer!" << endl;
      assert( false );
    }
    else if (curAttr->IsDefined()) {
      unit = static_cast<ULabel*>(curAttr);
      m->Add(*unit);
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
\subsection{Operator Info}

*/
struct makemvalue_ULabelInfo : OperatorInfo {
  makemvalue_ULabelInfo() {
    name      = "makemvalue";
    signature = "stream (tuple ((x1 t1)...(xi ulabel)...(xn tn))) xi -> mlabel";
    syntax    = "_ makemvalue[ _ ]";
    meaning   = "Create a moving object from a (not necessarily sorted) "
                "tuple stream containing a ulabel attribute. No two unit "
                "timeintervals may overlap. Undefined units are allowed and "
                "will be ignored. A stream without defined units will result "
                "in an \'empty\' moving object, not in an \'undef\'.";
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
    if (!p->verifyPattern()) {
      delete pattern;
      cout << "pattern not verified" << endl;
      return 0;
    }
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
  Match *match = new Match(1);
  vector<map<int, set<int> > > multiNFA = match->buildMultiNFA(patterns);
  c->setPersistentNFA(&multiNFA);
  for (unsigned int i = 0; i < patterns.size(); i++) {
    delete patterns[i];
  }
  delete match;
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
  bool ok = true;
  Pattern* p = 0;
  vector<string> texts;
  vector<Pattern*> patterns;
  while (tuple && ok) {
    desc = (FText*)tuple->GetAttribute(0);
    if (!desc->IsDefined()) {
      cout << "Undefined description" << endl;
      ok = false;
    }
    else {
      ptext = (FText*)tuple->GetAttribute(1);
      if (!ptext->IsDefined()) {
        cout << "Undefined pattern text" << endl;
        ok = false;
      }
      else {
        p = Pattern::getPattern(ptext->GetValue(), true); // do not build NFA
        if (!p) {
          cout << "invalid pattern" << endl;
          ok = false;
        }
        else {
          if (!p->verifyPattern()) {
            ok = false;
          }
          else { // store information iff no problem was found
            texts.push_back(desc->GetValue());
            texts.push_back(ptext->GetValue());
            patterns.push_back(p);
          }
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
  Match *match = new Match(1);
  vector<map<int, set<int> > > delta = match->buildMultiNFA(patterns);
  for (unsigned int i = 0; i < patterns.size(); i++) {
    delete patterns[i];
  }
  c->setPersistentNFA(&delta);
  delete match;
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

*/
ListExpr matchesTM(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
    return NList::typeError("Two arguments expected");
  }
  NList type(args);
  if ((type == NList(MLabel::BasicType(), Pattern::BasicType()))
   || (type == NList(MLabel::BasicType(), FText::BasicType()))) {
    return NList(CcBool::BasicType()).listExpr();
  }
  return NList::typeError("Expecting a mlabel and a text/pattern");
}

/*
\subsection{Selection Function}

*/
int matchesSelect(ListExpr args) {
  NList type(args);
  return (type.second().isSymbol(Pattern::BasicType())) ? 1 : 0;
}

/*
\subsection{Value Mapping (for a Pattern)}

*/
int matchesVM_P(Word* args, Word& result, int message, Word& local, Supplier s){
  MLabel* ml = static_cast<MLabel*>(args[0].addr);
  Pattern* p = static_cast<Pattern*>(args[1].addr);
  if (!p) {
    cout << "Invalid Pattern." << endl;
    delete ml;
    return 0;
  }
  result = qp->ResultStorage(s);
  CcBool* b = static_cast<CcBool*>(result.addr);
  ExtBool match = p->matches(*ml);
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
  FText* patternText = static_cast<FText*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* b = static_cast<CcBool*>(result.addr);
  Pattern *pattern = 0;
  if (patternText->IsDefined()) {
    pattern = Pattern::getPattern(patternText->toText());
  }
  else {
    cout << "Undefined pattern text." << endl;
    b->SetDefined(false);
    return 0;
  }
  if (!pattern) {
    b->SetDefined(false);
  }
  else {
    ExtBool res = pattern->matches(*ml);
    delete pattern;
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
  return 0;
}

/*
\subsection{Operator Info}

*/
struct matchesInfo : OperatorInfo {
  matchesInfo() {
    name      = "matches";
    signature = MLabel::BasicType() + " x " + Pattern::BasicType() + " -> "
                                    + CcBool::BasicType();
    appendSignature(MLabel::BasicType() + " x text -> " + CcBool::BasicType());
    syntax    = "_ matches _";
    meaning   = "Match predicate.";
  }
};

/*
\section{Operator ~indexmatches~}

\subsection{Type Mapping}

*/
ListExpr indexmatchesTM(ListExpr args) {
  const string errMsg = "Expecting a relation, the name of a {mlabel, mstring}"
             " attribute of that relation, an invfile, and a text";
  if (nl->HasLength(args, 4)) {
    if (FText::checkType(nl->Fourth(args))) {
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
          if (!MLabel::checkType(attrType) && !MString::checkType(attrType)) {
            return listutils::typeError
                   ("type " + nl->ToString(attrType) + " is an invalid type");
          }
          if (InvertedFile::checkType(nl->Third(args))) {
            return nl->ThreeElemList(
              nl->SymbolAtom(Symbol::APPEND()),
              nl->OneElemList(nl->IntAtom(i)),
              nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), tupleList));
          }
        }
      }
    }
  }
  return listutils::typeError(errMsg);
}

IndexLI::IndexLI(Word _mlrel, Word _inv, Word _attrNr, Pattern* _p) {
  mlRel = (Relation*)_mlrel.addr;
  invFile = static_cast<InvertedFile*>(_inv.addr);
  attrNr = ((CcInt*)_attrNr.addr)->GetIntval() - 1;
  p = _p;
  classifyTT = 0;
  matches = new vector<set<unsigned int> >[mlRel->GetNoTuples()];
  for (int i = 0; i < mlRel->GetNoTuples(); i++) {
    matches[i].resize(p->getSize());
  }
  vector<TupleId> matchingMLs = applyPattern();
  applyConditions(matchingMLs, false);
  delete[] matches;
}

/*
\subsection{Value Mapping}

*/
int indexmatchesVM(Word* args, Word& result, int message, Word& local,
                   Supplier s){
  IndexLI *li = (IndexLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      FText *pText = static_cast<FText*>(args[3].addr);
      Pattern* p = 0;
      if (pText->IsDefined()) {
        p = Pattern::getPattern(pText->GetValue(), true);
      }
      if (p) {
        local.addr = new IndexLI(args[0], args[2], args[4], p);
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextResultTuple(false) : 0;
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
    meaning   = "Filters a relation containing a mlabel attribute, passing "
                "only those trajectories matching the pattern on "
                "to the output stream.";
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
  if (!MLabel::checkType(type) && !MString::checkType(type)) {
    return listutils::typeError("wrong type " + nl->ToString(type)
                                + " of attritube " + name);
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->IntAtom(index - 1)), stream);
}

/*
\subsection{Class ~FiltermatchesLI~}

*/
class FiltermatchesLI {
 public:
  FiltermatchesLI(Word _stream, int _attrIndex, FText* text):
      stream(_stream), attrIndex(_attrIndex) {
    Pattern *pattern = Pattern::getPattern(text->GetValue());
    if (pattern) {
      pattern->setVerified(false);
      if (pattern->verifyPattern()) {
        match = new Match(pattern->getPats().size() + 1);
        pattern->setVerified(true);
        p = *pattern;
        stream.open();
        streamOpened = true;
      }
    }
    else {
      match = 0;
      streamOpened = false;
    }
  }

  FiltermatchesLI(Word _stream, int _attrIndex, Pattern* pattern):
      stream(_stream), attrIndex(_attrIndex), match(0) {
    if (pattern) {
      match = new Match(pattern->getPats().size() + 1);
//       p->buildNFA();
      p = *pattern;
      match->copyFromPattern(&p);
      stream.open();
      streamOpened = true;
    }
    else {
      match = 0;
      streamOpened = false;
    }
  }

  ~FiltermatchesLI() {
    if (match) {
      delete match;
      match = 0;
    }
    if (streamOpened) {
      stream.close();
    }
  }

  Tuple* next() {
    if (!match) {
      return 0;
    }
    Tuple* cand = stream.request();
    MLabel* ml = 0;
    if (cand) {
      while (cand) {
        ml = (MLabel*)cand->GetAttribute(attrIndex);
        match->setPattern(&p);
        bool matching = match->matches(*ml);
        match->resetStates();
        if (matching) {
          return cand;
        }
        cand->DeleteIfAllowed();
        cand = stream.request();
      }
      
    }
    return 0;
  }

 private:
  Stream<Tuple> stream;
  int attrIndex;
  Match* match;
  bool streamOpened;
  Pattern p;
};

/*
\subsection{Selection Function}

*/
int filtermatchesSelect(ListExpr args) {
  return (FText::checkType(nl->Third(args)) ? 0 : 1);
}


/*
\subsection{Value Mapping for a Text}

*/
int filtermatchesVM_Text(Word* args, Word& result, int message,
                         Word& local, Supplier s) {
  FiltermatchesLI* li = (FiltermatchesLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      int index = ((CcInt*)args[3].addr)->GetValue();
      FText* text = (FText*)args[2].addr;
      if (text->IsDefined()) {
        local.addr = new FiltermatchesLI(args[0], index, text);
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->next() : 0;
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
int filtermatchesVM_Pat(Word* args, Word& result, int message,
                        Word& local, Supplier s) {
  FiltermatchesLI* li = (FiltermatchesLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      int index = ((CcInt*)args[3].addr)->GetValue();
      Pattern* p = (Pattern*)args[2].addr;
      if (p->isVerified()) {
        local.addr = new FiltermatchesLI(args[0], index, p);
      }
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->next() : 0;
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
  if ((patternPos == -1) && (text[0].empty() || (text[1].empty() &&
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
}

void Assign::deleteOpTrees() {
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

void Condition::deleteOpTree() {
  if (opTree.first) {
    opTree.first->Destroy(opTree.second, true);
    delete opTree.first;
    for (unsigned int i = 0; i < pointers.size(); i++) {
      deleteIfAllowed(pointers[i]);
    }
  }
}

bool RewriteResult::initAssignOpTrees() {
  for (unsigned int i = 0; i < assigns.size(); i++) {
    if (!assigns[i].initOpTrees()) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Value Mapping (for a text)}

*/
int rewriteVM_T(Word* args, Word& result, int message, Word& local, Supplier s){
  MLabel *source(0), *dest(0);
  FText* pText = 0;
  Pattern *p = 0;
  RewriteResult *rr = 0;
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
        if (!p->isVerified()) {
          if (!p->verifyPattern()) {
            cout << "Error: Invalid pattern." << endl;
          }
          else {
            p->setVerified(true);
          }
        }
        if (!p->hasAssigns()) {
          cout << "No result specified." << endl;
        }
        else {
          set<pair<vector<unsigned int>, vector<unsigned int> > > rewriteSeqs =
                                                     p->getRewriteSeqs(*source);
          map<string, int> varPosInSeq = p->getVarPosInSeq();
          rr = new RewriteResult(rewriteSeqs, source, p->getAssigns(),
                                 varPosInSeq);
        }
      }
      delete p;
      local.addr = rr;
      return 0;
    }
    case REQUEST: {
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      rr = ((RewriteResult*)local.addr);
      if (rr->finished()) {
        result.addr = 0;
        return CANCEL;
      }
      if (!rr->initAssignOpTrees()) {
        return CANCEL;
      }
      dest = new MLabel(1);
      do {
        dest->rewrite(rr->getML(), rr->getCurrentSeq(), rr->getAssignments(),
                      rr->getVarPosInSeq());
        rr->next();
      } while (!dest->IsDefined() && !rr->finished());
      if (dest->IsDefined()) {
        result.addr = dest;
        return YIELD;
      }
      else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (local.addr) {
        rr = ((RewriteResult*)local.addr);
        delete rr;
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
  MLabel *source(0), *dest(0);
  Pattern *p = 0;
  RewriteResult *rr = 0;
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
        p->setVerified(true);
        MLabel* mlNew = new MLabel(source);
        set<pair<vector<unsigned int>, vector<unsigned int> > > rewriteSeqs =
                                                    p->getRewriteSeqs(*mlNew);
        map<string, int> varPosInSeq = p->getVarPosInSeq();
        rr = new RewriteResult(rewriteSeqs, mlNew, p->getAssigns(),
                               varPosInSeq);
      }
      local.addr = rr;
      return 0;
    }
    case REQUEST: {
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      rr = ((RewriteResult*)local.addr);
      if (rr->finished()) {
        result.addr = 0;
        return CANCEL;
      }
      if (!rr->initAssignOpTrees()) {
        return CANCEL;
      }
      dest = new MLabel(1);
      do {
        dest->rewrite(rr->getML(), rr->getCurrentSeq(), rr->getAssignments(),
                       rr->getVarPosInSeq());
        rr->next();
      } while (!dest->IsDefined() && !rr->finished());
      if (dest->IsDefined()) {
        result.addr = dest;
        return YIELD;
      }
      else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      if (local.addr) {
        rr = ((RewriteResult*)local.addr);
        delete rr;
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
  ClassifyLI *li = (ClassifyLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      bool dummy = false;
      local.addr = new ClassifyLI(args[0], args[1], dummy);
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

This constructor is used for the operator ~classify~ (trajectory type mlabel).

*/
ClassifyLI::ClassifyLI(MLabel *ml, Word _classifier) : mlStream(0),
                  classifyTT(0), mainMatch(0), streamOpen(false), newML(false) {
  currentML = ml;
  if (currentML) {
    if (currentML->IsDefined()) {
      Classifier *c = static_cast<Classifier*>(_classifier.addr);
      int startPos = 0;
      set<unsigned int> emptyset;
      Pattern *p = 0;
      for (int i = 0; i < (c->getCharPosSize() / 2); i++) {
        p = Pattern::getPattern(c->getPatText(i), true);
        if (p) {
          p->setDescr(c->getDesc(i));
          start2pat[startPos] = pats.size();
          pat2start[pats.size()] = startPos;
          initialStates.insert(startPos);
          startPos += p->getSize() + 1;
          end2pat[startPos - 1] = pats.size();
          if (!p->getConds().empty()) {
            for (int j = 0; j < p->getSize(); j++) {
              matches[pats.size()].push_back(emptyset);
            }
          }
          pats.push_back(p);
        }
        else {
          cout << "pattern could not be parsed" << endl;
        }
      }
      if (!pats.size()) {
        cout << "no classification data specified" << endl;
      }
      else {
        numOfStates = startPos;
        mainMatch = new Match(numOfStates);
        delta = createNFAfromPersistent(*(c->getDelta()));
        matched = mainMatch->applyMultiNFA(this);
        matched = mainMatch->applyConditions(this);
      }
    }
  }
}

/*
\subsection{Constructor for class ~ClassifyLI~}

This constructor is used for the operator ~rewrite~.

*/
ClassifyLI::ClassifyLI(Word _mlstream, Word _pstream, bool rewrite) :
mlStream(_mlstream), currentML(0), mainMatch(0),streamOpen(false),newML(false){
  classifyTT = 0;
  Stream<FText> pStream(_pstream);
  pStream.open();
  FText* inputText = pStream.request();
  int startPos = 0;
  set<unsigned int> emptyset;
  while (inputText) {
    if (!inputText->IsDefined()) {
      cout << "undefined input" << endl;
    }
    else {
      Pattern *p = Pattern::getPattern(inputText->GetValue(), true);
      if (p) {
        if (!p->hasAssigns()) {
          cout << "pattern without rewrite part" << endl;
        }
        else {
          start2pat[startPos] = pats.size();
          pat2start[pats.size()] = startPos;
          initialStates.insert(startPos);
          startPos += p->getSize() + 1;
          end2pat[startPos - 1] = pats.size();
          for (int i = 0; i < p->getSize(); i++) {
            matches[pats.size()].push_back(emptyset);
          }
          pats.push_back(p);
        }
      }
      else {
        cout << "pattern \'" << inputText->GetValue() << "\' not parsed" <<endl;
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
    numOfStates = startPos;
    mainMatch = new Match(numOfStates);
    delta = mainMatch->buildMultiNFA(pats);
    //mainMatch->printMultiNFA();
  }
}

/*
\subsection{Destructor for class ~ClassifyLI~}

*/
ClassifyLI::~ClassifyLI() {
  if (streamOpen) {
    mlStream.close();
  }
  if (classifyTT) {
    delete classifyTT;
  }
  if (newML) {
    currentML->DeleteIfAllowed();
  }
  currentML = 0;
  if (mainMatch) {
    delete mainMatch;
  }
  map<int, set<size_t>* >::iterator i;
  vector<Pattern*>::iterator ip;
  for (ip = pats.begin(); ip != pats.end(); ip++) {
    delete (*ip);
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
\subsection{Function ~printMatches~}

*/
void ClassifyLI::printMatches() {
  set<unsigned int>::iterator it;
  for (unsigned int i = 0; i < pats.size(); i++) {
    cout << "--===-- Pattern " << i << " --===--" << endl;
    for (unsigned int j = 0; j < matches[i].size(); j++) {
      cout << "upat " << j << " matches ulabels ";
      for (it = matches[i][j].begin(); it != matches[i][j].end(); it++) {
        cout << *it << ", ";
      }
      cout << endl;
    }
  }
}

/*
\subsection{Function ~nextResultText~}

This function is used for the operator ~classify~.

*/
FText* ClassifyLI::nextResultText() {
  if (!pats.size()) {
    return 0;
  }
  if (!matched.empty()) {
    FText* result = new FText(true, pats[matched.back()]->getDescr());
    matched.pop_back();
    return result;
  }
  return 0;
}

/*
\subsection{Function ~nextResultML~}

This function is used for the operator ~rewrite~.

*/
MLabel* ClassifyLI::nextResultML() {
  if (!pats.size()) {
    return 0;
  }
  MLabel *result = 0;
  while (rewritten.empty()) {
    if (currentML) {
      currentML->DeleteIfAllowed();
    }
    currentML = (MLabel*)mlStream.request();
    if (!currentML) { // stream finished
      return 0;
    }
    if (currentML->IsDefined()) {
      matched = mainMatch->applyMultiNFA(this, true);
      if (!matched.empty()) {
        mainMatch->multiRewrite(this);
        mainMatch->setFinalState(numOfStates - 1);
      }
    }
  }
//   result = (MLabel*)rewritten.back()->Copy();
  result = rewritten.back();
  rewritten.pop_back();
  return result;
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
      local.addr = new ClassifyLI(source, args[1]);
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
  const string errMsg = "Expecting a relation, the name of an {mlabel, mstring}"
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
          if (!MLabel::checkType(attrType) && !MString::checkType(attrType)) {
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
              nl->OneElemList(nl->IntAtom(i)),
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
\subsection{Constructor for class ~IndexLI~}

This constructor is used for the operator ~indexclassify~.

*/
IndexLI::IndexLI(Word _mlrel, Word _inv, Word _classifier, Word _attrNr) :
                 p(0), pCounter(0) {
  mlRel = (Relation*)_mlrel.addr;
  invFile = static_cast<InvertedFile*>(_inv.addr);
  attrNr = ((CcInt*)_attrNr.addr)->GetIntval() - 1;
  c = (Classifier*)_classifier.addr;
  classifyTT = ClassifyLI::getTupleType();
}

/*
\subsection{Function ~timesMatch~}

*/
bool IndexLI::timesMatch(TupleId tId, unsigned int ulId, set<string> ivs) {
  if (ivs.empty()) {
    return true;
  }
  ULabel ul = getUL(tId, ulId);
  return ::timesMatch(&ul.timeInterval, ivs);
}

/*
\subsection{Function ~initIterator~}

*/
set<unsigned int>::iterator IndexLI::initIterator(int tId, int pPos) {
  set<unsigned int>::iterator it = matches[tId - 1][pPos].end();
  if (it != matches[tId - 1][pPos].begin()) {
    it--;
  }
  return it;
}

/*
\subsection{Function ~applyUnitPattern~}

*/
void IndexLI::applyUnitPattern(int pPos, vector<int>& prev, Wildcard& wild,
                               vector<bool>& active, int &lastId) {
  InvertedFile::exactIterator* eit = 0;
  TupleId id;
  wordPosType wc;
  charPosType cc;
  ULabel ul(1);
  UPat up = p->getPat(pPos);
  set<string> labels = up.getL();
  set<string>::iterator j = labels.begin();
  set<unsigned int>::iterator i1, i2;
  if (up.getW() && up.getL().empty() && up.getI().empty()) { // *, +, or (())
    wild = up.getW();
    return;
  } // (), (...), or ((...))
  while (j != labels.end()) {
    eit = invFile->getExactIterator(*j, 4096);
    while (eit->next(id, wc, cc)) {
      if (active[id - 1]) {
        if (timesMatch(id, wc, up.getI())) {
          if (!pPos) {
            if (!wc) { // first unit pattern matches first unit label
              matches[id - 1][0].insert(0);
              prev[id - 1] = 0;
            }
          }
          else if (((wild == NO) && matches[id - 1][prev[id - 1]].count(wc-1)
                                 && (lastId != (int)id))
           || ((wild == PLUS) && ((prev[id - 1] == -1)
                        || *(matches[id - 1][prev[id - 1]].begin()) < wc))
           || ((wild == STAR) && ((prev[id - 1] == -1)
                        || *(matches[id - 1][prev[id - 1]].begin()) <= wc))) {
            matches[id - 1][pPos].insert(wc);
            lastId = id;
            prev[id - 1] = pPos;
          }
        }
      }
    }
    if (eit) {
      delete eit;
    }
    j++;
  }
  if (labels.empty()) {
    if (!pPos) { // first unit pattern
        for (int i = 1; i <= mlRel->GetNoTuples(); i++) {
          if (timesMatch(i, 0, up.getI())) {
            matches[i - 1][0].insert(0);
            prev[i - 1] = 0;
          }
        }
    }
    else { // not the first unit pattern
      if (wild == NO) {
        for (int i = 1; i <= mlRel->GetNoTuples(); i++) {
          if (active[i - 1]) {
            i2 = initIterator(i, pPos);
            for (i1 = matches[i - 1][prev[i - 1]].begin();
                 ((i1 != matches[i - 1][prev[i - 1]].end())
                   && (*i1 + 1 <= (unsigned int)getMLsize(i))); i1++) {
              if (timesMatch(i, *i1, up.getI())) {
                matches[i - 1][pPos].insert(i2, *i1 + 1);
                i2++;
              }
            }
          }
          if (matches[i - 1][pPos].size()) {
            prev[i - 1] = pPos;
          }
        }
      }
      else { // wildcard before current unit pattern
        int add = (wild == STAR ? 1 : 2);
        for (int i = 1; i <= mlRel->GetNoTuples(); i++) {
          if (active[i - 1]) {
            if (prev[i - 1] > -1) {
              i2 = initIterator(i, pPos);
              for (int k = *(matches[i - 1][prev[i - 1]].begin());
                   (k + add <= getMLsize(i)); k++) {
                if (timesMatch(i, k, up.getI())) {
                  matches[i - 1][pPos].insert(i2, k + add);
                  i2++;
                }
              }
            }
            else { // no match before current unit pattern
              i2 = initIterator(i, pPos);
              for (unsigned int k = (wild == STAR ? 0 : 1);
                   k < (unsigned int)getMLsize(i); k++) {
                if (timesMatch(i, k, up.getI())) {
                  matches[i - 1][pPos].insert(i2, k);
                  i2++;
                }
              }
            }
          }
          if (matches[i - 1][pPos].size()) {
            prev[i - 1] = pPos;
          }
        }
      }
    }
  }
  wild = NO;
  for (int i = 1; i <= mlRel->GetNoTuples(); i++) { // deactivate non-matching
    if (active[i - 1]) {                            // trajectories
      if (matches[i - 1][pPos].empty()) {
        active[i - 1] = false;
      }
    }
  }
}

/*
\subsection{Function ~applyPattern~}

*/
vector<TupleId> IndexLI::applyPattern() {
  vector<TupleId> result;
  Wildcard wild = NO;
  vector<int> prev(mlRel->GetNoTuples(), -1);
  vector<bool> active(mlRel->GetNoTuples(), true);
  int lastId = -1;
  for (int i = 0; i < p->getSize(); i++) { // iterate over pattern elements
    applyUnitPattern(i, prev, wild, active, lastId);
  }
  for (int j = 0; j < mlRel->GetNoTuples(); j++) {
    if (active[j]) {
      if (wild || matches[j][p->getSize() - 1].count(getMLsize(j + 1) - 1)) {
        result.push_back(j + 1);
      }
    }
  }
  return result;
}

/*
\subsection{Function ~applyConditions~}

*/
void IndexLI::applyConditions(vector<TupleId> tupleIds, bool classify) {
  if (p->getConds().empty()) { // no condition
    if (classify) {
      for (unsigned int i = 0; i < tupleIds.size(); i++){
        classification.push(make_pair(p->getDescr(), tupleIds[i]));
      }
    }
    else {
      for (unsigned int i = 0; i < tupleIds.size(); i++){
        resultIds.push(tupleIds[i]);
      }
    }
  }
  else {
    if (classify) {
      for (unsigned int i = 0; i < tupleIds.size(); i++){
        Match* m = new Match(this, tupleIds[i]);
        m->computeCardsets();
        m->initCondOpTrees(true);
        Tuple* tuple = mlRel->GetTuple(tupleIds[i], false);
        if (m->conditionsMatch((MLabel*)tuple->GetAttribute(attrNr))) {
          classification.push(make_pair(p->getDescr(), tupleIds[i]));
        }
        deleteIfAllowed(tuple);
        m->deleteCondOpTrees();
        delete m;
      }
    }
    else {
      for (unsigned int i = 0; i < tupleIds.size(); i++){
        Match* m = new Match(this, tupleIds[i]);
        m->computeCardsets();
        m->initCondOpTrees(true);
        Tuple* tuple = mlRel->GetTuple(tupleIds[i], false);
        if (m->conditionsMatch((MLabel*)tuple->GetAttribute(attrNr))) {
          resultIds.push(tupleIds[i]);
        }
        deleteIfAllowed(tuple);
        m->deleteCondOpTrees();
        delete m;
      }
    }
  }
} 

/*
\subsection{Function ~getMLsize~}

*/
int IndexLI::getMLsize(TupleId tId) {
  Tuple* tuple = mlRel->GetTuple(tId, false);
  int result = ((MLabel*)tuple->GetAttribute(attrNr))->GetNoComponents();
  deleteIfAllowed(tuple);
  return result;
}

/*
\subsection{Function ~getUL~}

*/
ULabel IndexLI::getUL(TupleId tId, unsigned int ulId) {
  ULabel result(1);
  Tuple* tuple = mlRel->GetTuple(tId, false);
  ((MLabel*)tuple->GetAttribute(attrNr))->Get(ulId, result);
  deleteIfAllowed(tuple);
  return result;
}

/*
\subsection{Function ~nextResultTuple~}

This function is used for the operators ~indexclassify~ and ~indexmatches~.

*/
Tuple* IndexLI::nextResultTuple(bool classify) {
  if (!mlRel->GetNoTuples()) { // no mlabel => no result
    return 0;
  }
  if (!classify) {
    if (resultIds.empty()) { // no mlabel => no result
      return 0;
    }
    Tuple* result = mlRel->GetTuple(resultIds.front(), false);
    resultIds.pop();
    return result;
  }
  pair<string, TupleId> onePair;
  while (classification.empty() && pCounter < c->getCharPosSize() / 2) {
    if (p) {
      delete p;
      p = 0;
    }
    p = Pattern::getPattern(c->getPatText(pCounter), true);
    if (p) {
      p->setDescr(c->getDesc(pCounter));
      matches = new vector<set<unsigned int> >[mlRel->GetNoTuples()];
      for (int i = 0; i < mlRel->GetNoTuples(); i++) {
        matches[i].resize(p->getSize());
      }
      vector<TupleId> matchingMLs = applyPattern();
      applyConditions(matchingMLs, true);
      delete[] matches;
    }
    pCounter++;
  }
  MLabel* ml = 0;
  if (classification.empty()) {
    return 0;
  }
  onePair = classification.front();
  Tuple *result = new Tuple(classifyTT);
  result->PutAttribute(0, new FText(true, onePair.first));
  Tuple* tuple = mlRel->GetTuple(onePair.second, false);
  ml = (MLabel*)tuple->GetAttribute(attrNr)->Copy();
  result->PutAttribute(1, ml);
  classification.pop();
  tuple->DeleteIfAllowed();
  return result;
}

/*
\subsection{Destructor for class ~IndexLI~}

*/
IndexLI::~IndexLI() {
  if (classifyTT) {
    delete classifyTT;
    classifyTT = 0; 
  }
  if (p) {
    delete p;
    p = 0;
  }
}

/*
\subsection{Value Mapping with index}

*/
int indexclassifyVM(Word* args, Word& result, int message, Word& local,
                    Supplier s){
  IndexLI *li = (IndexLI*)local.addr;
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      local.addr = new IndexLI(args[0], args[2], args[3], args[4]);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextResultTuple(true) : 0;
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
    appendSignature("rel(tuple(..., mstring, ...)) x attrname x invfile x "
                    "classifier -> stream(tuple(string, mstring))");
    syntax    = "_ indexclassify [_ , _ , _]";
    meaning   = "Classifies an indexed relation of trajectories according to a "
                " classifier";
  }
};

/*
\section{Operator ~compress~}

\subsection{Type Mapping}

*/
ListExpr compressTM(ListExpr args) {
  const string errMsg
          = "Expecting mlabel or mstring or stream(mlabel) or stream(mstring).";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(errMsg);
  }
  ListExpr arg = nl->First(args);
  if (MLabel::checkType(arg) || MString::checkType(arg)
   || Stream<MLabel>::checkType(arg) || Stream<MString>::checkType(arg)) {
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
  if (MString::checkType(arg)) return 1;
  if (Stream<MLabel>::checkType(arg)) return 2;
  if (Stream<MString>::checkType(arg)) return 3;
  return -1;
}

/*
\subsection{Value Mapping (for a single MLabel)}

*/
template<class T>
int compressVM_1(Word* args, Word& result, int message, Word& local,
                  Supplier s){
  T* mlabel = static_cast<T*>(args[0].addr);
  result = qp->ResultStorage(s);
  T* res = (T*)result.addr;
  T* tmp = mlabel->compress();
  res->CopyFrom(tmp);
  delete tmp;
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
        result.addr = mlabel->compress();
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
  if ((MString::checkType(arg) || MLabel::checkType(arg)
    || Stream<MString>::checkType(arg) || Stream<MLabel>::checkType(arg))
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
  if (MString::checkType(arg)) return 1;
  if (Stream<MLabel>::checkType(arg)) return 2;
  if (Stream<MString>::checkType(arg)) return 3;
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
  DateTime* dur = new DateTime(0, ccDur->GetValue(), durationtype);
  fillML(*source, *res, dur);
  dur->DeleteIfAllowed();
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
        DateTime* dur = new DateTime(0, ccInt->GetValue(), durationtype);
        fillML(*source, *res, dur);
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
int createmlVM(Word* args, Word& result, int message, Word& local, Supplier s){
  CcInt* ccint = static_cast<CcInt*>(args[0].addr);
  CcReal* ccreal = static_cast<CcReal*>(args[1].addr);
  int size;
  double rate;
  MLabel* ml = new MLabel(1);
  if (ccint->IsDefined() && ccreal->IsDefined()) {
    size = ccint->GetValue();
    rate = ccreal->GetValue();
    ml->createML(size, false, rate);
  }
  else {
//     cout << "Error: undefined value." << endl;
    ml->SetDefined(false);
  }
  result.addr = ml;
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

/*
\section{Operator ~index~}

\subsection{Type Mapping}

*/
ListExpr createindexTM(ListExpr args) {
  if (nl->ListLength(args) != 1) {
    return listutils::typeError("One argument expected.");
  }  
  if (MLabel::checkType(nl->First(args))||MString::checkType(nl->First(args))) {
    bool ml = MLabel::checkType(nl->First(args));
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->BoolAtom(ml)),
                             nl->SymbolAtom(MLabel::BasicType()));
  }
  return listutils::typeError("Argument type must be MLabel or MString.");
}

/*
\subsection{Value Mapping}

*/
int createindexVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  MLabel* sourceML = 0;
  MString* sourceMS = 0;
  MLabel* res = new MLabel(1);
  ULabel ul(1);
//   set<string> labels;
  set<size_t> positions;
  if (((CcBool*)args[1].addr)->GetBoolval()) {
    sourceML = (MLabel*)(args[0].addr);
    for (int i = 0; i < sourceML->GetNoComponents(); i++) {
      sourceML->Get(i, ul);
//       labels.insert(ul.constValue.GetValue());
      positions.insert(i);
      res->index.insert(ul.constValue.GetValue(), positions);
      positions.clear();
      res->Add(ul);
    }
  }
  else {
    sourceMS = (MString*)(args[0].addr);
    for (int i = 0; i < sourceMS->GetNoComponents(); i++) {
      sourceMS->Get(i, ul);
  //     labels.insert(ul.constValue.GetValue());
      positions.insert(i);
      res->index.insert(ul.constValue.GetValue(), positions);
      positions.clear();
      res->Add(ul);
    }
  }
  res->index.makePersistent();
  res->index.removeTrie();
//   res->index.printDbArrays();
//   res->index.printContents(labels);
  result.addr = res;
  return 0;
}

/*
\subsection{Operator Info}

*/
struct createindexInfo : OperatorInfo {
  createindexInfo() {
    name      = "createindex";
    signature = "mlabel -> mlabel";
    syntax    = "createindex(_)";
    meaning   = "Builds an index for a moving label.";
  }
};

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
  ULabel ul(1);
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
      for (int j = 0; j < ml->GetNoComponents(); j++) {
        ml->Get(j, ul);
        inv->insertString(tuple->GetTupleId(), ul.constValue.GetValue(), j, 0,
                          cache, trieCache);
      }
    }
    else {
      ms = (MString*)tuple->GetAttribute(((CcInt*)args[2].addr)->GetIntval()-1);
      for (int j = 0; j < ms->GetNoComponents(); j++) {
        ms->Get(j, ul);
        inv->insertString(tuple->GetTupleId(), ul.constValue.GetValue(), j, 0,
                          cache, trieCache);
      }
    }
  }
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
\section{Class ~SymbolicTrajectoryAlgebra~}

*/
  
class SymbolicTrajectoryAlgebra : public Algebra {
  public:
    SymbolicTrajectoryAlgebra() : Algebra() {

      AddTypeConstructor(&labelTC);
      AddTypeConstructor(&intimelabel);
      AddTypeConstructor(&unitlabel);
      AddTypeConstructor(&movinglabel);

      unitlabel.AssociateKind(Kind::DATA());
      movinglabel.AssociateKind(Kind::TEMPORAL());
      movinglabel.AssociateKind(Kind::DATA());

      AddTypeConstructor(&labelsTC);
      AddTypeConstructor(&patternTC);
      AddTypeConstructor(&classifierTC);

//       AddOperator(&temporalatinstantext);
      ValueMapping tolabelVMs[] = {tolabelVM<FText>, tolabelVM<CcString>, 0};
      
      AddOperator(tolabelInfo(), tolabelVMs, tolabelSelect, tolabelTM);
      
      AddOperator(mstringtomlabelInfo(), mstringtomlabelVM, mstringtomlabelTM);

      AddOperator(containsInfo(), containsVM, containsTM);

      AddOperator(the_unit_LabelInfo(), the_unit_Label_VM, the_unit_Label_TM);

      AddOperator(makemvalue_ULabelInfo(), makemvalue_ULabelVM,
                  makemvalue_ULabelTM);
      
      AddOperator(topatternInfo(), topatternVM, topatternTM);

      AddOperator(toclassifierInfo(), toclassifierVM, toclassifierTM);

      ValueMapping matchesVMs[] = {matchesVM_T, matchesVM_P, 0};
      
      AddOperator(matchesInfo(), matchesVMs, matchesSelect, matchesTM);

      AddOperator(indexmatchesInfo(), indexmatchesVM, indexmatchesTM);

      ValueMapping filtermatchesVMs[] = {filtermatchesVM_Text,
                                         filtermatchesVM_Pat, 0};

      AddOperator(filtermatchesInfo(), filtermatchesVMs, filtermatchesSelect,
                  filtermatchesTM);
      
      ValueMapping rewriteVMs[] = {rewriteVM_T, rewriteVM_P,
                                   rewriteVM_Stream, 0};
                                   
      AddOperator(rewriteInfo(), rewriteVMs, rewriteSelect, rewriteTM);

      AddOperator(classifyInfo(), classifyVM, classifyTM);

      AddOperator(indexclassifyInfo(), indexclassifyVM, indexclassifyTM);

      ValueMapping compressVMs[] = {compressVM_1<MLabel>,
                                    compressVM_1<MString>,
                                    compressVM_Str<MLabel>,
                                    compressVM_Str<MString>, 0};
      
      AddOperator(compressInfo(), compressVMs, compressSelect, compressTM);

      ValueMapping fillgapsVMs[] = {fillgapsVM_1<MLabel>,
                                    fillgapsVM_1<MString>,
                                    fillgapsVM_Str<MLabel>,
                                    fillgapsVM_Str<MString>, 0};
      
      AddOperator(fillgapsInfo(), fillgapsVMs, fillgapsSelect, fillgapsTM);

      AddOperator(createmlInfo(), createmlVM, createmlTM);

      AddOperator(createmlrelationInfo(), createmlrelationVM,
                  createmlrelationTM);

      AddOperator(createindexInfo(), createindexVM, createindexTM);

      AddOperator(createtrieInfo(), createtrieVM, createtrieTM);

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
