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
#include "GenericTC.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

#include <string>
#include <vector>
#include <math.h>

using namespace std;

namespace stj {

Label::Label(const string& value) : Attribute(true) {
  strncpy(text, value.c_str(), MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';
}

Label::Label(const Label& rhs) : Attribute(rhs) {
  strncpy(text, rhs.text, MAX_STRINGSIZE);
}

Label::Label(const bool def) : Attribute(def) {
  strncpy(text, "", MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';
}

Label::~Label() {}

string Label::GetValue() const {
  string value = text;
  return value;
}

void Label::Set(const bool def, const string &value) {
  SetDefined(def);
  SetValue(value);
}

void Label::SetValue(const string &value) {
  strncpy(text, value.c_str(), MAX_STRINGSIZE);
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
      result.addr = new Label(text);
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
  NList element(label->GetValue(), true);
  return element.listExpr();
}

Word Label::Create(const ListExpr typeInfo) {
  return (SetWord( new Label(true)));
}

void Label::Delete(const ListExpr typeInfo, Word& w) {
  delete static_cast<Label*>(w.addr);
  w.addr = 0;
}

void Label::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<Label*>(w.addr);
  w.addr = 0;
}

Word Label::Clone(const ListExpr typeInfo, const Word& w) {
  Label* label = static_cast<Label*>(w.addr);
  return SetWord(new Label(*label));
}

int Label::SizeOfObj() {
  return MAX_STRINGSIZE;
}

bool Label::Adjacent(const Attribute*) const {
  return false;
}

Label* Label::Clone() const {
  Label* result = new Label(this);
  return result;
}

size_t Label::HashValue() const {
  return 0;
}

bool Label::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual( type, Label::BasicType()));
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
  string value = ((Label*)right)->GetValue();
  strncpy(text, value.c_str(), MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';
}

int Label::Compare(const Attribute* arg) const {
  return this->GetValue().compare(((Label*)arg)->GetValue());
}

size_t Label::Sizeof() const {
  return sizeof(*this);
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
  CcString *ccstr = new CcString(true, label.GetValue());
  constValue.CopyFrom(ccstr);
  ccstr->DeleteIfAllowed();
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
MLabel::MLabel(MString* ms): MString(1) {
  ULabel ul(1);
  for (int i = 0; i < ms->GetNoComponents(); i++) {
    ms->Get(i, ul);
    Add(ul);
  }
  SetDefined(ms->IsDefined());
}

MLabel::MLabel(MLabel* ml): MString(0) {
  ULabel ul(1);
  for (int i = 0; i < ml->GetNoComponents(); i++) {
    ml->Get(i, ul);
    Add(ul);
  }
  SetDefined(ml->IsDefined());
}

MLabel::MLabel(const MLabel &ml) : MString(ml) {}

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
}

int MLabel::NumOfFLOBs() const {
  return 1;
}

Flob* MLabel::GetFLOB(const int i) {
  assert(i == 0);
  return &units;
}

bool MLabel::Passes(Label *label) {
  ULabel ul(0);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ul);
    if (ul.constValue.GetValue() == label->GetValue()) {
      return true;
    }
  }
  return false;
}

MLabel* MLabel::At(Label *label) {
  MLabel *result = new MLabel(1);
  ULabel ul(0);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ul);
    if (ul.constValue.GetValue() == label->GetValue()) {
      result->Add(ul);
    }
  }
  return result;
}

void MLabel::DefTime(Periods *per) {
  per->Clear();
  ULabel ul(0);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ul);
    per->MergeAdd(ul.timeInterval);
  }
}

void MLabel::Inside(MBool *mbool, Label *label) {
  mbool->Clear();
  ULabel ul(0);
  for (int i = 0; i < GetNoComponents(); i++) {
    Get(i, ul);
    CcBool ccbool(true, ul.constValue.GetValue() == label->GetValue());
    UBool ub(ul.timeInterval, ccbool);
    mbool->MergeAdd(ub);
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
  return newML;
}

/*
\subsubsection{Function ~create~}

Creates an MLabel of a certain size for testing purposes. The labels will be
contain only numbers between 1 and size[*]rate; rate being the number of
different labels divided by the size.

*/
void MLabel::createML(int size, bool text, double rate = 1.0) {
  if ((size > 0) && (rate > 0) && (rate <= 1)) {
    int max = size * rate;
    MLabel* newML = new MLabel(1);
    ULabel ul(1);
    DateTime* start = new DateTime(instanttype);
    DateTime* halfHour = new DateTime(0, 1800000, durationtype); // duration
    start->Set(2014, 1, 1);
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
    newML->TrimToSize();
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
  MLabel *result = new MLabel(0);
  result->SetDefined(true);
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
                assigns[i].setLabelPtr(k, ul.constValue.GetValue());
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
  result->TrimToSize();
  result->SetDefined(result->IsValid());
  return result;
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

/*
\subsection{Function ~CompareLabels~}

*/
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

/*
\subsection{Print function for class ~Label~}

*/
ostream& operator<<(ostream& os, const Label& lb) {
  os << "(" << lb.GetValue() << ")";
  return os;
}

/*
\section{Implementation of class ~Labels~}

\subsection{Constructors}

*/
Labels::Labels(const int n, const Label *lb) : Attribute(true), labels(n) {
  SetDefined(true);
  if (n > 0) {
    for(int i = 0; i < n; i++) {
      Append(lb[i]);
    }
  }
}

Labels::Labels(const bool defined) : Attribute(defined), labels(0) {}

Labels::Labels(const Labels& src):
  Attribute(src.IsDefined()), labels(src.labels.Size()) {
  labels.copyFrom(src.labels);
}

/*
\subsection{Destructor}

*/
Labels::~Labels() {}

/*
\subsection{Operator ~=~}

*/
Labels& Labels::operator=(const Labels& src) {
  Attribute::operator=(src);
  labels.copyFrom(src.labels);
  return *this;
}

/*
\subsection{Function ~NumOfFLOBs~}

*/
int Labels::NumOfFLOBs() const {
  return 1;
}

/*
\subsection{Function ~GetFLOB~}

*/
Flob *Labels::GetFLOB(const int i) {
  assert(i >= 0 && i < NumOfFLOBs());
  return &labels;
}

/*
\subsection{Function ~Compare~}

*/
int Labels::Compare(const Attribute* arg) const {
  if (NumOfFLOBs() > arg->NumOfFLOBs()) {
    return 1;
  }
  if (NumOfFLOBs() < arg->NumOfFLOBs()) {
    return -1;
  }
  return 0;
}

/*
\subsection{Function ~HashValue~}

*/
size_t Labels::HashValue() const {
  return  1;
}

/*
\subsection{Function ~Adjacent~}

*/
bool Labels::Adjacent(const Attribute*) const {
  return 0;
}

/*
\subsection{Function ~Clone~}

*/
Labels *Labels::Clone() const {
  Labels *lbs = new Labels(*this);
  return lbs;
}

/*
\subsection{Function ~CopyFrom~}

*/
void Labels::CopyFrom(const Attribute* right) {
  *this = *((Labels*)right);
}

/*
\subsection{Function ~Sizeof~}

*/
size_t Labels::Sizeof() const {
  return sizeof(*this);
}

/*
\subsection{Function ~Print~}

*/
ostream& Labels::Print(ostream& os) const {
  return (os << *this);
}

/*
\subsection{Function ~Append~}

*/
void Labels::Append(const Label& lb) {
  labels.Append(lb);
}

/*
\subsection{Function ~Destroy~}

*/
void Labels::Destroy() {
  labels.destroy();
}

/*
\subsection{Function ~GetNoLabels~}

*/
int Labels::GetNoLabels() const {
  return labels.Size();
}

/*
\subsection{Function ~GetLabel~}

*/
Label Labels::GetLabel(int i) const {
  assert((0 <= i) && (i < GetNoLabels()));
  Label lb(true);
  labels.Get(i, lb);
  return lb;
}

/*
\subsection{Function ~IsEmpty~}

*/
const bool Labels::IsEmpty() const {
  return GetNoLabels() == 0;
}

/*
\subsection{Operator ~<<~}

*/
ostream& operator<<(ostream& os, const Labels& lbs) {
  for(int i = 0; i < lbs.GetNoLabels() - 1; i++) {
    os << lbs.GetLabel(i) << " ";
  }
  os << lbs.GetLabel(lbs.GetNoLabels() - 1);
  return os;
}

/*
\subsection{Function ~Out~}

*/
ListExpr Labels::ToListExpr(ListExpr typeInfo) {
  if (!IsDefined()) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (IsEmpty()) {
    return nl->Empty();
  }
  else {
    ListExpr res = nl->OneElemList(nl->StringAtom(GetLabel(0).GetValue()));
    ListExpr last = res;
    for (int i = 1; i < GetNoLabels(); i++) {
      last = nl->Append(last, nl->StringAtom(GetLabel(i).GetValue()));
    }
    return res;
  }
}

/*
\subsection{Function ~Out~}

*/
ListExpr Labels::Out(ListExpr typeInfo, Word value) {
  Labels* lbs = static_cast<Labels*>(value.addr);
  return lbs->ToListExpr(typeInfo);
}

/*
\subsection{Function ~In~}

*/
Word Labels::In(const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word result = SetWord(Address(0));
  correct = false;
  NList list(instance); 
  Labels* lbs = new Labels(0);
  lbs->SetDefined(true);
  while (!list.isEmpty()) {
    if (!list.isAtom() && list.first().isAtom() && list.first().isString()) {
      Label label(list.first().str());
      lbs->Append(label);
      correct = true;
    }
    else {
      correct = false;
      cmsg.inFunError("Expecting a list of string atoms!");
      delete lbs;
      return SetWord(Address(0));
    }
    list.rest();
  }
//   labels->labels.Sort(CompareLabels);
  result.addr = lbs;
  return result;
}

/*
\subsection{Function ~Property~}

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
\subsection{Function ~KindCheck~}

*/
bool Labels::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Labels::BasicType()));
}

/*
\subsection{Function ~Create~}

*/
Word Labels::Create(const ListExpr typeInfo) {
  Labels* labels = new Labels(0);
  return (SetWord(labels));
}

/*
\subsection{Function ~Delete~}

*/
void Labels::Delete(const ListExpr typeInfo, Word& w) {
  Labels* labels = (Labels*)w.addr;
  labels->Destroy();
  delete labels;
}

/*
\subsection{Function ~Open~}

*/
bool Labels::Open(SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value) {
  Labels *lbs = (Labels*)Attribute::Open(valueRecord, offset, typeInfo);
  value.setAddr(lbs);
  return true;
}

/*
\subsection{Function ~Save~}

*/
bool Labels::Save(SmiRecord& valueRecord, size_t& offset,
             const ListExpr typeInfo, Word& value) {
  Labels *lbs = (Labels *)value.addr;
  Attribute::Save(valueRecord, offset, typeInfo, lbs);
  return true;
}

/*
\subsection{Function ~Close~}

*/
void Labels::Close(const ListExpr typeInfo, Word& w) {
  Labels* labels = (Labels*)w.addr;
  delete labels;
}

/*
\subsection{Function ~Clone~}

*/
Word Labels::Clone(const ListExpr typeInfo, const Word& w) {
  return SetWord(((Labels*)w.addr)->Clone());
}

/*
\subsection{Function ~SizeOfObj~}

*/
int Labels::SizeOfObj() {
  return sizeof(Labels);
}

/*
\subsection{Function ~Cast~}

*/
void* Labels::Cast(void* addr) {
  return (new (addr)Labels);
}

/*
\subsection{Type Constructor}

*/
TypeConstructor labelsTC(
        Labels::BasicType(),               //name
        Labels::Property,                  //property function
        Labels::Out,   Labels::In,         //Out and In functions
        0,              0,                 //SaveTo and RestoreFrom functions
        Labels::Create,  Labels::Delete,   //object creation and deletion
        OpenAttribute<Labels>, 
        SaveAttribute<Labels>,             //object open and save
        Labels::Close,   Labels::Clone,    //object close and clone
        Labels::Cast,                      //cast function
        Labels::SizeOfObj,                 //sizeof function
        Labels::KindCheck );               //kind checking function

/*
\section{Implementation of class ~ILabels~}

\subsection{Constructors}

*/
ILabels::ILabels(const bool defined) : Intime<Labels>(defined) {
  SetDefined(defined);
}

ILabels::ILabels(const ILabels &ils) : Intime<Labels>(ils.instant, ils.value) {
  SetDefined(ils.IsDefined());
}

ILabels::ILabels(const Instant &inst, const Labels &lbs) : 
  Intime<Labels>(inst, lbs) {}

/*
\subsection{Function ~Property~}

*/
ListExpr ILabels::Property() {
  return (nl->TwoElemList(
         nl->FourElemList(nl->StringAtom("Signature"),
                          nl->StringAtom("Example Type List"),
                          nl->StringAtom("List Rep"),
                          nl->StringAtom("Example List")),
         nl->FourElemList(nl->StringAtom("->" + Kind::DATA() ),
                          nl->StringAtom(ILabels::BasicType()),
                          nl->StringAtom("<instant> <labels>)"),
                          nl->StringAtom("(\"2014-01-28\" (\"Dortmund\" "
                          "\"home\")"))));
}

/*
\subsection{Function ~Create~}

*/
Word ILabels::Create(const ListExpr typeInfo) {
  return SetWord(new ILabels(false));
}

/*
\subsection{Function ~Delete~}

*/
void ILabels::Delete(const ListExpr typeInfo, Word &w) {
  ILabels *ils = (ILabels*)w.addr;
  delete ils;
  w.addr = 0;
}

/*
 \subsection{Function ~Close~}
 
*/
void ILabels::Close(const ListExpr typeInfo, Word &w) {
  ILabels *ils = (ILabels*)w.addr;
  delete ils;
  w.addr = 0;
}

/*
\subsection{Function ~Clone~}

*/
Word ILabels::Clone(const ListExpr typeInfo, const Word& w) {
  return SetWord(new ILabels(*((ILabels*)w.addr)));
}

/*
\subsection{Function ~KindCheck~}

*/
bool ILabels::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, ILabels::BasicType());
}

/*
\subsection{Type Constructor}

*/
TypeConstructor intimelabels(
  ILabels::BasicType(),               //name
  ILabels::Property,                  //property function
  OutIntime<Labels, Labels::Out>, 
  InIntime<Labels, Labels::In>,       //Out and In functions
  0,              0,                  //SaveTo and RestoreFrom functions
  ILabels::Create,  ILabels::Delete,  //object creation and deletion
  OpenAttribute<ILabels>,
  SaveAttribute<ILabels>,             //object open and save
  ILabels::Close,   ILabels::Clone,   //object close and clone
  ILabels::Cast,                      //cast function
  ILabels::SizeOfObj,                 //sizeof function
  ILabels::KindCheck );               //kind checking function

/*
\section{Implementation of class ~ULabels~}

\subsection{Constructors}

*/
ULabels::ULabels(bool defined) : ConstTemporalUnit<Labels>(defined) {}

ULabels::ULabels(const SecInterval &iv, const Labels &lbs)
  : ConstTemporalUnit<Labels>(iv, lbs) {
  SetDefined(true);
}

ULabels::ULabels(int i, MLabels &mls) 
  : ConstTemporalUnit<Labels>(mls.IsDefined() && i < mls.GetNoComponents()) {
  if ((i >= 0) && (i < mls.GetNoComponents())) {
    timeInterval = mls.GetInterval(i);
    constValue.Clean();
    vector<Label> *labelsVec = mls.GetLabels(i);
    for (unsigned int j = 0; j < labelsVec->size(); j++) {
      constValue.Append((*labelsVec)[j]);
    }
  }
}

ULabels::ULabels(const ULabels& uls) : ConstTemporalUnit<Labels>(uls) {
  SetDefined(uls.IsDefined());
}

/*
\subsection{Function ~Property~}

*/
ListExpr ULabels::Property() {
  return (nl->TwoElemList(
         nl->FiveElemList(nl->StringAtom("Signature"),
                          nl->StringAtom("Example Type List"),
                          nl->StringAtom("List Rep"),
                          nl->StringAtom("Example List"),
                          nl->StringAtom("")),
         nl->FiveElemList(nl->StringAtom("->" + Kind::DATA() ),
                          nl->StringAtom(ULabels::BasicType()),
                          nl->StringAtom("<interval> <labels>)"),
                          nl->StringAtom("((\"2014-01-29\" \"2014-01-30\" "),
                          nl->StringAtom("TRUE FALSE) (\"home\" \"work\"))"))));
}

/*
\subsection{Function ~Create~}

*/
Word ULabels::Create(const ListExpr typeInfo) {
  return SetWord(new ULabels(false));
}

/*
\subsection{Function ~Delete~}

*/
void ULabels::Delete(const ListExpr typeInfo, Word &w) {
  ULabels *uls = (ULabels*)w.addr;
  delete uls;
  w.addr = 0;
}

/*
 \subsection{Function ~Close~}
 
*/
void ULabels::Close(const ListExpr typeInfo, Word &w) {
  ULabels *uls = (ULabels*)w.addr;
  delete uls;
  w.addr = 0;
}

/*
\subsection{Function ~Clone~}

*/
Word ULabels::Clone(const ListExpr typeInfo, const Word& w) {
  return SetWord(new ULabels(*((ULabels*)w.addr)));
}

/*
\subsection{Function ~KindCheck~}

*/
bool ULabels::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, ULabels::BasicType());
}

/*
\subsection{Type Constructor}

*/
TypeConstructor unitlabels(
  ULabels::BasicType(),                   //name
  ULabels::Property,                      //property function
  OutConstTemporalUnit<Labels, Labels::Out>,
  InConstTemporalUnit<Labels, Labels::In>,//Out and In functions
  0,              0,                      //SaveTo and RestoreFrom functions
  ULabels::Create,  ULabels::Delete,      //object creation and deletion
  OpenAttribute<ULabels>, 
  SaveAttribute<ULabels>,                 //object open and save
  ULabels::Close,   ULabels::Clone,       //object close and clone
  ULabels::Cast,                          //cast function
  ULabels::SizeOfObj,                     //sizeof function
  ULabels::KindCheck );                   //kind checking functions

/*
\section{Implementation of class ~ULabels~}

\subsection{Constructors}

*/
MLabels::MLabels(int n) : Attribute(n > 0), units(n), labels(0) {}

MLabels::MLabels(const MLabels &mls) : Attribute(mls.IsDefined()), 
                                       units(mls.GetNoComponents()), labels(0) {
  labels.copyFrom(mls.labels);
  units.copyFrom(mls.units);
}

/*
\subsection{Function ~Property~}

*/
ListExpr MLabels::Property() {
  return gentc::GenProperty("-> DATA", BasicType(),
    "((<interval> <labels>) (<interval> <labels>) ...)",
    "(((\"2014-01-29\" \"2014-01-30\" TRUE FALSE) (\"home\" \"Dortmund\")))");
}

/*
\subsection{Function ~KindCheck~}

*/
bool MLabels::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return nl->IsEqual(type, MLabels::BasicType());
}

/*
\subsection{Function ~GetFLOB~}

*/
Flob* MLabels::GetFLOB(const int i) {
  assert((i >= 0) && (i < 2));
  if (i == 0) {
    return &units;
  }
  return &labels;
}

/*
\subsection{Function ~Compare~}

*/
int MLabels::Compare(const Attribute* arg) const {
  if (units.Size() == ((MLabels*)arg)->units.Size()) {
    if (labels.Size() == ((MLabels*)arg)->labels.Size()) {
      return 0;
    }
    return (labels.Size() > ((MLabels*)arg)->labels.Size() ? 1 : -1);
  }
  else {
    return (units.Size() > ((MLabels*)arg)->units.Size() ? 1 : -1);
  }
}

/*
\subsection{Function ~getEndPos~}

*/
int MLabels::getEndPos(int i) const {
  if (i < GetNoComponents() - 1) {
    MLabelsUnit nextUnit(true);
    units.Get(i + 1, nextUnit);
    return nextUnit.startPos - 1;
  }
  else { // last unit
    return labels.Size() - 1;
  }
}

/*
\subsection{Function ~labelsToListExpr~}

*/
ListExpr MLabels::labelsToListExpr(int start, int end) {
  if (start > end) {
    return nl->Empty();
  }
  if ((start < 0) || (end >= GetNoLabels())) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  Label lb(true);
  labels.Get(start, lb);
  ListExpr result = nl->OneElemList(nl->StringAtom(lb.GetValue()));
  ListExpr last = result;
  for (int i = start + 1; i <= end; i++) {
    labels.Get(i, lb);
    last = nl->Append(last, nl->StringAtom(lb.GetValue()));
  }
  return result;
}

/*
\subsection{Function ~ToListExpr~}

*/
ListExpr MLabels::unitToListExpr(int i) {
  if ((i < 0) || (i >= GetNoComponents())) {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  MLabelsUnit unit(-1);
  units.Get(i, unit);
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  return nl->TwoElemList(unit.interval.ToListExpr(sc->GetTypeExpr(
                                                     SecInterval::BasicType())),
                         labelsToListExpr(unit.startPos, getEndPos(i)));
}

/*
\subsection{Function ~ToListExpr~}

*/
ListExpr MLabels::ToListExpr(ListExpr typeInfo) {
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
\subsection{Function ~readLabels~}

*/
void MLabels::readLabels(ListExpr labelslist) {
  ListExpr rest = labelslist;
  while (!nl->IsEmpty(rest)) {
    if (!listutils::isSymbolUndefined(nl->First(rest))) {
      string labelStr = nl->ToString(nl->First(rest));
      Label label(labelStr.substr(1, labelStr.length() - 2));
      labels.Append(label);
    }
    rest = nl->Rest(rest);
  }
}

/*
\subsection{Function ~readUnit~}

*/
bool MLabels::readUnit(ListExpr unitlist) {
  if (!nl->HasLength(unitlist, 2)) {
    return false;
  }
  MLabelsUnit unit(GetNoLabels());
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  if (!unit.interval.ReadFrom(nl->First(unitlist), 
                              sc->GetTypeExpr(SecInterval::BasicType()))) {
    return false;
  }
  unit.interval.SetDefined(true);
  readLabels(nl->Second(unitlist));
  units.Append(unit);
  return true;
}

/*
\subsection{Function ~ReadFrom~}

*/
bool MLabels::ReadFrom(ListExpr LE, ListExpr typeInfo) {
  if (listutils::isSymbolUndefined(LE)) {
    SetDefined(false);
    units.clean();
    labels.clean();
    return true;
  }
  ListExpr rest = LE;
  while (!nl->IsEmpty(rest)) {
    if (!readUnit(nl->First(rest))) {
      SetDefined(false);
      return false;
    }
    rest = nl->Rest(rest);
  }
  SetDefined(true);
  return true;
}

/*
\subsection{Function ~unitToString~}

*/
string MLabels::unitToString(int i) const {
  if ((i < 0) || (i >= GetNoComponents())) {
    return "ERROR: invalid unit " + i;
  }
  MLabelsUnit unit(-1);
  units.Get(i, unit);
  string labelsStr = "";
  if (unit.startPos > -1) {
    for (int j = unit.startPos; j <= getEndPos(i); j++) {
      Label label(true);
      labels.Get(j, label);
      labelsStr.append(label.GetValue() + (j < getEndPos(i) ? " " : ""));
    }
  }
  return unit.interval.ToString() + "(" + labelsStr + ")\n";
}

/*
\subsection{Function ~toString~}

*/
string MLabels::toString() const {
  if (!IsDefined()) {
    return "(undefined)";
  }
  string result = "(\n";
  for (int i = 0; i < GetNoComponents(); i++) {
    result.append(unitToString(i));
  }
  return result + ")";
}

/*
\subsection{Operator ~<<~}

*/
ostream& operator<<(ostream& o, const MLabels& mls) {
  o << mls.toString();
  return o;
}

/*
\subsection{Function ~GetLabels~}

*/
vector<Label>* MLabels::GetLabels(int i) const {
  vector<Label>* result(0);
  MLabelsUnit unit(-1);
  for (int j = unit.startPos; j <= getEndPos(i); j++) {
    Label label(true);
    labels.Get(j, label);
    result->push_back(label);
  }
  return result;
}

/*
\subsection{Function ~GetInterval~}

*/
SecInterval MLabels::GetInterval(int i) const {
  if ((i < 0) || (i >= GetNoComponents())) {
    SecInterval iv(false);
    return iv;
  }
  MLabelsUnit unit(-1);
  return unit.interval;
}

/*
\subsection{Type Constructor}

*/
GenTC<MLabels> movinglabels;

/*
TypeConstructor movinglabels(
  MLabels::BasicType(),                   //name
  MLabels::Property,                      //property function
  OutMapping<MLabels, ULabels, OutConstTemporalUnit<Labels, Labels::Out> >,
  InMapping<MLabels, ULabels, InConstTemporalUnit<Labels, Labels::In> >,//Out and In functions
  0,              0,                      //SaveTo and RestoreFrom functions
  MLabels::Create,  MLabels::Delete,      //object creation and deletion
  OpenAttribute<MLabels>, 
  SaveAttribute<MLabels>,                 //object open and save
  MLabels::Close,   MLabels::Clone,       //object close and clone
  MLabels::Cast,                          //cast function
  MLabels::SizeOfObj,                     //sizeof function
  MLabels::KindCheck );                   //kind checking functions
*/

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
    res->Set(true, source->GetValue());
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
    res->Set(true, source->GetValue());
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
  Label *label = new Label(ccstr->GetValue());
  int pos;
  bool res = labels->GetDbArray().Find(label, CompareLabels, pos);
  delete label;
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
  ULabel ul(0);
  ml->Get(ulId, ul);
  if (store) {
    if (ulId < ml->GetNoComponents() - 1) { // usual case
      for (itm = transitions.begin(); itm != transitions.end(); itm++) {
        if (labelsMatch(ul.constValue.GetValue(), elems[itm->first].getL())
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
        if (labelsMatch(ul.constValue.GetValue(), elems[itm->first].getL())
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
      if (labelsMatch(ul.constValue.GetValue(), elems[itm->first].getL())
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
      switch (cond.getKey(i)) {
        case 0: { // label
          ml->Get(from, ul);
          cond.setLabelPtr(i, ul.constValue.GetValue());
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
            ml->Get(j, ul);
            Label *label = new Label(ul.constValue.GetValue());
            cond.appendToLabelsPtr(i, *label);
            delete label;
          }
          cond.completeLabelsPtr(i);
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

void Condition::completeLabelsPtr(unsigned int pos) {
  if (pos < pointers.size()) {
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

makemvalue: stream (tuple ((x1 t1)...(xi ulabel)...(xn tn))) xi -> mlabel

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
\section{Operator ~passes~}

passes: mlabel x label -> bool

\subsection{Type Mapping}

*/
ListExpr passesMLabelTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    if (MLabel::checkType(nl->First(args)) &&
        Label::checkType(nl->Second(args))) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  mlabel x label -> bool");
}

/*
\subsection{Value Mapping}

*/
int passesMLabelVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  MLabel *ml = static_cast<MLabel*>(args[0].addr);
  Label *label = static_cast<Label*>(args[1].addr);
  if (ml->IsDefined() && label->IsDefined()) {
    res->Set(true, ml->Passes(label));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct passesMLabelInfo : OperatorInfo {
  passesMLabelInfo() {
    name      = "passes";
    signature = "mlabel x label -> bool";
    syntax    = "_ passes _ ";
    meaning   = "Returns TRUE if and only if the label occurs at least once "
                "in the mlabel.";
  }
};

/*
\section{Operator ~at~}

at: mlabel x label -> mlabel

\subsection{Type Mapping}

*/
ListExpr atMLabelTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    if (MLabel::checkType(nl->First(args)) &&
        Label::checkType(nl->Second(args))) {
      return nl->SymbolAtom(MLabel::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  mlabel x label -> mlabel");
}

/*
\subsection{Value Mapping}

*/
int atMLabelVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  MLabel *src = static_cast<MLabel*>(args[0].addr);
  Label *label = static_cast<Label*>(args[1].addr);
  if (src->IsDefined() && label->IsDefined()) {
    result.addr = src->At(label);
  }
  else {
    result.addr = 0;
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct atMLabelInfo : OperatorInfo {
  atMLabelInfo() {
    name      = "at";
    signature = "mlabel x label -> mlabel";
    syntax    = "_ at _ ";
    meaning   = "Reduces the mlabel to those units whose label equals the "
                "label.";
  }
};

/*
\section{Operator ~deftime~}

deftime: mlabel -> periods

\subsection{Type Mapping}

*/
ListExpr deftimeMLabelTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (MLabel::checkType(nl->First(args))) {
      return nl->SymbolAtom(Periods::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  mlabel -> periods");
}

/*
\subsection{Value Mapping}

*/
int deftimeMLabelVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  result = qp->ResultStorage(s);
  Periods* res = static_cast<Periods*>(result.addr);
  MLabel *src = static_cast<MLabel*>(args[0].addr);
  if (src->IsDefined()) {
    src->DefTime(res);
  }
  else {
    res = 0;
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct deftimeMLabelInfo : OperatorInfo {
  deftimeMLabelInfo() {
    name      = "deftime";
    signature = "mlabel -> periods";
    syntax    = "deftime ( _ )";
    meaning   = "Returns the periods containing the time intervals during which"
                "the mlabel is defined.";
  }
};

/*
\section{Operator ~units~}

units: mlabel -> (stream ulabel)

\subsection{Type Mapping}

*/
ListExpr unitsMLabelTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (MLabel::checkType(nl->First(args))) {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                             nl->SymbolAtom(ULabel::BasicType()));
    }
  }
  return listutils::typeError("Correct signature:  mlabel -> (stream ulabel)");
}

ULabel* UnitsLI::getNextUnit() {
  if ((index >= ml->GetNoComponents()) || (index < 0)) {
    return 0;
  }
  ULabel result(true);
  ml->Get(index, result);
  index++;
  return (ULabel*)(result.Clone());
}

/*
\subsection{Value Mapping}

*/
int unitsMLabelVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  MLabel *source = static_cast<MLabel*>(args[0].addr);
  UnitsLI *li = static_cast<UnitsLI*>(local.addr);
  switch (message) {
    case OPEN: {
      if (li) {
        li = 0;
      }
      li = new UnitsLI(source);
      local.addr = li;
      return 0;
    }
    case REQUEST: {
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      li = (UnitsLI*)local.addr;
      result.addr = li->getNextUnit();
      return (result.addr ? YIELD : CANCEL);
    }
    case CLOSE: {
      if (local.addr) {
        li = (UnitsLI*)local.addr;
        delete li;
        local.addr=0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct unitsMLabelInfo : OperatorInfo {
  unitsMLabelInfo() {
    name      = "units";
    signature = "mlabel -> (stream ulabel)";
    syntax    = "units ( _ )";
    meaning   = "Splits a mlabel into its units and returns them as a stream.";
  }
};

/*
\section{Operator ~initial~}

initial: ulabel -> ilabel

\subsection{Type Mapping}

*/
ListExpr initialULabelTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (ULabel::checkType(nl->First(args))) {
      return nl->SymbolAtom(ILabel::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  ulabel -> ilabel");
}

void ULabel::Initial(ILabel *result) {
  result->instant = timeInterval.start;
  result->value.Set(true, constValue.GetValue());
  result->SetDefined(true);
}

/*
\subsection{Value Mapping}

*/
int initialULabelVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  result = qp->ResultStorage(s);
  ILabel* res = static_cast<ILabel*>(result.addr);
  ULabel *src = static_cast<ULabel*>(args[0].addr);
  if (src->IsDefined()) {
    src->Initial(res);
  }
  else {
    res = 0;
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct initialULabelInfo : OperatorInfo {
  initialULabelInfo() {
    name      = "initial";
    signature = "ulabel -> ilabel";
    syntax    = "initial ( _ )";
    meaning   = "Returns the ilabel belonging to the initial instant of the "
                "ulabel.";
  }
};

/*
\section{Operator ~final~}

initial: ulabel -> ilabel

\subsection{Type Mapping}

*/
ListExpr finalULabelTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (ULabel::checkType(nl->First(args))) {
      return nl->SymbolAtom(ILabel::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  ulabel -> ilabel");
}

void ULabel::Final(ILabel *result) {
  result->instant = timeInterval.end;
  result->value.Set(true, constValue.GetValue());
  result->SetDefined(true);
}

/*
\subsection{Value Mapping}

*/
int finalULabelVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  result = qp->ResultStorage(s);
  ILabel* res = static_cast<ILabel*>(result.addr);
  ULabel *src = static_cast<ULabel*>(args[0].addr);
  if (src->IsDefined()) {
    src->Final(res);
  }
  else {
    res = 0;
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct finalULabelInfo : OperatorInfo {
  finalULabelInfo() {
    name      = "final";
    signature = "ulabel -> ilabel";
    syntax    = "final ( _ )";
    meaning   = "Returns the ilabel belonging to the final instant of the "
                "ulabel.";
  }
};

/*
\section{Operator ~val~}

val: ilabel -> label

\subsection{Type Mapping}

*/
ListExpr valILabelTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (ILabel::checkType(nl->First(args))) {
      return nl->SymbolAtom(Label::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  ilabel -> label");
}

/*
\subsection{Value Mapping}

*/
int valILabelVM(Word* args, Word& result, int message, Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Label* res = static_cast<Label*>(result.addr);
  ILabel *src = static_cast<ILabel*>(args[0].addr);
  if (src->IsDefined()) {
    res->SetDefined(true);
    res->SetValue(src->value.GetValue());
  }
  else {
    res = 0;
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct valILabelInfo : OperatorInfo {
  valILabelInfo() {
    name      = "val";
    signature = "ilabel -> label";
    syntax    = "val ( _ )";
    meaning   = "Returns the value of the ilabel.";
  }
};

/*
\section{Operator ~inst~}

inst: ilabel -> instant

\subsection{Type Mapping}

*/
ListExpr instILabelTM(ListExpr args) {
  if (nl->HasLength(args, 1)) {
    if (ILabel::checkType(nl->First(args))) {
      return nl->SymbolAtom(Instant::BasicType());
    }
  }
  return listutils::typeError("Correct signature:  ilabel -> instant");
}

/*
\subsection{Value Mapping}

*/
int instILabelVM(Word* args, Word& result, int message, Word& local,Supplier s){
  result = qp->ResultStorage(s);
  Instant* res = static_cast<Instant*>(result.addr);
  ILabel *src = static_cast<ILabel*>(args[0].addr);
  if (src->IsDefined()) {
    res->CopyFrom(&(src->instant));
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct instILabelInfo : OperatorInfo {
  instILabelInfo() {
    name      = "inst";
    signature = "ilabel -> instant";
    syntax    = "inst ( _ )";
    meaning   = "Returns the instant of the ilabel.";
  }
};

/*
\section{Operator ~inside~}

inside: mlabel x label -> mbool

\subsection{Type Mapping}

*/
ListExpr insideMLabelTM(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    if (MLabel::checkType(nl->First(args)) &&
        Label::checkType(nl->Second(args))) {
      return nl->SymbolAtom(MBool::BasicType());
    }
  }
  return listutils::typeError("Correct signature: mlabel x label -> mbool");
}

/*
\subsection{Value Mapping}

*/
int insideMLabelVM(Word* args, Word& result, int message, Word& local,
                   Supplier s){
  result = qp->ResultStorage(s);
  MBool* res = static_cast<MBool*>(result.addr);
  MLabel *src = static_cast<MLabel*>(args[0].addr);
  Label *label = static_cast<Label*>(args[1].addr);
  if (src->IsDefined() && label->IsDefined()) {
    src->Inside(res, label);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

/*
\subsection{Operator Info}

*/
struct insideMLabelInfo : OperatorInfo {
  insideMLabelInfo() {
    name      = "inside";
    signature = "mlabel x label -> mbool";
    syntax    = "_ inside _";
    meaning   = "Returns a mbool with the same time intervals as the mlabel. "
                "A unit\'s value is TRUE if and only if its value equals the "
                "specified label.";
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
        if (pText->IsDefined()) {
          local.addr = Pattern::getPattern(pText->toText());
        }
        else {
          cout << "Undefined pattern text." << endl;
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

      AddTypeConstructor(&labelTC);
      AddTypeConstructor(&intimelabel);
      AddTypeConstructor(&unitlabel);
      AddTypeConstructor(&movinglabel);
      
      AddTypeConstructor(&intimelabels);
      AddTypeConstructor(&unitlabels);
      AddTypeConstructor(&movinglabels);

      unitlabel.AssociateKind(Kind::DATA());
      movinglabel.AssociateKind(Kind::TEMPORAL());
      movinglabel.AssociateKind(Kind::DATA());
      intimelabels.AssociateKind(Kind::DATA());
      unitlabels.AssociateKind(Kind::DATA());
      movinglabels.AssociateKind(Kind::DATA());

      AddTypeConstructor(&labelsTC);
      AddTypeConstructor(&patternTC);
      AddTypeConstructor(&classifierTC);

//       AddOperator(&temporalatinstantext);
      ValueMapping tolabelVMs[] = {tolabelVM<FText>, tolabelVM<CcString>, 0};
      
      AddOperator(tolabelInfo(), tolabelVMs, tolabelSelect, tolabelTM);

      AddOperator(tostringInfo(), tostringVM, tostringTM);

      AddOperator(totextInfo(), totextVM, totextTM);
      
      AddOperator(mstringtomlabelInfo(), mstringtomlabelVM, mstringtomlabelTM);

      AddOperator(containsInfo(), containsVM, containsTM);

      AddOperator(the_unit_LabelInfo(), the_unit_Label_VM, the_unit_Label_TM);

      AddOperator(makemvalue_ULabelInfo(), makemvalue_ULabelVM,
                  makemvalue_ULabelTM);

      AddOperator(passesMLabelInfo(), passesMLabelVM, passesMLabelTM);

      AddOperator(atMLabelInfo(), atMLabelVM, atMLabelTM);

      AddOperator(deftimeMLabelInfo(), deftimeMLabelVM, deftimeMLabelTM);

      AddOperator(unitsMLabelInfo(), unitsMLabelVM, unitsMLabelTM);

      AddOperator(initialULabelInfo(), initialULabelVM, initialULabelTM);

      AddOperator(finalULabelInfo(), finalULabelVM, finalULabelTM);

      AddOperator(valILabelInfo(), valILabelVM, valILabelTM);

      AddOperator(instILabelInfo(), instILabelVM, instILabelTM);

      AddOperator(insideMLabelInfo(), insideMLabelVM, insideMLabelTM);
      
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
