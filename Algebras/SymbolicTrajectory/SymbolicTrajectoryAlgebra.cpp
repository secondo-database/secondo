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
  if (newML->GetNoComponents() < this->GetNoComponents()) {
    cout << "MLabel was compressed from " << this->GetNoComponents() << " to "
         << newML->GetNoComponents() << " components." << endl;
  }
  else {
    cout << "MLabel could not be compressed, still has "
         << newML->GetNoComponents() << " components." << endl;
  }
  return newML;
}

/*
\subsubsection{Function ~createml~}

Creates an MLabel of a certain size for testing purposes. The labels will be
contain only numbers between 1 and size[*]rate; rate being the number of
different labels divided by the size.

*/
void MLabel::create(int size, double rate) {
  if ((size > 0) && (rate > 0) && (rate <= 1)) {
    int max = size * rate;
    MLabel* newML = new MLabel(1);
    ULabel ul(1);
    Instant* end = new Instant();
    Instant* start = new Instant();
    DateTime* minute = new DateTime(0, 60000, durationtype); // duration
    end->Now();
    end->Add(minute);
    start->Now();
    SecInterval* iv = new SecInterval(*start, *end, true, false);
    for (int i = 0; i < size; i++) {
      ul.constValue.Set(true, int2String(max - (i % max)));
      start->Minus(minute);
      end->Minus(minute);
      iv->Set(*start, *end, true, false);
      ul.timeInterval = *iv;
      newML->MergeAdd(ul);
    }
    *this = *newML;
    delete minute;
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
\subsubsection{Function ~rewrite~}

Rewrites a moving label using another moving label and a vector.

*/
void MLabel::rewrite(MLabel const &ml, vector<size_t> seq,
                     vector<UPat> assigns) {
  ULabel ul(1);
  for (int i = 0; i < (int)seq.size(); i++) {
    if ((seq[i] < 0) || (seq[i] > (size_t)ml.GetNoComponents())
     || (seq[i] > seq[i + 1])) {
       cout << "Error: " << seq[i] << ", " << seq[i + 1] << endl;
    }
    else {
      for (size_t j = seq[i]; j < seq[i + 1]; j++) {
        ml.Get(j, ul);
        if (!assigns[i / 2].getL().empty()) {
          ul.constValue.Set(true, (*(assigns[i / 2].getL().begin())));
        }
        if (!assigns[i / 2].getI().empty() && (seq[i] + 1 == seq[i + 1])) {
          Instant *newStart = new DateTime(instanttype);
          Instant *newEnd = new DateTime(instanttype);
          SecInterval *newIv = new SecInterval(0);
          string iv = *(assigns[i / 2].getI().begin());
          if (iv.at(0) == '~') { // case ~2012-05-12
            *newStart = ul.timeInterval.start;
            newEnd->ReadFrom(extendDate(iv.substr(1), false));
          }
          else if (iv[iv.size() - 1] == '~') { //case 2011-04-02-19:09~
            newStart->ReadFrom(extendDate(iv.substr(0, iv.size() - 1), true));
            *newEnd = ul.timeInterval.end;
          }
          else if ((iv.find('~')) == string::npos) { // no [~] found
            newStart->ReadFrom(extendDate(iv, true));
            newEnd->ReadFrom(extendDate(iv, false));
          }
          else { // 2012-05-12-20:00~2012-05-12-22:00
            newStart->ReadFrom(extendDate(iv.substr(0, iv.find('~')), true));
            newEnd->ReadFrom(extendDate(iv.substr(iv.find('~') + 1), false));
          }
          newIv->Set(*newStart, *newEnd, true, false);
          if (newIv->IsValid()) {
            ul.timeInterval = *newIv;
          }
          else {
            cout << "Error: new interval invalid." << endl;
          }
          delete newIv;
          delete newStart;
          delete newEnd;
        }
        this->MergeAdd(ul);
      }
    }
    i++;
  }
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
  for (int i = 0; i < (int)patterns.size(); i++) {
    str << "[" << i << "] " << patterns[i].getV() << " | "
        << setToString(patterns[i].getI()) << " | "
        << setToString(patterns[i].getL()) << " | "
        << (patterns[i].getW() ? (patterns[i].getW() == STAR ? "*" : "+") :"")
        << endl;
  }
  str << "~~~~~~conditions~~~~~~" << endl;
  for (int i = 0; i < (int)conds.size(); i++) {
    str << "[" << i << "] " << conds[i].toString() << endl;
  }
  str << "~~~~~~results~~~~~~" << endl;
  for (int i = 0; i < (int)results.size(); i++) {
    str << "[" << i << "] " << results[i].getV() << " | "
        << setToString(results[i].getI()) << " | "
        << setToString(results[i].getL()) << " | "
        << (results[i].getW() ? (results[i].getW() == STAR ? "*" : "+") : "")
        << endl;
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
  set<string> vars, ivs;
  if (!this) {
    cout << "Error: Pattern not initialized." << endl;
    return false;
  }
  for (int i = 0; i < (int)patterns.size(); i++) {
    ivs = patterns[i].getI();
    for (it = ivs.begin(); it != ivs.end(); it++){
      if ((*it).at(0) >= 65 && (*it).at(0) <= 122
        && !checkSemanticDate(*it, iv, false)) {
        return false;
      }
    }
    if (!patterns[i].getV().empty()) {
      if (vars.count(patterns[i].getV())) {
        cout << "Error: variables in the pattern must be unique." << endl;
        return false;
      }
      else {
        vars.insert(patterns[i].getV());
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
  for (int i = 0; i < (int)conds.size(); i++) {
    if (!evaluate(conds[i].getSubst(), false)) {
      cout << "condition \'" << conds[i].getSubst() << "\' is invalid." << endl;
      return false;
    }
  }
  return true;
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
//   cout << nfa->toString() << endl;
  bool result = nfa->matches(ml, false);
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
//   cout << nfa->toString() << endl;
  if (!nfa->matches(ml, true)) {
    cout << "Error: Mismatch" << endl;
    delete nfa;
    return result;
  }
  nfa->computeResultVars(this->results);
  nfa->buildSequences();
//   nfa->printSequences(300);
  nfa->filterSequences(ml);
//   nfa->printRewriteSequences(50);
  result = nfa->getRewriteSequences();
  delete nfa;
  return result;
}

/*
\subsection{Function ~getRewriteSequences~}

*/
set<vector<size_t> > NFA::getRewriteSequences() {
  return rewriteSeqs;
}

/*
\subsection{Function ~buildResultVars~}

Computes a vector containing the positions of the unit patterns which the
result variables belong to.

*/
void NFA::computeResultVars(vector<UPat> results) {
  bool found = false;
  for (int i = 0; i < (int)results.size(); i++) {
    found = false;
    int j = 0;
    while (!found) {
      if (!results[i].getV().compare(patterns[j].getV())) {
        resultVars.push_back(j);
        j = 0;
        found = true;
      }
      else {
        j++;
      }
    }
  }
}

/*
\subsection{Function ~filterSequences~}

Searches for sequences which fulfill all conditions and stores parts of them for
rewriting.

*/
void NFA::filterSequences(MLabel const &ml) {
  set<multiset<size_t> >::iterator it;
  for (it = sequences.begin(); it != sequences.end(); it++) {
    for (int i = 0; i < (int)conds.size(); i++) {
//       cout << "processing cond #" << i << endl;
      if (conds[i].getKeysSize()) {
        buildCondMatchings(i, *it);
      }
      if (!evaluateCond(ml, i, *it)) {
//         cout << "mismatch at #" << i << endl;
        i = conds.size(); // continue with next sequence
      }
      else if (i == (int)conds.size() - 1) { // all conditions are fulfilled
        buildRewriteSequence(*it);
      }
    }
    if (conds.empty()) {
      buildRewriteSequence(*it);
    }
  }
}

void NFA::buildRewriteSequence(multiset<size_t> sequence) {
  vector<size_t> seq(sequence.begin(), sequence.end());
  vector<size_t> rewriteSeq;
  for (int j = 0; j < (int)resultVars.size(); j++) {
    rewriteSeq.push_back(seq[resultVars[j]]); // begin
    if (resultVars[j] < f - 1) {
      rewriteSeq.push_back(seq[resultVars[j] + 1]); // end
    }
    else { // last state
      rewriteSeq.push_back(numOfLabels);
    }
  }
  rewriteSeqs.insert(rewriteSeq);
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

Reads the pattern and generates the delta function.

*/
void NFA::buildNFA(Pattern p) {
  copyFromPattern(p);
  int prev[3] = {-1, -1, -1}; // prevStar, prevNotStar, secondPrevNotStar
  for (int i = 0; i < f; i++) {
    delta[i][i].insert(i + 1); // state i, read pattern i => new state i+1
    if (!patterns[i].getW() || !patterns[i].getI().empty() // last pattern or
     || !patterns[i].getL().empty() || (i == f - 1)) { // any pattern except +,*
      if ((prev[0] == i - 1) || (i == f - 1)) { // '...* #(1 a)...'
        for (int j = prev[1] + 1; j < i; j++) {
          delta[j][i].insert(i + 1); // '* * * #(1 a ) ...'
          for (int k = j; k <= i; k++) {
            delta[j][k].insert(j);
            for (int m = j; m <= i; m++) {
              delta[j][k].insert(m); // step 1
            }
            if ((patterns[i].getW() == STAR) && (i == f - 1)) { // end
              delta[j][k].insert(f);
            }
          }
        } 
        if (prev[1] > -1) { // match before current pattern
          for (int j = prev[1] + 1; j <= i; j++) {
            delta[prev[1]][prev[1]].insert(j); // step 2
          }
          if ((patterns[i].getW() == STAR) && (i == f - 1)) { // end
            delta[prev[1]][prev[1]].insert(f);
          }
        }
        if (prev[2] < prev[1] - 1) { // '* ... * (1 a) * ... * #(2 b) ...'
          for (int j = prev[2] + 1; j < prev[1]; j++){
            for (int k = prev[1] + 1; k <= i; k++) {
              delta[j][prev[1]].insert(k); // step 3
            }
            if ((patterns[i].getW() == STAR) && (i == f - 1)) { // end
              delta[j][prev[1]].insert(f);
            }
          }
        }
      }
      if (patterns[i].getW() == PLUS) {
        delta[i][i].insert(i);
      }
      prev[2] = prev[1];
      prev[1] = i;
    }
    else if (patterns[i].getW() == STAR) { // reading '*'
      prev[0] = i;
    }
    else if (patterns[i].getW() == PLUS) { // reading '+'
      delta[i][i].insert(i);
      prev[2] = prev[1];
      prev[1] = i;
    }
  }
  if (patterns[f - 1].getW()) { // '... #*' or '... #+'
    delta[f - 1][f - 1].insert(f - 1);
  }
}

/*
\subsection{Function ~match~}

Loops through the MLabel calling updateStates() for every ULabel. True is
returned if and only if the final state is an element of currentStates after
the loop. If ~rewrite~ is true (which happens in case of the operator ~rewrite~)
the matching procedure ends after the unit pattern test.

*/
bool NFA::matches(MLabel const &ml, bool rewrite) {
  numOfLabels = (size_t)ml.GetNoComponents();
  for (size_t i = 0; i < numOfLabels; i++) {
    ml.Get(i, ul);
    ulId = i;
    updateStates();
    if (currentStates.empty()) {
      cout << "no current state" << endl;
      return false;
    }
  }
  if (!numOfLabels) { // empty MLabel
    int pos = 0;
    while (patterns[pos].getW() == STAR) {
      currentStates.insert(pos + 1);
      pos++;
    }
  }
  if (!currentStates.count(f)) { // is the final state active?
    return false;
  }
  if (rewrite) {
    if (!numOfLabels) {
      cout << "no rewriting for an empty MLabel." << endl;
      return false;
    }
    computeCardsets();
//     printCards();
    return true;
  }
  if (conds.size()) {
    computeCardsets();
//     printCards();
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
  stringstream nfa;
  set<int>::iterator k;
  for (int i = 0; i < f; i++) {
    for (int j = i; j < f; j++) {
      if (delta[i][j].size() > 0) {
        nfa << "state " << i << " | upat #" << j << " | new states {";
        if (delta[i][j].size() == 1) {
          nfa << *(delta[i][j].begin());
        }
        else {
          for (k = delta[i][j].begin();
               k != delta[i][j].end(); k++) {
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
void NFA::printCards() {
  set<size_t>::iterator it;
  for (int j = 0; j < f; j++) {
    cout << "upat " << j << " matches ulabels ";
    for (it = match[j].begin(); it != match[j].end(); it++) {
      cout << *it << ", ";
    }
    cout << endl;
  }
  set<int>::iterator j;
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
void NFA::printSequences(size_t max) {
  set<multiset<size_t> >::iterator it1;
  set<size_t>::iterator it2;
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
\subsection{Function ~printRewriteSequences~}

Displays the sequences for rewriting. As the number of sequences may be very
high, only the first ~max~ sequences are printed.

*/
void NFA::printRewriteSequences(size_t max) {
  set<vector<size_t> >::iterator it;
  unsigned int seqCount = 0;
  it = rewriteSeqs.begin();
  while ((seqCount < max) && (it != rewriteSeqs.end())) {
    cout << "result_" << (seqCount < 9 ? "0" : "") << seqCount  + 1 << " | ";
    for (unsigned int i = 0; i < (*it).size(); i++) {
      cout << (*it)[i] << ", ";
    }
    cout << endl;
    it++;
    seqCount++;
  }
  cout << "there are " << rewriteSeqs.size() << " result sequences" << endl;
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
  set<int>::iterator i, k;
  map<int, set<int> >::iterator j;
  for (i = currentStates.begin(); (i != currentStates.end() && *i < f); i++) {
    for (j = delta[*i].begin(); j != delta[*i].end(); j++) {
      if (labelsMatch(j->first) && timesMatch(j->first)) {
        if (!patterns[j->first].getW() || !patterns[j->first].getI().empty()
         || !patterns[j->first].getL().empty()) {//(_ a), ((1 _)), () or similar
          match[j->first].insert(ulId);
        }
        newStates.insert(j->second.begin(), j->second.end());
      }
    }
  }
  currentStates = newStates;
//   printCurrentStates();
}

/*
\subsection{Function ~processDoublePars~}

Computes the set of possible cardinalities for sequence patterns in double
parentheses and stores their positions into a set.

*/
void NFA::processDoublePars(int pos) {
  set<size_t>::iterator j;
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
  if (!patterns[pos].getI().empty() || !patterns[pos].getL().empty()) {
    doublePars.insert(pos);
  }
}

/*
\subsection{Function ~computeCardsets~}

Computes the set of possible cardinalities for every state.

*/
void NFA::computeCardsets() {
  set<size_t>::iterator j, k;
  int prev = -1; // previous matching position
  int numOfNonStars = 0;
  size_t limit;
  for (int i = 0; i < f; i++) {
    if (match[i].size()) {
      cardsets[i].insert(1);
      if (patterns[i].getW() == PLUS) { // '... #((1 a)) ...'
        processDoublePars(i);
      }
      if (prev == i - 2) {
        if (prev > -1) { // '(1 a) +|* #(2 b)'
          for (j = match[i - 2].begin(); j != match[i - 2].end(); j++) {
            for (k = match[i].begin(); (k != match[i].end() && *k > *j); k++) {
              cardsets[i - 1].insert(*k - *j - 1);
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
    if (patterns[i].getW() != STAR) {
      numOfNonStars++;
    }
  }
  correctCardsets(numOfNonStars);
}

/*
\subsection{Function ~correctCardsets~}

Inserts and deletes missing and incorrect (respectively) zero values in the
sets of cardinality candidates. Subsequently, all elements exceeding a certain
threshold (which depends on the number of non-asterisk units in the pattern)
are erased.

*/
void NFA::correctCardsets(int nonStars) {
  set<size_t>::iterator j;
  for (int i = 0; i < f; i++) { // correct zeros
    if (patterns[i].getW() == STAR) {
      cardsets[i].insert(0);
    }
    else {
      cardsets[i].erase(0);
    }
    int k = (patterns[i].getW() == STAR ? 1 : 2);
    if ((j = cardsets[i].lower_bound(numOfLabels - nonStars + k))
        != cardsets[i].end()) { // delete too high candidates
      cardsets[i].erase(j, cardsets[i].end());
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
  Instant *pStart = new DateTime(instanttype);
  Instant *pEnd = new DateTime(instanttype);
  SecInterval *pIv = new SecInterval(0);
  SecInterval *uIv = new SecInterval(ul.timeInterval);
  set<string> ivs;
  if (!patterns[pos].getI().empty()) {
    ivs = patterns[pos].getI();
    for (j = ivs.begin(); j != ivs.end(); j++) {
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
  set<string> lbs = patterns[pos].getL();
  if (!lbs.empty()) {
    result = false;
    for (i = lbs.begin(); i != lbs.end(); i++) {
      if (!ul.constValue.GetValue().compare(*i)) { // look for a matching label
        result = true;
      }
    }
  }
  return result;
}

/*
\subsection{Function ~buildSequences~}

Derives all possible ULabel sequences from the cardinality candidates. Only
sequences with length ~numOfLabels~ are accepted. This function is necessary
for ~rewrite~. For ~matches~, see ~getNextSeq~.

*/
void NFA::buildSequences() {
  multiset<size_t> seq;
  vector<size_t> cards;
  set<size_t>::iterator it;
  int maxNumber = 0;
  size_t totalSize = 1;
  size_t j, cardSum, partSum;
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
  cout << "totalSize = " << totalSize << endl;
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
bool NFA::checkDoublePars(multiset<size_t> sequence) {
  vector<size_t> seq(sequence.begin(), sequence.end());
  size_t max = -1;
  for (int i = 0; i < (int)seq.size(); i++) {
    if (doublePars.count(i)) {
      if (i < (int)seq.size() - 1) {
        max = seq[i + 1] - 1;
      }
      else {
        max = numOfLabels - 1;
      }  
      for (size_t j = seq[i]; j <= max; j++) {
        if (!match[i].count(j)) {
          return false;
        }
      }
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
bool NFA::conditionsMatch(MLabel const &ml) {
  bool proceed(false);
  multiset<size_t>::iterator it;
  if (conds.empty()) {
    return true;
  }
  if (!ml.GetNoComponents()) { // empty MLabel
    return evaluateEmptyML();
  }
  maxCardPos = 0;
  for (int i = 0; i < f; i++) { // determine id of most cardinality candidates
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
  cout << "seqMax = " << seqMax << endl;
  seqCounter = 0;
  computeSeqOrder();
  multiset<size_t> seq = getNextSeq();
  size_t numOfRelCombs = getRelevantCombs();
  if (seqMax <= numOfRelCombs) { // standard case
    for (int i = 0; i < (int)conds.size(); i++) {
      do {
        proceed = false;
        if (conds[i].getKeysSize()) {
          buildCondMatchings(i, seq);
        }
        if (!evaluateCond(ml, i, seq)) {
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
  }
  else { // accelerated case
    numOfNegEvals = 0;
    for (int i = 0; i < (int)conds.size(); i++) {
      do {
        proceed = false;
        if (conds[i].getKeysSize()) {
          buildCondMatchings(i, seq);
        }
        if (!evaluateCond(ml, i, seq)) {
//           if (numOfNegEvals % 10 == 0) {
//             cout << numOfNegEvals << " negative evaluations now." << endl;
//           }
          if (numOfNegEvals == numOfRelCombs) { // relevant sequences tested
            return false;
          }
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
  }
  return true;
}

/*
\subsection{Function ~computeSeqOrder~}

Computes the order in which the sequences will be built. More exactly, the
sequences will first differ in the positions having variables.

*/
void NFA::computeSeqOrder() {
  set<int> used;
  int k = 0;
  for (int i = 0; i < (int)conds.size(); i++) {
    for (int j = 0; conds[i].getPId(j) != -1; j++) {
      if (!used.count(conds[i].getPId(j))) {
        seqOrder[k] = conds[i].getPId(j);
        k++;
        used.insert(conds[i].getPId(j));
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
\subsection{Function ~getRelevantCombs~}

Computes and returns the (maximal) number of relevant combinations, depending
on the types of the occurring conditions and their number.

*/
size_t NFA::getRelevantCombs() {
  map<int, size_t> factors;
  map<int, size_t>::iterator it;
  int key, pos;
  size_t result, factor;
  for (int i = 0; i < (int)conds.size(); i++) {
    for (int j = 0; j < (int)conds[i].getKeysSize(); j++) {
      pos = conds[i].getPId(j);
      key = conds[i].getKey(j);
      if (((key == 2) && isFixed(pos, true))
       || ((key == 3) && isFixed(pos, false))) {
        factor = 1; // unique substitution possibility for these cases
      }
      else if ((key == 4) || !pos || (pos == f - 1)) { // card or border
        factor = cardsets[pos].size();
      }
      else if ((key > 1) || !patterns[pos].getW()) { // start/end or no wildcard
        factor = cardsets[maxCardPos].size(); // numOfLabels
      }
      else {
        factor = (numOfLabels * numOfLabels);
      }
      if ((factors.find(pos) == factors.end()) || (factors[pos] < factor)) {
        factors[pos] = factor; // new or update
      }
    }
  }
  result = 1;
  for (it = factors.begin(); it != factors.end(); it++) {
    result *= (*it).second;
  }
  cout << "there are " << result << " relevant combinations" << endl;
  return result;
}

/*
\subsection{Function ~isFixed~}

Returns true if and only if the start or the end of matching (depending on the
second parameter) is fixed by non-wildcard unit patterns, e.g., for () () X +,
the first unit label matching X is always the same, so isFixed(2, true) = true.

*/
bool NFA::isFixed(int pos, bool start) {
  int a = (start ? 0 : pos + 1);
  int b = (start ? pos : f);
  for (int i = a; i < b; i++) {
    if (patterns[i].getW()) {
      return false;
    }
  }
  return true;
}

/*
\subsection{Function ~getNextSeq~}

Invoked during ~matches~, similar to ~buildSequences~, but with two crucial
differences: (1) Not all the possible matching sequences are built but only one
is constructed and returned -- in contrast to the operator ~rewrite~, where all
the sequences are needed. (2) The order of the sequences depends on whether a
unit pattern is referred to in the conditions.

*/
multiset<size_t> NFA::getNextSeq() {
  multiset<size_t> result;
  size_t cardSum, partSum, j;
  set<size_t>::iterator it;
  multiset<size_t>::iterator m;
  while (seqCounter < seqMax) {
    j = seqCounter;
    cardSum = 0;
    partSum = 0;
    size_t cards[f];
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
  cout << "no more sequences" << endl;
  return result;
}

/*
\subsection{Function ~evaluateEmptyML~}

This function is invoked in case of an empty moving label (i.e., with 0
components). A match is possible for a pattern like 'X [*] Y [*]' and conditions
X.card = 0, X.card = Y.card [*] 7. Time or label constraints are invalid.

*/
bool NFA::evaluateEmptyML() {
  for (int i = 0; i < (int)conds.size(); i++) {
    conds[i].resetSubst();
    for (int j = 0; j < conds[i].getKeysSize(); j++) {
      if (conds[i].getKey(j) != 4) { // only card conditions possible
        cout << "Error: Only cardinality conditions allowed" << endl;
        return false;
      }
      conds[i].substitute(j, "0");
    }
    if (!evaluate(conds[i].getSubst(), true)) {
      return false;
    }
  }
  cout << "conditions ok" << endl;
  return true;
}

/*
\subsection{Function ~buildCondMatchings~}

For one condition and one cardinality sequence, a set of possible matching
sequences is built if necessary, i.e., if the condition contains a label.

*/
void NFA::buildCondMatchings(int cId, multiset<size_t> sequence) {
  bool necessary(false);
  condMatchings.clear();
  int pId;
  size_t totalSize = 1;
  vector<size_t> condMatching;
  vector<size_t> seq(sequence.begin(), sequence.end());
  set<int> consideredIds;
  set<int>::iterator it;
  size_t size;
  seq.push_back(numOfLabels); // easier for last pattern
  for (int i = 0; i < conds[cId].getKeysSize(); i++) {
    pId = conds[cId].getPId(i);
    if (!conds[cId].getKey(i) && !consideredIds.count(pId)) { //no doubles
      totalSize *= seq[pId + 1] - seq[pId];
      consideredIds.insert(pId);
      necessary = true;
    }
  }
  if (necessary) {
    for (size_t j = 0; j < totalSize; j++) {
      size_t k = j;
      condMatching.clear();
      for (it = consideredIds.begin(); it != consideredIds.end(); it++) {
        size = seq[*it + 1] - seq[*it];
        condMatching.push_back(seq[*it] + k % size);
        k /= size;
      }
      condMatchings.insert(condMatching);
    }
//     printCondMatchings(5);
  }
}

/*
\subsection{Function ~evaluateCond~}

This function is invoked by ~conditionsMatch~ and checks whether a sequence of
possible cardinalities matches a certain condition.

*/
bool NFA::evaluateCond(MLabel const &ml, int cId, multiset<size_t> sequence) {
  conds[cId].resetSubst();
  bool success(false), replaced(false);
  string condStrCardTime, subst;
  vector<size_t> seq(sequence.begin(), sequence.end());
  for (int j = 0; j < conds[cId].getKeysSize(); j++) {
    int pId = conds[cId].getPId(j);
    if (conds[cId].getKey(j) == 4) { // card
      if (pId == f - 1) {
        subst.assign(int2Str(numOfLabels - seq[pId]));
      }
      else {
        subst.assign(int2Str(seq[pId + 1] - seq[pId]));
      }
    }
    else if (conds[cId].getKey(j) > 0) { // time, start, end
      size_t from = seq[pId];
      size_t to = (pId == f - 1 ? numOfLabels - 1 : seq[pId + 1] - 1);
      subst.assign(getTimeSubst(ml, conds[cId].getKey(j), from, to));
      if (!subst.compare("error")) {
        return false;
      }
    }
    if (conds[cId].getKey(j) > 0) { // time, start, end, card
      conds[cId].substitute(j, subst);
      if (conds[cId].getSubst().compare("error")) {
        replaced = true;
      }
      else {
        return false;
      }
    }
  } // cardinality and time substitutions completed
  condStrCardTime.assign(conds[cId].getSubst()); // save status
  if (condMatchings.empty() && replaced) {
    if (knownEval.count(condStrCardTime)) {
      return knownEval[condStrCardTime];
    }
    else {
      if (!evaluate(condStrCardTime, true)) {
        knownEval[condStrCardTime] = false;
//         cout << "false expression: " << condStrCardTime << endl;
        numOfNegEvals++;
        return false;
      }
      else {
        knownEval[condStrCardTime] = true;
        return true;
      }
    }
  }
  while (!condMatchings.empty()) {
    conds[cId].setSubst(condStrCardTime);
    int pos = 0;
    for (int j = 0; j < conds[cId].getKeysSize(); j++) {
      if (!conds[cId].getKey(j)) { // label
        subst.assign(getLabelSubst(ml, pos));
        conds[cId].substitute(j, subst);
        if (conds[cId].getSubst().compare("error")) {
          replaced = true;
        }
        else {
          return false;
        }
        pos++;
      }
    }
    if (knownEval.count(conds[cId].getSubst())) {
      if (knownEval[conds[cId].getSubst()]) {
        success = true;
      }
      else {
        return false;
      }
    }
    else {
      if (!evaluate(conds[cId].getSubst(), true)) {
        knownEval[conds[cId].getSubst()] = false;
//         cout << "false expression: " << conds[cId].getSubst() << endl;
        numOfNegEvals++;
        return false; // one false evaluation is enough to yield ~false~
      }
      else {
        knownEval[conds[cId].getSubst()] = true;
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
void Condition::substitute(int pos, string subst) {
  string varKey(vars[pos]);
  varKey.append(getType(keys[pos]));
  size_t varKeyPos = textSubst.find(varKey);
  if (varKeyPos == string::npos) {
    cout << "var.key " << varKey << " not found in " << textSubst << endl;
    textSubst.assign("error");
  }
  else {
    textSubst.replace(varKeyPos, varKey.size(), subst);
  }
}

string Condition::getType(int t) {
  switch (t) {
    case 0: return ".label";
    case 1: return ".time";
    case 2: return ".start";
    case 3: return ".end";
    case 4: return ".card";
    default: return ".ERROR";
  }
}

string Condition::getSubst(int s) {
  switch (s) {
    case 0: return "\"a\"";
    case 1: return "[const periods value ((\"2003-11-20-07:01:40\" "
                   "\"2003-11-20-07:45\" TRUE TRUE))]";
    case 2: return "[const instant value \"1909-12-19\"]";
    case 3: return "[const instant value \"2012-05-12\"]";
    case 4: return "1";
    default: return "";
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

/*
\subsection{Function ~getTimeSubst~}

Depending on the key (~time~, ~start~, or ~end~ are possible here) and on the
limits ~from~ and ~to~, the respective time information is retrieved from the
MLabel and returned as a string.

*/
string NFA::getTimeSubst(MLabel const &ml, Key key, size_t from, size_t to) {
  stringstream result;
  if ((from < 0) || (to >= numOfLabels)) {
    cout << "ULabel #" << (from < 0 ? from : to) << " does not exist." << endl;
    return "error";
  }
  if (from > to) {
    cout << "[" << from << ", " << to << "] is an invalid interval." << endl;
    return "error";
  }
  pair<size_t, size_t> from_to;
  Periods per(0);
  SecInterval iv(0);
  switch (key) {
    case 1: // time
      from_to = make_pair(from, to);
      if (knownPers.count(from_to)) {
        return knownPers[from_to];
      }
      else {
        result << "[const periods value("; // build string
        for (size_t i = from; i <= to; i++) {
          ml.Get(i, ul);
          per.MergeAdd(ul.timeInterval);
        }
        for (size_t i = 0; i < (size_t)per.GetNoComponents(); i++) {
          per.Get(i, iv);
          result << "(\"" << iv.start.ToString() << "\" \""
                 << iv.end.ToString() << "\" "
                 << (iv.lc ? "TRUE " : "FALSE ")
                 << (iv.rc ? "TRUE" : "FALSE") << ")"
                 << (i == to ? "" : " ");
        }
        result << ")]";
        knownPers[from_to] = result.str();
      }
      break;
    case 2: // start
      ml.Get(from, ul);
      result << "[const instant value \"" << ul.timeInterval.start.ToString()
             << "\"]";
      break;
    case 3: // end
      ml.Get(to, ul);
      result << "[const instant value \"" << ul.timeInterval.end.ToString()
             << "\"]";
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
string NFA::getLabelSubst(MLabel const &ml, int pos) {
  stringstream result;
  set<vector<size_t> >::iterator it = condMatchings.begin();
  ml.Get((*it)[pos], ul);
  if ((*it)[pos] >= numOfLabels) {
    cout << "Error: " << (*it)[pos] << " >= " << numOfLabels << endl;
    return "error";
  }
  if (pos == (int)(*it).size() - 1) {
    condMatchings.erase(it);
  }
  result << "\"" << ul.constValue.GetValue() << "\"";
  return result.str();
}

/*
\subsection{Type Mapping for operator ~stjpattern~}

*/
ListExpr textToPatternMap(ListExpr args) {
  if(!nl->HasLength(args,1)){
    return listutils::typeError("expected text");
  }
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
    case OPEN: {
      mlabel = static_cast<MLabel*>(args[0].addr);
      patternText = static_cast<FText*>(args[1].addr);
      if (!patternText->IsDefined()) {
        cout << "Error: undefined pattern text." << endl;
        return 0;
      }
      pattern = Pattern::getPattern(patternText->toText());
      if (!pattern) {
        cout << "Error: pattern not initialized." << endl;
        delete pattern;
        return 0;
      }
      if (!mlabel->IsDefined()) {
        cout << "Error: undefined MLabel." << endl;
      }
      MLabel* mlNew = mlabel->compress();
      rr = new RewriteResult(pattern->getRewriteSequences(*mlNew), mlNew,
                             pattern->getResults());
      delete pattern;
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
      ml = new MLabel(1);
      ml->rewrite(rr->getML(), rr->getCurrentSeq(), rr->getAssignments());
      result.addr = ml;
      rr->next(); 
      return YIELD;
    }  
    case CLOSE: {
      if (local.addr) {
        rr = ((RewriteResult*)local.addr);
        rr->killMLabel();
        delete rr;
      }
      return 0;
    }  
    default:
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
    signature = "MLabel x Text -> stream(MLabel)";
    appendSignature("MString x Text -> + stream(MLabel)");
    syntax    = "_ rewrite _";
    meaning   = "Rewrite a mlabel.";
  }
};

/*
\subsection{Type Mapping for operator ~compress~}

*/
ListExpr compressTypeMap(ListExpr args) {
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
\subsection{Selection Function for operator ~compress~}

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
\subsection{Value Mapping for operator ~compress~ (for a single MLabel)}

*/
template<class T>
int compressFun_1(Word* args, Word& result, int message, Word& local,
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
\subsection{Value Mapping for operator ~compress~ (for a stream of MLabels)}

*/
template<class T>
int compressFun_Str(Word* args, Word& result, int message, Word& local,
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
\subsection{Operator Info for operator ~compress~}

*/
struct compressInfo : OperatorInfo {
  compressInfo() {
    name      = "compress";
    signature = "MLabel -> MLabel";
    appendSignature("MString -> MLabel");
    appendSignature("stream(MLabel) -> stream(MLabel)");
    appendSignature("stream(MString) -> stream(MLabel)");
    syntax    = "compress(_)";
    meaning   = "Unites subsequent units.";
  }
};

/*
\subsection{Type Mapping for operator ~createml~}

*/
ListExpr createmlTypeMap(ListExpr args) {
  const string errMsg = "Expecting an integer and a real.";
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("Two arguments expected.");
  }
  if (nl->IsEqual(nl->First(args), CcInt::BasicType())
   && nl->IsEqual(nl->Second(args), CcReal::BasicType())) {
    return nl->SymbolAtom(MLabel::BasicType());
  }
  return NList::typeError(errMsg);
}

/*
\subsection{Value Mapping for operator ~createml~}

*/
int createmlFun(Word* args, Word& result, int message, Word& local, Supplier s){
  CcInt* ccint = static_cast<CcInt*>(args[0].addr);
  CcReal* ccreal = static_cast<CcReal*>(args[1].addr);
  int size;
  double rate;
  MLabel* ml = new MLabel(1);
  if (ccint->IsDefined() && ccreal->IsDefined()) {
    size = ccint->GetValue();
    rate = ccreal->GetValue();
    ml->create(size, rate);
  }
  else {
    cout << "Error: undefined value." << endl;
    ml->SetDefined(false);
  }
  result.addr = ml;
  return 0;
}

/*
\subsection{Operator Info for operator ~createml~}

*/
struct createmlInfo : OperatorInfo {
  createmlInfo() {
    name      = "createml";
    signature = "int x real -> MLabel";
    syntax    = "createml(_,_)";
    meaning   = "Creates an MLabel, the size being determined by the first"
                "parameter. The second one is the rate of different entries.";
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

      ValueMapping compressFuns[] = {compressFun_1<MLabel>,
                                     compressFun_1<MString>,
                                     compressFun_Str<MLabel>,
                                     compressFun_Str<MString>, 0};
      AddOperator(compressInfo(), compressFuns, compressSelect,compressTypeMap);

      AddOperator(createmlInfo(), createmlFun, createmlTypeMap);

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
