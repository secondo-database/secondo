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
#include "SymbolicTrajectoryTools.h"
#include "Stream.h"
#include "SecParser.h"
#include "Pattern.h"
#include "TemporalUnitAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include <string>
#include <vector>

using namespace std;

namespace stj {

class Label {
 public:
  Label() {};
  Label(string text);
  Label(char* Text);
  Label(const Label& rhs);
  ~Label();
  
  string GetText() const;
  void SetText(string &text);
  Label* Clone();
  
  // algebra support functions
  
  static Word     In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word     Create(const ListExpr typeInfo);
  static void     Delete(const ListExpr typeInfo, Word& w);
  static void     Close(const ListExpr typeInfo, Word& w);
  static Word     Clone(const ListExpr typeInfo, const Word& w);
  static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj();
  static ListExpr Property();  
  static const string BasicType() {
    return "label";
  }
  static const bool checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }

 private:
//  Label() {}

  char text[MAX_STRINGSIZE+1];
};

Label::Label(string Text) {
  strncpy(text, Text.c_str(), MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';
}

Label::Label(char* Text) {
  strncpy(text, Text, MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';
}

Label::Label(const Label& rhs) {
  strncpy(text, rhs.text, MAX_STRINGSIZE); 
}

Label::~Label() {}

string Label::GetText() const {
  string str = text;
  return str;
}

void Label::SetText(string &Text) {
  strncpy(text, Text.c_str(), MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';    
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
  NList element (label->GetText(), true);
  return element.listExpr();
}

Word Label::Create(const ListExpr typeInfo) {
  return (SetWord( new Label( 0 ) ));
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

*/

class ILabel : public IString {
public:  
  static const string BasicType() { return "ilabel"; }
  static ListExpr IntimeLabelProperty();  
  static bool CheckIntimeLabel( ListExpr type, ListExpr& errorInfo );  
};


/*
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

\subsubsection{function Describing the Signature of the Type Constructor}

*/
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

/*
\subsubsection{Function ~compress~}

If there are subsequent ULabels with the same Label, this function squeezes
them to one ULabel.

*/
void MLabel::compress() {
  MLabel* newML = new MLabel(1);
  ULabel ul(1);
  for (size_t i = 0; i < (size_t)this->GetNoComponents(); i++) {
    this->Get(i, ul);
    newML->MergeAdd(ul);
  }
  *this = *newML;
  delete newML;
}

/*
\subsubsection{Creation of the type constructor ~mlabel~}

*/
TypeConstructor movinglabel(
    MLabel::BasicType(), // name
    MLabel::MLabelProperty,    //property function describing signature
    //Out and In functions
    OutMapping<MString, UString, OutConstTemporalUnit<CcString, OutCcString> >,
    InMapping<MString, UString, InConstTemporalUnit<CcString, InCcString> >,
    0,
    0,  //SaveToList and RestoreFromList functions
    CreateMapping<MString>,
    DeleteMapping<MString>,     //object creation and deletion
    0,
    0,  // object open and save
    CloseMapping<MString>,
    CloneMapping<MString>,  //object close and clone
    CastMapping<MString>,   //cast function
    SizeOfMapping<MString>, //sizeof function
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

//**********************************************************************



//**********************************************************************



//**********************************************************************

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
  os << " State: " << lbs.GetState()
     << "<";
  for(int i = 0; i < lbs.GetNoLabels(); i++)
    os << lbs.GetLabel(i) << " ";
  os << ">";
  return os;
}

/*
2.3.1 Constructors.

This first constructor creates a new polygon.

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
int Labels::Compare(const Attribute*) const
{
  return 0;
}

/*
2.3.6 HashValue

Because Compare returns alway 0, we can only return a constant hash value.

*/
size_t Labels::HashValue() const{
  return  1;
}

/*
2.3.5 Adjacent

Not yet implemented. 

*/
bool Labels::Adjacent(const Attribute*) const
{
  return 0;
}

/*
2.3.7 Clone

Returns a new created element labels (clone) which is a
copy of ~this~.

*/
Labels *Labels::Clone() const
{
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
  assert( state == complete );
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
  Label lb;
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
      element.append(NList((labels->GetLabel(i)).GetText(), true));
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
      labels->Append(Label(list.first().str()));
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


//**********************************************************************
/*
\section{Pattern}
*/

/*
\subsection{Function ~toString~}
Writes all pattern information into a string.

*/
string Pattern::toString() const {
  stringstream str;
  str << "~~~~~~pattern~~~~~~" << endl;
  for (unsigned int i = 0; i < patterns.size(); i++) {
    str << "[" << i << "] " << patterns[i].var << " | "
        << setToString(patterns[i].ivs) << " | "
        << setToString(patterns[i].lbs) << " | "
        << (patterns[i].wc ? (patterns[i].wc == STAR ? "*" : "+") :"") << endl;
  }
  str << "~~~~~~conditions~~~~~~" << endl;
  for (unsigned int i = 0; i < conds.size(); i++) {
    str << "[" << i << "] " << conds[i].text << endl;
    for (unsigned int j = 0; j < conds[i].vars.size(); j++) {
      str << "  [[" << j << "]] " << conds[i].vars[j] << "."
          << conds[i].keys[j] << " in #" << conds[i].pIds[j] << endl;
    }
  }
  str << "~~~~~~results~~~~~~" << endl;
  for (unsigned int i = 0; i < results.size(); i++) {
    str << "[" << i << "] " << results[i].var << " | "
        << setToString(results[i].ivs) << " | " << setToString(results[i].lbs)
        << " | " << (results[i].wc ? (results[i].wc == STAR ? "*" : "+") : "")
        << endl;
  }
  str << "~~~~~~assignments~~~~~~" << endl;
  for (unsigned int i = 0; i < assigns.size(); i++) {
    str << "[" << i << "] " << assigns[i].var << " | ";
    str << setToString(assigns[i].ivs) << " | "
        << setToString(assigns[i].lbs) << " | "
        << (assigns[i].wc ? (assigns[i].wc == STAR ? "*" : "+") : "") << endl;
  }
  str << endl;
  return str.str();
}

/*
\subsection{Function ~GetText()~}

Returns the pattern text as specified by the user.

*/
string Pattern::GetText() const {
  return text;
}

/*
4.2 Function ~BasicType~

*/
const string Pattern::BasicType() {
  return "pattern";
}

/*
4.3 Function ~checkType~

*/
const bool Pattern::checkType(const ListExpr type){
  return listutils::isSymbol(type, BasicType());
}

/*
4.4 Function ~In~

*/
Word Pattern::In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct) {
  Word result = SetWord(Address(0));
  correct = false;
  NList list(instance);
  if (list.isAtom()) {
    if (list.isText()) {
      string text = list.str();
      Pattern *pattern = 0;
      text += "\n";
      pattern = stj::parseString(text.c_str());
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
4.5 Function ~Out~

*/
ListExpr Pattern::Out(ListExpr typeInfo, Word value) {
  Pattern* pattern = static_cast<Pattern*>(value.addr);
  NList element(pattern->GetText(), true, true);
  return element.listExpr();
}

/*
4.6 Function ~Create~

*/
Word Pattern::Create(const ListExpr typeInfo) {
  return (SetWord(new Pattern()));
}

/*
4.7 Function ~Delete~

*/
void Pattern::Delete(const ListExpr typeInfo, Word& w) {
  delete static_cast<Pattern*>(w.addr);
  w.addr = 0;
}

/*
4.8 Function ~Close~

*/
void Pattern::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<Pattern*>(w.addr);
  w.addr = 0;
}

/*
4.9 Function ~Clone~

*/
Word Pattern::Clone(const ListExpr typeInfo, const Word& w) {
  Pattern* pattern = static_cast<Pattern*>(w.addr);
  return SetWord(new Pattern(*pattern));
}

/*
4.10 Function ~SizeOfObj~

*/
int Pattern::SizeOfObj() {
  return sizeof(Pattern);
}

/*
4.11 Function ~KindCheck~

*/
bool Pattern::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Pattern::BasicType()));
}

/*
\subsection{Function ~verifyPattern~}

Loops through the unit patterns and checks whether every specified interval
is valid and whether the variables are unique.

*/
bool Pattern::verifyPattern() {
  set<string>::iterator it;
  SecInterval iv;
  set<string> vars;
  for (unsigned int i = 0; i < patterns.size(); i++) {
    for (it = patterns[i].ivs.begin(); it != patterns[i].ivs.end(); it++) {
      if ((*it).at(0) >= 65 && (*it).at(0) <= 122
        && !checkSemanticDate(*it, iv, false)) {
        return false;
      }
    }
    if (!patterns[i].var.empty()) {
      if (vars.count(patterns[i].var)) {
        cout << "Error: variables in the pattern must be unique." << endl;
        return false;
      }
      else {
        vars.insert(patterns[i].var);
      }
    }
  }
  return true;
}

/*
\subsection{Function ~verifyConditions~}

Loops through the conditions and checks whether each one is a syntactically
correct boolean expression. If there is an invalid condition, the user input
is rejected.

*/
bool Pattern::verifyConditions() {
  for (unsigned int i = 0; i < conds.size(); i++) {
    if (!evaluate(conds[i].textSubst, false)) {
      cout << "condition \'" << conds[i].textSubst << "\' is invalid." << endl;
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~hasResults~}

Returns ~true~ iff the pattern results vector is not empty. This is necessary
for the operator ~rewrite~.

*/
bool Pattern::hasResults() {
  return !results.empty();
}

/*
\subsection{Function ~getPattern~}

Calls the parser.

*/
Pattern* Pattern::getPattern(string input) {
  input.append("\n");
  cout << input << endl;
  const char *patternChar = input.c_str();
  return parseString(patternChar);
}

/*
\subsection{Function ~matches~}

Checks the pattern and the condition and (if no problem occurs) invokes the NFA
construction and the matching procedure.

*/
bool Pattern::matches(MLabel const &ml) {
  if (!verifyPattern() || !verifyConditions()) {
    return false;
  }
  NFA *nfa = new NFA(patterns.size() + 1);
  nfa->buildNFA(*this);
  cout << nfa->toString() << endl;
  bool result = nfa->match(ml, false);
  delete nfa;
  return result;
}

/*
\subsection{Function ~getRewriteSequences~}

Performs a match and returns the set of matching sequences for the operator
~rewrite~.

*/
set<vector<size_t> > Pattern::getRewriteSequences(MLabel const &ml) {
  set<vector<size_t> > result;
  if (!verifyPattern() || !verifyConditions() || !hasResults()) {
    cout << "Error: Invalid pattern." << endl;
    return result;
  }
  NFA *nfa = new NFA(patterns.size() + 1);
  nfa->buildNFA(*this);
  cout << nfa->toString() << endl;
  if (!nfa->match(ml, true)) {
    cout << "Error: Mismatch" << endl;
    return result;
  }
  nfa->computeResultVars(this->results);
  nfa->buildSequences();
  nfa->printSequences(25);
  nfa->filterSequences(ml);
  nfa->printRewriteSequences(25);
  return nfa->getRewriteSequences();
}

/*
\subsection{Function ~getRelevantSequences~}

*/
set<vector<size_t> > NFA::getRewriteSequences() {
  return rewriteSeqs;
}

/*
\subsection{Function ~buildResultVars~}

*/
void NFA::computeResultVars(vector<UnitPattern> results) {
  bool found = false;
  for (unsigned int i = 0; i < results.size(); i++) {
    found = false;
    int j = 0;
    while (!found) {
      if (!results[i].var.compare(patterns[j].var)) {
        resultVars.push_back(j);
        j = 0;
        found = true;
      }
      else {
        j++;
      }
    }
  }
  for (unsigned int i = 0; i < resultVars.size(); i++) {
    cout << " " << resultVars[i];
  }
  cout << endl;
}

/*
\subsection{Function ~filterSequences~}

Searches for sequences which fulfill all conditions and stores parts of them for
rewriting.

*/
void NFA::filterSequences(MLabel const &ml) {
  set<vector<size_t> >::iterator it;
  vector<size_t> rewriteSeq;
  if (!conds.empty()) {
    for (it = sequences.begin(); it != sequences.end(); it++) {
      for (unsigned int i = 0; i < conds.size(); i++) {
        cout << "processing cond #" << i << endl;
        if (!conds[i].keys.empty()) {
          buildCondMatchings(i, *it);
        }
        cout << "matchings built for cond #" << i << endl;
        if (!evaluateCond(ml, i, *it)) {
          cout << "mismatch at #" << i << endl;
          i = conds.size(); // continue with next sequence
        }
        else if (i == conds.size() - 1) { // all conditions are fulfilled
          for (unsigned int j = 0; j < resultVars.size(); j++) {
            rewriteSeq.push_back((*it)[resultVars[j]]); // begin
            if (resultVars[j] < numOfStates - 2) {
              rewriteSeq.push_back((*it)[resultVars[j] + 1] - 1); // end
            }
            else { // last state
              rewriteSeq.push_back(maxLabelId);
            }
          }
          rewriteSeqs.insert(rewriteSeq);
          rewriteSeq.clear();
        }
      }
    }
  }
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
       nl->TextAtom("\' (monday at_home) X (_ _) // X.start = 2011-01-01 \'"),
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

//**********************************************************************
/*
\section{NFA}
*/

/*
\subsection{Function ~buildNFA~}

Reads the pattern and generates the state transitions.

*/
void NFA::buildNFA(Pattern p) {
  patterns = p.patterns;
  conds = p.conds;
  for (int i = 0; i < numOfStates - 1; i++) { // solve epsilon-transitions
    transitions[i][i].insert(i + 1);// state i, read pattern i => new state i+1
    if (patterns[i].wc == STAR) { // reading '*'
      if (i > 0) {
        transitions[i - 1][i - 1].insert(i + 1);
      }
      transitions[i][i].insert(i);
      if (i < numOfStates - 2) {
        transitions[i][i + 1].insert(i + 2);
        int j = i + 1;
        while ((j < numOfStates - 1) && (patterns[j].wc == STAR)) {
          transitions[i][i].insert(j + 1); // handle '* * ... *'
          transitions[i][j + 1].insert(j + 2); // handle '* * * (1 a) ...'
          j++;
        }
        if (patterns[i + 1].wc != STAR) { // handle '* (1909 bvb) * * ... *'
          transitions[i][i + 1].insert(i);
          transitions[i][i + 1].insert(i + 1);
          int j = i + 2;
          while ((j < numOfStates - 1) && (patterns[j].wc == STAR)) {
            transitions[i][i + 1].insert(j + 1);
            j++;
          }
        }
      }
      else {
        transitions[i][i + 1].insert(numOfStates - 1);
      }
    }
    else { // not reading '*'
      int j = i + 1; // handle '(a 1) * * ... *'
      while ((j < numOfStates - 1) && (patterns[j].wc == STAR)) {
        transitions[i][i].insert(j + 1);
        j++;
      }
    }
    if (patterns[i].wc == PLUS) { // reading '+'
      transitions[i][i].insert(i);
    }
//     if (i > 0) {  // remove duplicates for '* *'
//       if ((patterns[i - 1].wc == PLUS) && (patterns[i].wc == PLUS)){
//         transitions[i - 1][i].clear();  
//       }
//     }
  }
}

/*
\subsection{Function ~match~}

Loops through the MLabel calling updateStates() for every ULabel. True is
returned if and only if the final state is an element of currentStates after
the loop. If ~rewrite~ is true (which happens in case of the operator ~rewrite~)
the matching procedure ends after the unit pattern test.

*/
bool NFA::match(MLabel const &ml, bool rewrite) {
  maxLabelId = (size_t)ml.GetNoComponents() - 1;
  for (size_t i = 0; i <= maxLabelId; i++) {
    ml.Get(i, ul);
    ulId = i;
    cout << "unit label #" << i << " is processed now" << endl;
    updateStates();
    if (currentStates.empty()) {
      cout << "no current state" << endl;
      return false;
    }
  }
  if (!currentStates.count(numOfStates - 1)) { // is the final state active?
    return false;
  }
  if (rewrite) {
    return true;
  }
  printCards();
  if (conds.size()) {
    buildSequences();
    printSequences(50);
    if (!conditionsMatch(ml)) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~toString~}

Returns a string displaying the information stored in the NFA.

*/
string NFA::toString() {
  cout << "start output, " << numOfStates << " states" << endl;
  stringstream nfa;
  set<int>::iterator k;
  for (int i = 0; i < numOfStates - 1; i++) {
    for (int j = i; j < numOfStates - 1; j++) {
      if (transitions[i][j].size() > 0) {
        nfa << "state " << i << " | unitpat #" << j << " | new states {";
        if (transitions[i][j].size() == 1) {
          nfa << *(transitions[i][j].begin());
        }
        else {
          for (k = transitions[i][j].begin();
               k != transitions[i][j].end(); k++) {
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
void NFA::printCurrentStates() {
  set<int>::iterator i;
  cout << "the set of active states is {";
  for (i = currentStates.begin(); i != currentStates.end(); i++) {
    cout << *i << " ";
  }
  cout << "}" << endl;
}

/*
\subsection{Function ~printCards~}

Prints the ulabels matched by every unit pattern. Subsequently, the possible
cardinalities for every unit pattern are displayed.

*/
void NFA::printCards() {
  set<size_t>::iterator it;
  for (int j = 0; j < numOfStates - 1; j++) {
    cout << "unit pattern " << j << " matches ulabels ";
    for (it = matchings[j].begin(); it != matchings[j].end(); it++) {
      cout << *it << ", ";
    }
    cout << endl;
  }
  set<int>::iterator j;
  for (int i = 0; i < numOfStates - 1; i++) {
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
void NFA::printSequences(size_t max) {
  set<vector<size_t> >::iterator it;
  unsigned int seqCount = 0;
  it = sequences.begin();
  while ((seqCount < max) && (it != sequences.end())) {
    cout << "seq_" << (seqCount < 9 ? "0" : "") << seqCount  + 1 << " | ";
    for (unsigned int i = 0; i < (*it).size(); i++) {
      cout << (*it)[i] << ", ";
    }
    cout << endl;
    it++;
    seqCount++;
  }
  cout << "there are " << sequences.size() << " possible sequences" << endl;
}

/*
\subsection{Function ~printRewriteSequences~}

Displays the sequences for rewriting. As the number of sequences may be very
high, only the first ~max~ sequences are printed.

*/
void NFA::printRewriteSequences(size_t max) {
  set<vector<size_t> >::iterator it;
  unsigned int seqCount = 0;
  it = rewriteSeqs.begin();
  while ((seqCount < max) && (it != rewriteSeqs.end())) {
    cout << "rseq_" << (seqCount < 9 ? "0" : "") << seqCount  + 1 << " | ";
    for (unsigned int i = 0; i < (*it).size(); i++) {
      cout << (*it)[i] << ", ";
    }
    cout << endl;
    it++;
    seqCount++;
  }
  cout << "there are " << rewriteSeqs.size() << " possible sequences" << endl;
}

/*
\subsection{Function ~printCondMatchings~}

Displays the possible condition matching sequences. As the number of sequences
may be very high, only the first ~max~ sequences are printed.

*/
void NFA::printCondMatchings(size_t max) {
  set<vector<size_t> >::iterator it;
  unsigned int count = 0;
  it = condMatchings.begin();
  while ((count < max) && (it != condMatchings.end())) {
    cout << "cm_" << (count < 9 ? "0" : "") << count  + 1 << " | ";
    for (unsigned int i = 0; i < (*it).size(); i++) {
      cout << (*it)[i] << ", ";
    }
    cout << endl;
    it++;
    count++;
  }
  cout << condMatchings.size() << " possible condition matchings." << endl;
}

/*
\subsection{Function ~updateStates~}

Further functions are invoked to decide which transition can be applied to
which current state. The set of current states is updated.

*/
void NFA::updateStates() {
  set<int> newStates;
  newStates.clear();
  set<int>::iterator i, it;
  for (i = currentStates.begin(); i != currentStates.end(); i++) {
    for (int j = *i; j < numOfStates - 1; j++) {
      if (!transitions[*i][j].empty()) {
        if (labelsMatch(j) && timesMatch(j)) { // insert the new states
          storeMatch(j);
          for (it = transitions[*i][j].begin();
               it != transitions[*i][j].end(); it++) {
            newStates.insert(*it);
          }
        }
      }
    }
  }
  currentStates = newStates;
  printCurrentStates();
}

/*
\subsection{Function ~storeMatch~}

Stores the matching positions (unit pattern and unit label, respectively) and
the possible cardinalities into arrays of sets.

*/
void NFA::storeMatch(int state) { // TODO: shorten this
  size_t fromLabel, toLabel;
  int fromState, toState;
  set<size_t>::iterator it;
  if (!patterns[state].wc || !patterns[state].ivs.empty()
   || !patterns[state].lbs.empty()) { // (1 a) or ((1 a)) or ()
    matchings[state].insert(ulId);
    cardsets[state].insert(1); // cardinality is 1 for a single match
    if ((state > 0) && patterns[state - 1].wc) {
      if (state == 1) { // wildcard at 0, match(es) at 1
        cardsets[0].insert(*(matchings[1].rbegin()));
      }
      else {
        if (matchings[state - 2].empty()) {
          int j = state - 3; // search previous matching position
          while ((j >= 0) && matchings[j].empty()) {
            j--;
          }
          toLabel = *(matchings[state].rbegin()) - *(matchings[j].begin()) - 1;
          fromLabel = 0;
          fromState = j + 1;
          toState = state - 1;
          for (int k = fromState; k <= toState; k++) {
            for (size_t i = fromLabel; i <= toLabel; i++) {
              if ((i > 0) || (patterns[k].wc == STAR)) {
                cardsets[k].insert(i); // do not insert 0 when wildcard is +
              }
            }
          }
        }
        else { // matching in state - 2
          it = matchings[state - 2].begin();
          while ((*it < ulId) && (it != matchings[state - 2].end())) {
            if ((ulId - *it - 1 > 0) || (patterns[state - 1].wc == STAR)) {
              cardsets[state - 1].insert(ulId - *it - 1); // card != 0 for wc +
            }
            it++;
          }
        }
      }
    }
  }
  else if ((state == numOfStates - 2) && patterns[state].wc) {  // last state
    int j = state - 1;
    if ((j >= 0) && matchings[state - 1].empty()) {
      j--; // search previous matching position
      while ((j >= 0) && matchings[j].empty()) {
        j--;
      }
      if ((j < 0) || (j < state - 1)) {
        fromLabel = 0;
        if (j < 0) {
          toLabel = maxLabelId;
        }
        else {
          toLabel = maxLabelId - *(matchings[j].begin());
        }
        fromState = j + 1; // 0 iff no match exists
        for (int k = fromState; k <= state; k++) {
          for (size_t i = fromLabel; i <= toLabel; i++) {
            if ((i > 0) || (patterns[k].wc == STAR)) {
              cardsets[k].insert(i); // do not insert 0 when wildcard is +
            }
          }
        }
      }
    }
    else if (j >= 0) { // matching in state - 1
      for (it = matchings[j].begin(); it != matchings[j].end(); it++) {
        cardsets[state].insert(maxLabelId - *it);
      }
    }
    else { // pattern consists of one unit which is a wildcard
      cardsets[state].insert(maxLabelId + 1);
    }
  }
}

/*
\subsection{Function ~timesMatch~}

Checks whether the time interval of the current unit label is completely
enclosed by every interval of the unit pattern at position pos. If no pattern
interval is specified, the result is true.

*/
bool NFA::timesMatch(int pos) {
  bool result(true), elementOk(false);
  set<int>::iterator i;
  set<string>::iterator j;
  string varKey, currentLabelString;
  Instant *pStart = new DateTime(instanttype);
  Instant *pEnd = new DateTime(instanttype);
  SecInterval *pIv = new SecInterval(0);
  SecInterval *uIv = new SecInterval(ul.timeInterval);
  if (!patterns[pos].ivs.empty()) {
    for (j = patterns[pos].ivs.begin(); j != patterns[pos].ivs.end(); j++) {
      if (((*j)[0] > 96) && ((*j)[0] < 123)) { // 1st case: semantic date/time
        elementOk = checkSemanticDate(*j, *uIv, true);
      }
      else if (((*j).find('-') == string::npos) // 2nd case: 19:09~22:00
            && (((*j).find(':') < (*j).find('~')) // on each side of [~],
                || ((*j)[0] == '~')) // there has to be either xx:yy or nothing
            && (((*j).find(':', (*j).find('~')) != string::npos)
                || (*j)[(*j).size() - 1] == '~')) {
        elementOk = checkDaytime(*j, *uIv);
      }
      else {
        if ((*j)[0] == '~') { // 3rd case: ~2012-05-12
          pStart->ToMinimum();
          pEnd->ReadFrom(extendDate((*j).substr(1), false));
        }
        else if ((*j)[(*j).size() - 1] == '~') { // 4th case: 2011-04-02-19:09~
          pStart->ReadFrom(extendDate((*j).substr(0, (*j).size() - 1), true));
          pEnd->ToMaximum();
        }
        else if (((*j).find('~')) == string::npos) { // 5th case: no [~] found
          pStart->ReadFrom(extendDate(*j, true));
          pEnd->ReadFrom(extendDate(*j, false));
        }
        else { // sixth case: 2012-05-12-20:00~2012-05-12-22:00
          pStart->ReadFrom(extendDate((*j).substr(0, (*j).find('~')), true));
          pEnd->ReadFrom(extendDate((*j).substr((*j).find('~') + 1), false));
        }
        pIv->Set(*pStart, *pEnd, true, true);
        elementOk = pIv->Contains(*uIv);
      }
      if (!elementOk) { // all intervals have to match
        result = false;
      }
    }
  }
  uIv->DeleteIfAllowed();
  pIv->DeleteIfAllowed();
  pStart->DeleteIfAllowed();
  pEnd->DeleteIfAllowed();
  return result;
}

/*
\subsection{Function ~labelsMatch~}

Checks whether the label of the current ULabel matches one of the unit pattern
labels at position pos. If no label is specified in the pattern, ~true~ is
returned.

*/
bool NFA::labelsMatch(int pos) {
  bool result = true;
  set<string>::iterator i;
  string currentLabelString;
  if (!patterns[pos].lbs.empty()) {
    result = false;
    for (i = patterns[pos].lbs.begin(); i != patterns[pos].lbs.end(); i++) {
      CcString *label = new CcString(true, *i);
      if (ul.Passes(*label)) { // look for a matching label
        result = true;
      }
      label->DeleteIfAllowed();
    }
  }
  return result;
}

/*
\subsection{Function ~buildSequences~}

Derives all possible ULabel sequences from the cardinality candidates. Only
sequences with length maxLabelId + 1 are accepted.

*/
void NFA::buildSequences() {
  vector<size_t> sequence;
  set<size_t>::iterator it;
  size_t totalSize = 1;
  for (int i = 0; i < numOfStates - 1; i++) {
    totalSize *= cardsets[i].size();
  }
  for (size_t j = 0; j < totalSize; j++) {
    size_t k = j;
    sequence.clear();
    sequence.push_back(0);
    size_t sequenceSum = 0;
    for (int state = 0; state < numOfStates - 1; state++) {
      it = cardsets[state].begin();
      advance(it, k % cardsets[state].size());
      if (state < numOfStates - 2) {
        sequence.push_back(*it + *sequence.rbegin());
      }
      k /= cardsets[state].size();
      sequenceSum += *it;
      if (sequenceSum > maxLabelId + 1) { // stop if sum exceeds maximum
        state = numOfStates;
      }
    }
    if (sequenceSum == maxLabelId + 1) {
      sequences.insert(sequence);
    }
  }
}

/*
\subsection{Function ~conditionsMatch~}

Checks whether the specified conditions are fulfilled. The result is true if
and only if there is (at least) one cardinality sequence that matches every
condition.

*/
bool NFA::conditionsMatch(MLabel const &ml) {
  bool goToNextCond(false);
  set<vector<size_t> >::iterator it;
  if (conds.empty()) {
    return true;
  }
  for (unsigned int i = 0; i < conds.size(); i++) {
    it = sequences.begin();
    goToNextCond = false;
    while ((it != sequences.end()) && !goToNextCond) {
      goToNextCond = false;
      if (!conds[i].keys.empty()) {
        buildCondMatchings(i, *it);
      }
      if (!evaluateCond(ml, i, *it)) {
        sequences.erase(it);
        it = sequences.begin();
        i = 0; // in case of a mismatch, go back to the first condition
      }
      else {
        goToNextCond = true;
      }
    }
    if (!goToNextCond) { // no matching sequence found
      cout << "mismatch in condition " << i << endl;
      return false;
    }
    cout << sequences.size() << " sequences remain after cond #" << i << endl;
  }
  cout << "Matching ok, " << sequences.size() << " remaining." << endl;
  return !sequences.empty();
}

/*
\subsection{Function ~buildCondMatchings~}

For one condition and one cardinality sequence, a set of possible matching
sequences is built if necessary (that is, if the condition contains a label).

*/
void NFA::buildCondMatchings(unsigned int condId, vector<size_t> sequence) {
  bool necessary(false);
  condMatchings.clear();
  int pId;
  size_t totalSize = 1;
  vector<size_t> condMatching;
  set<int> consideredIds;
  set<int>::iterator it;
  size_t size;
  sequence.push_back(maxLabelId + 1); // easier for last pattern
  for (unsigned int i = 0; i < conds[condId].keys.size(); i++) {
    pId = conds[condId].pIds[i];
    if (!conds[condId].keys[i] && !consideredIds.count(pId)) { // no doubles
      totalSize *= sequence[pId + 1] - sequence[pId];
      consideredIds.insert(pId);
      necessary = true;
    }
  }
  if (!necessary) {
    totalSize = 0;
  }
  for (size_t j = 0; j < totalSize; j++) {
    size_t k = j;
    condMatching.clear();
    for (it = consideredIds.begin(); it != consideredIds.end(); it++) {
      size = sequence[*it + 1] - sequence[*it];
      condMatching.push_back(sequence[*it] + k % size);
      k /= size;
    }
    condMatchings.insert(condMatching);
  }
  if (totalSize) {
    printCondMatchings(50);
  }
}

/*
\subsection{Function ~evaluateCond~}

This function is invoked by ~conditionsMatch~ and checks whether a sequence of
possible cardinalities matches a certain condition.

*/
bool NFA::evaluateCond(MLabel const &ml, unsigned int condId,
                       vector<size_t> sequence) {
  conds[condId].textSubst.assign(conds[condId].text); // reset textSubst
  bool success(false), replaced(false);
  string condStrCardTime, subst;
  for (unsigned int j = 0; j < conds[condId].keys.size(); j++) {
    int pId = conds[condId].pIds[j];
    if (conds[condId].keys[j] == 4) { // card
      if (pId == numOfStates - 2) {
        subst.assign(int2Str(maxLabelId - sequence[pId] + 1));
      }
      else {
        subst.assign(int2Str(sequence[pId + 1] - sequence[pId]));
      }
    }
    else if (conds[condId].keys[j] > 0) { // time, start, end
      size_t from = sequence[pId];
      size_t to = (pId == numOfStates - 2 ? maxLabelId : sequence[pId + 1]);
      subst.assign(getTimeSubst(ml, conds[condId].keys[j], from, to));
    }
    if (conds[condId].keys[j] > 0) { // time, start, end, card
      conds[condId].substitute(j, subst);
      if (conds[condId].textSubst.compare("error")) {
        replaced = true;
      }
      else {
        return false;
      }
    }
  } // cardinality and time substitutions completed
  condStrCardTime.assign(conds[condId].textSubst); // save status
  if (condMatchings.empty() && replaced) {
    if (conds[condId].falseExprs.count(condStrCardTime)) {
      return false; // if condStrCardTime is already known as false
    }
    else if (conds[condId].trueExprs.count(condStrCardTime)) {
      return true;
    }
    else {
      if (!evaluate(condStrCardTime, true)) {
        cout << "cardinality & time evaluation negative" << endl;
        conds[condId].falseExprs.insert(condStrCardTime);
        return false;
      }
      else {
        conds[condId].trueExprs.insert(condStrCardTime);
        return true;
      }
    }
  }
  while (!condMatchings.empty()) {
    conds[condId].textSubst.assign(condStrCardTime);
    int pos = 0;
    for (unsigned int j = 0; j < conds[condId].keys.size(); j++) {
      if (!conds[condId].keys[j]) { // label
        subst.assign(getLabelSubst(ml, pos));
        conds[condId].substitute(j, subst);
        if (conds[condId].textSubst.compare("error")) {
          replaced = true;
        }
        else {
          return false;
        }
        pos++;
      }
    }
    if (conds[condId].falseExprs.count(conds[condId].textSubst)) {
      return false;
    }
    else if (conds[condId].trueExprs.count(conds[condId].textSubst)) {
      return true;
    }
    else {
      if (!evaluate(conds[condId].textSubst, true)) {
        cout << "evaluation negative" << endl;
        conds[condId].falseExprs.insert(conds[condId].textSubst);
        return false; // one false evaluation is enough to yield ~false~
      }
      else {
        conds[condId].trueExprs.insert(conds[condId].textSubst);
        success = true;
      }
    }
  }
  return success;
}

/*
\subsection{Function ~substitute~}

Searches a string like ~X.label~ in the ~textSubst~ of a condition and replaces
it by the parameter ~subst~.

*/
void Condition::substitute(unsigned int pos, string subst){
  string varKey(vars[pos]);
  varKey.append(types[keys[pos]]);
  size_t varKeyPos = textSubst.find(varKey);
  if (varKeyPos == string::npos) {
    cout << "var.key " << varKey << " not found in " << textSubst << endl;
    textSubst.assign("error");
  }
  else {
    textSubst.replace(varKeyPos, varKey.size(), subst);
  }
}

/*
\subsection{Function ~getTimeSubst~}

Depending on the key (~time~, ~start~, or ~end~ are possible here) and on the
limits ~from~ and ~to~, the respective time information is retrieved from the
MLabel and returned as a string.

*/
string NFA::getTimeSubst(MLabel const &ml, Key key, size_t from, size_t to) {
  stringstream result;  
  if ((from < 0) || (to > maxLabelId)) {
    cout << "ULabel #" << (from < 0 ? from : to) << " does not exist." << endl;
    return "error";
  }
  Periods timesML(0); // TODO store this in the class?
  SecInterval uIv;
  switch (key) {
    case 1: // time
      for (size_t i = from; i <= to; i++) { // build periods
        ml.Get(i, ul);
        timesML.Add(ul.timeInterval);
      }
      result << "[const periods value("; // build string
      for (size_t j = 0; j < (size_t)timesML.GetNoComponents(); j++) {
        timesML.Get(j, uIv);
        result << "(\"" << uIv.start.ToString() << "\" \"" << uIv.end.ToString()
               << "\" " << (uIv.lc ? "TRUE " : "FALSE ")
               << (uIv.rc ? "TRUE" : "FALSE") << ")"
               << (j == (size_t)timesML.GetNoComponents() - 1 ? "" : " ");
      }
      result << ")]";
      break;
    case 2: // start
      ml.Get(from, ul);
      uIv = ul.timeInterval;
      result << "[const instant value \"" << uIv.start.ToString() << "\"]";
      break;
    case 3: // end
      ml.Get(to, ul);
      uIv = ul.timeInterval;
      result << "[const instant value \"" << uIv.end.ToString() << "\"]";
      break;
    default: // should not occur
      result << "error";
  }
  return result.str();
}

/*
\subsection{Function ~getLabelSubst~}

The label substitution is found and returned.

*/
string NFA::getLabelSubst(MLabel const &ml, unsigned int pos) {
  stringstream result;
  set<vector<size_t> >::iterator it = condMatchings.begin();
  ml.Get((*it)[pos], ul);
  if ((*it)[pos] > maxLabelId) {
    cout << "PROBLEM: " << (*it)[pos] << " > " << maxLabelId << endl;
    return "error";
  }
  if (pos == (*it).size() - 1) {
    condMatchings.erase(it);
  }
  result << "\"" << ul.constValue.GetValue() << "\"";
  return result.str();
}

/*
\subsection{Type Mapping for operator ~stjpattern~}

*/
ListExpr textToPatternMap(ListExpr args) {
  NList type(args);
  if (type.first() == NList(FText::BasicType())) {
    return NList(Pattern::BasicType()).listExpr();
  }
  return NList::typeError("Expecting a text!");
}

/*
\subsection{Value Mapping for operator ~stjpattern~}

*/
int patternFun(Word* args, Word& result, int message, Word& local, Supplier s) {
  FText* patternText = static_cast<FText*>(args[0].addr);
  result = qp->ResultStorage(s);
  Pattern* pattern = static_cast<Pattern*>(result.addr);
  cout << "parse with new pattern" << endl;
  Pattern* p = 0;
  if (patternText->IsDefined()) {
    p = Pattern::getPattern(patternText->toText());
  }
  if (p) {
    (*pattern) = (*p);
    delete p;
    // TODO store pattern into database
  }
  else {
    //ppp->SetDefined(false);
    cout << "failed" << endl;
  }
  return 0;
}

struct patternInfo : OperatorInfo {
  patternInfo() {
    name      = "stjpattern";
    signature = " Text -> " + Pattern::BasicType();
    syntax    = "_ stjpattern";
    meaning   = "Creates a Pattern from a Text.";
  }
};

/*
\subsection{Type Mapping for operator ~matches~}

*/
ListExpr matchesTypeMap(ListExpr args) {
  NList type(args);
  const string errMsg = "Expecting a mlabel and a text or a mstring and a text";
  if ((type == NList(MLabel::BasicType(), Pattern::BasicType()))
   || (type == NList(MLabel::BasicType(), FText::BasicType()))
   || (type == NList(MString::BasicType(), Pattern::BasicType()))
   || (type == NList(MString::BasicType(), FText::BasicType()))) {
    return NList(CcBool::BasicType()).listExpr();
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Selection Function for operator ~rewrite~}

*/
int matchesSelect(ListExpr args) {
  NList type(args);
  return (type.second().isSymbol(Pattern::BasicType())) ? 1 : 0;
}

/*
\subsection{Value Mapping for operator ~matches~}

*/
int matchesFun_MP (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
//   MLabel* mlabel = static_cast<MLabel*>(args[0].addr);
//   Pattern* pattern = static_cast<Pattern*>(args[1].addr);
//   result = qp->ResultStorage(s);
//   CcBool* b = static_cast<CcBool*>(result.addr);
//   bool res = (pattern->TotalMatch(*mlabel));
//   b->Set(true, res);
  return 0;
}

/*
\subsection{Value Mapping for operator ~matches~}

*/
int matchesFun_MT (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  MLabel* mlabel = static_cast<MLabel*>(args[0].addr);
  FText* patternText = static_cast<FText*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* b = static_cast<CcBool*>(result.addr);
  Pattern *pattern = 0;
  if (patternText->IsDefined()) {
    pattern = Pattern::getPattern(patternText->toText());
  }
  if (!pattern) {
    b->SetDefined(false);
  }
  else {
    bool res = pattern->matches(*mlabel);
    delete pattern;
    b->Set(true, res);
  }
  return 0;
}

/*
\subsection{Operator Info for operator ~matches~}

*/
struct matchesInfo : OperatorInfo {
  matchesInfo() {
    name      = "matches";
    signature = MLabel::BasicType() + " x " + Pattern::BasicType() + " -> "
                                    + CcBool::BasicType();
    // overloaded operator => alternative signature appended
    appendSignature(MLabel::BasicType() + " x Text -> " + CcBool::BasicType());
    appendSignature(MString::BasicType() +" x " + Pattern::BasicType() + " -> "
                                         + CcBool::BasicType());
    appendSignature(MString::BasicType() + " x Text -> " + CcBool::BasicType());
    syntax    = "_ matches _";
    meaning   = "Match predicate.";
  }
};

/*
\subsection{Type Mapping for operator ~rewrite~}

*/
ListExpr rewriteTypeMap(ListExpr args) {
  NList type(args);
  const string errMsg = "Expecting a mlabel and a text or a mstring and a text";
  if ((type == NList(MLabel::BasicType(), Pattern::BasicType()))
   || (type == NList(MLabel::BasicType(), FText::BasicType()))
   || (type == NList(MString::BasicType(), Pattern::BasicType()))
   || (type == NList(MString::BasicType(), FText::BasicType()))) {
    return nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                           nl->SymbolAtom(MLabel::BasicType()));
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Selection Function for operator ~rewrite~}

*/
int rewriteSelect(ListExpr args) {
  NList type(args);
  return type.second().isSymbol(Pattern::BasicType()) ? 1 : 0;
}

/*
\subsection{Value Mapping for operator ~rewrite~, type ~text~}

*/
int rewriteFun_MT(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  MLabel* mlabel = 0;
  MLabel* ml = 0;
  FText* patternText = 0;
  Pattern *pattern = 0;
  RewriteResult *rr = 0;
  switch (message) {
    case OPEN: // initialize the local storage
      mlabel = static_cast<MLabel*>(args[0].addr);
      patternText = static_cast<FText*>(args[1].addr);
      if (!patternText->IsDefined()) {
        cout << "Error: pattern cannot be read." << endl;
        return 0;
      }
      pattern = Pattern::getPattern(patternText->toText());
      cout << ".................::::::::::::::!!!!!!!!!!!!! input read" << endl;
      if (!mlabel->IsDefined()) {
        cout << "Error: undefined MLabel." << endl;
      }
      mlabel->compress();
      cout << "mlabel compressed" << endl;
      rr = new RewriteResult();
      // TODO: match, assign set of all matching sequences to rr->sequences
      rr->sequences = pattern->getRewriteSequences(*mlabel);
      rr->it = rr->sequences.begin();
      local.addr = rr;
      return 0;
    case REQUEST: // return the next stream element
      if (!local.addr) {
        result.addr = 0;
        return CANCEL;
      }
      rr = ((RewriteResult*)local.addr);
      if (rr->it == rr->sequences.end()) {
        result.addr = 0;
        cout << "all sequences processed" << endl;
        return CANCEL;
      }
      // TODO: ml <- build moving label from sequence
      // TODO: move iterator to next sequence. If finished, return CANCEL
      ml = new MLabel(1);
      result.addr = ml;
      rr->it++; 
      return YIELD;
    case CLOSE: // free the local storage
      if (local.addr) {
        rr = ((RewriteResult*)local.addr);
        delete rr;
      }
      return 0;
    default: // should never happen
      return -1;
  }
}

/*
\subsection{Value Mapping for operator ~rewrite~, type ~text~}

*/
int rewriteFun_MP(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  return 0;
}
/*
\subsection{Operator Info for operator ~rewrite~}

*/
struct rewriteInfo : OperatorInfo {
  rewriteInfo() {
    name      = "rewrite";
    signature = MLabel::BasicType() + " x " + Pattern::BasicType() + " -> "
                                    + CcBool::BasicType();
    // overloaded operator => alternative signature appended
    appendSignature(MLabel::BasicType() + " x Text -> " + CcBool::BasicType());
    appendSignature(MString::BasicType() +" x " + Pattern::BasicType() + " -> "
                                         + CcBool::BasicType());
    appendSignature(MString::BasicType() + " x Text -> " + CcBool::BasicType());
    syntax    = "_ rewrite _";
    meaning   = "Rewrite a mlabel.";
  }
};


//***********************************************************************//

  
class SymbolicTrajectoryAlgebra : public Algebra {
  public:
    SymbolicTrajectoryAlgebra() : Algebra() {

      AddTypeConstructor(&labelTC);
      AddTypeConstructor(&intimelabel);
      AddTypeConstructor(&unitlabel);
      AddTypeConstructor(&movinglabel);

      movinglabel.AssociateKind(Kind::TEMPORAL());
      movinglabel.AssociateKind(Kind::DATA());

      AddTypeConstructor(&labelsTC);
      AddTypeConstructor(&patternTC);

      AddOperator(&temporalatinstantext);
      AddOperator(patternInfo(), patternFun, textToPatternMap);

      ValueMapping matchesFuns[] = {matchesFun_MT, matchesFun_MP, 0};
      AddOperator(matchesInfo(), matchesFuns, matchesSelect, matchesTypeMap);
      ValueMapping rewriteFuns[] = {rewriteFun_MT, rewriteFun_MP, 0};
      AddOperator(rewriteInfo(), rewriteFuns, rewriteSelect, rewriteTypeMap);

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
