/*

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
    
  // name of the type constructor
  
  static const string BasicType() { return "label"; }

  static const bool checkType(const ListExpr type) {
    return listutils::isSymbol(type, BasicType());
  }

 private:
   
//  Label() {}
  
//  string text;
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
      result.addr = new Label( text );
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

//**********************************************************************


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
6 Type Constructor ~mlabel~

Type ~mlabel~ represents a moving string.

6.1 List Representation

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

6.2 function Describing the Signature of the Type Constructor

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
6.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool MLabel::CheckMLabel(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, MLabel::BasicType()));
}

/*
6.4 Creation of the type constructor ~mlabel~

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
int MappingAtInstantExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s)
{
    result = qp->ResultStorage( s );
    Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;

    ((Mapping*)args[0].addr)->AtInstant( *((Instant*)args[1].addr), *pResult );

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
  if(nl->SymbolValue(arg1) == MLabel::BasicType())
    return 0;
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

enum LabelsState { partial, complete };

class Labels : public Attribute {
  public:
    Labels( const int n, const Label *Lb = 0 );
    ~Labels();

    Labels(const Labels& src);
    Labels& operator=(const Labels& src);

    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);
    int Compare(const Attribute*) const;
    bool Adjacent(const Attribute*) const;
    Labels *Clone() const;
    size_t Sizeof() const;
    ostream& Print( ostream& os ) const;

    void Append( const Label &lb );
    void Complete();
    bool Correct();
    void Destroy();
    int GetNoLabels() const;
    Label GetLabel( int i ) const;
    string GetState() const;
    const bool IsEmpty() const;
    void CopyFrom(const Attribute* right);
    size_t HashValue() const;

    friend ostream& operator <<( ostream& os, const Labels& p );

    static Word     In( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct );

    static ListExpr Out( ListExpr typeInfo, Word value );

    static Word     Create( const ListExpr typeInfo );

    static void     Delete( const ListExpr typeInfo, Word& w );

    static void     Close( const ListExpr typeInfo, Word& w );

    static bool     Save( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );

    static bool     Open( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );

    static Word     Clone( const ListExpr typeInfo, const Word& w );


    static bool     KindCheck( ListExpr type, ListExpr& errorInfo );

    static int      SizeOfObj();

    static ListExpr Property();

    static void* Cast(void* addr);

    static const string BasicType() { return "labels"; }
    static const bool checkType(const ListExpr type){
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
    os << lbs.GetLabel( i ) << " ";
  os << ">";
  return os;
}

/*
2.3.1 Constructors.

This first constructor creates a new polygon.

*/
Labels::Labels( const int n, const Label *lb ) :
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
4 ~Pattern~
*/

/*
4.1 Function ~GetText~
Writes all pattern information into a string

*/
string Pattern::GetText() const {
  stringstream text;
  text << "~~~~~~pattern~~~~~~" << endl;
  for (unsigned int i = 0; i < patterns.size(); i++) {
    text << "[" << i << "] " << patterns[i].variable << " | ";
    text << setToString(patterns[i].intervalset);
    text << " | ";
    text << setToString(patterns[i].labelset);
    text << " | " << patterns[i].wildcard << endl;
  }
  text << "~~~~~~conditions~~~~~~" << endl;
  for (unsigned int i = 0; i < conditions.size(); i++) {
    text << "[" << i << "] " << conditions[i].condition << endl;
    text << "[subst] " << conditions[i].condsubst << endl;
    for (unsigned int j = 0; j < conditions[i].variables.size(); j++) {
      text << "[[" << j << "]] " << conditions[i].variables[j]
           << "." << conditions[i].keys[j] << " in #"
           << conditions[i].patternIds[j] << endl;
    }
  }
  text << "~~~~~~results~~~~~~" << endl;
  for (unsigned int i = 0; i < results.size(); i++) {
    text << "[" << i << "] " << results[i].variable << " | ";
    text << setToString(results[i].intervalset);
    text << " | ";
    text << setToString(results[i].labelset);
    text << " | " << results[i].wildcard << endl;
  }
  text << "~~~~~~assignments~~~~~~" << endl;
  for (unsigned int i = 0; i < assignments.size(); i++) {
    text << "[" << i << "] " << assignments[i].variable << " | ";
    text << setToString(assignments[i].intervalset);
    text << " | ";
    text << setToString(assignments[i].labelset);
    text << " | "  << assignments[i].wildcard << endl;
  }
  text << endl;
  return text.str();
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
      Pattern *pattern = new Pattern();
      cout << text << endl;
      text +="\n";
      if (stj::parseString(text.c_str(), &pattern)) {
        correct = true;
        result.addr = pattern;
      }
      else {
        correct = false;
        cmsg.inFunError("Parsing error.");
        delete pattern;
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
4.12 Function ~verifyConditions~
Loops through the conditions and checks whether each one is a syntactically
correct boolean expression. Invalid conditions are removed.

*/
void Pattern::verifyConditions() {
  int condSize = conditions.size();
  int removedConds = 0;
  for (int i = 0; i < condSize; i++) {
    cout << "there " << (condSize > 1 ? "are" : "is") << " still " << condSize
         << " condition" << (condSize > 1 ? "s" : "") << endl;
    if (!evaluate(conditions[i].condsubst, false)) {
      conditions.erase(conditions.begin() + i);
      cout << "condition deleted" << endl;
      removedConds++;
      i--;
      condSize = conditions.size();
    }
  }
  if (removedConds) {
    cout << removedConds << " invalid condition"
         << ((removedConds > 1) ? "s" : "") << " removed" << endl;
  }
}

/*
4.13 Function ~evaluate~
In case of testing a condition's syntactical correctness, we are only
interested in the result type. Thus, resultNeeded is false, an operator tree
is built, and true is returned if and only if the result type is boolean.
On the other hand, if we ask for the condition result, the function executes
the appropiate query and returns its result.

*/
bool evaluate(string conditionString, const bool resultNeeded) {
  bool isBool = false;
  bool isTrue = false;
  SecParser condParser;
  string queryString;
  ListExpr queryList, resultType;
  Word queryResult;
  bool correct = false;
  bool evaluable = false;
  bool defined = false;
  bool isFunction = false;
  OpTree tree = 0;
  conditionString.insert(0, "query ");
  switch (condParser.Text2List(conditionString, queryString)) {
    case 0:
      if (!nl->ReadFromString(queryString, queryList)) {
        cout << "ReadFromString error" << endl;
      }
      else {
        if (nl->IsEmpty(nl->Rest(queryList))) {
          cout << "Rest of list is empty" << endl;
        }
        else {
          cout << nl->ToString(nl->First(nl->Rest(queryList))) << endl;
          if (resultNeeded) { // evaluate the condition
            if (!qp->ExecuteQuery(nl->ToString(nl->First(nl->Rest(queryList))),
                                  queryResult)) {
              cout << "execution error" << endl;
            }
            else {
              CcBool *ccResult = static_cast<CcBool*>(queryResult.addr);
              isTrue = ccResult->GetValue();
            }
          }
          else { // get the result type
            qp->Construct(nl->First(nl->Rest(queryList)), correct, evaluable,
                          defined, isFunction, tree, resultType);
            if (!correct) {
              cout << "type error" << endl;
            }
            else if (!evaluable) {
              cout << "not evaluable" << endl;
            }
            else if (nl->ToString(resultType).compare("bool")) {
              cout << "wrong result type " << nl->ToString(resultType) << endl;
            }
            else {
              isBool = true;
            }
          }
        }
      }
      break;
    case 1:
      cout << "String cannot be converted to list" << endl;
      break;
    case 2:
      cout << "stack overflow" << endl;
      break;
    default: // should not occur
      break;
  }
  if (tree) {
    qp->Destroy(tree, true);
  }
  return resultNeeded ? isTrue : isBool;
}


/*
4.14 Function ~getPattern~
Calls the parser.

*/
bool Pattern::getPattern(string input, Pattern** p) {
  input.append("\n");
  cout << input << endl;
  const char *patternChar = input.c_str();
  return parseString(patternChar, p);
}

/*
4.15 Function ~matches~
Invokes the NFA construction and the matching procedure.

*/
bool Pattern::matches(MLabel const &ml) {
  NFA *nfa = new NFA(patterns.size() + 1);
  nfa->buildNFA(*this);
  cout << nfa->toString() << endl;
  bool result = nfa->match(ml);
  delete nfa;
  return result;
}

/*
4.16 Function Describing the Signature of the Type Constructor

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
4.17 Creation of the Type Constructor Instance

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
5 ~NFA~
*/

/*
5.1 Function ~buildNFA~
Reads the pattern and generates the state transitions

*/
void NFA::buildNFA(Pattern p) {
  nfaPatterns = p.patterns;
  if (!p.conditions.empty()) {
    nfaConditions = p.conditions;
  }
  for (int i = 0; i < numOfStates - 1; i++) { // solve epsilon-transitions
    if (!(nfaPatterns[i].wildcard.compare("*"))) { // reading '*'
      if (i > 0) {
        transitions[i - 1][i - 1].insert(i + 1);
      }
      transitions[i][i].insert(i);
      //nfaPatterns[i].wildcard.assign("+");
      if (i < numOfStates - 2) {
        transitions[i][i + 1].insert(i + 2);
        int j = i + 1;
        while ((j < numOfStates - 1)  // handle '* * ... *'
                && !(nfaPatterns[j].wildcard.compare("*"))) {
          transitions[i][i].insert(j + 1);
          transitions[i][j + 1].insert(j + 2);
          j++;
        }
        if (nfaPatterns[i + 1].wildcard.compare("*")) { // handle '* (a 1) * *'
          transitions[i][i + 1].insert(i);
          transitions[i][i + 1].insert(i + 1);
          int j = i + 2;
          while ((j < numOfStates - 1)
                  && !(nfaPatterns[j].wildcard.compare("*"))) {
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
      while ((j < numOfStates - 1)
              && !(nfaPatterns[j].wildcard.compare("*"))) {
        transitions[i][i].insert(j + 1);
        j++;
      }
    }
    if (!(nfaPatterns[i].wildcard.compare("+"))) { // reading '+'
      transitions[i][i].insert(i);
    }
    transitions[i][i].insert(i + 1);// state i, read pattern i => new state i+1
    if (i > 0) {  // remove duplicates for '* *'
      if (!(nfaPatterns[i - 1].wildcard.compare("+"))
       && !(nfaPatterns[i].wildcard.compare("+"))) {
        transitions[i - 1][i].clear();  
      }
    }
  }
}

/*
5.2 Function ~match~
Loops through the MLabel calling updateStates() for every ULabel.
True is returned if and only if the final state is an element of
currentStates after the loop.

*/
bool NFA::match(MLabel const &ml) {
  maxLabelId = (size_t)ml.GetNoComponents() - 1;
  for (size_t i = 0; i <= maxLabelId; i++) {
    ml.Get(i, curULabel);
    curULabelId = i;
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
  printCards();
  buildSequences();
  printSequences(50);
  if (!conditionsMatch(ml)) {
    return false;
  }
  return true;
}

/*
5.3 Function ~toString~
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
5.4 Function ~printCurrentStates~
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
5.5 Function ~printCards~
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
5.6 Function ~printSequences~
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
5.7 Function ~printCondMatchings~
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
5.8 Function ~updateStates~
Further functions are invoked to decide which transition can be applied to
which current state. The set of current states is updated.

*/
void NFA::updateStates() {
  bool result = false;
  bool labelResult, timeResult; // TODO: add cardResult
  set<int> newStates;
  newStates.clear();
  set<int>::iterator i, it;
  for (i = currentStates.begin(); i != currentStates.end(); i++) {
    for (int j = 0; j < numOfStates - 1; j++) {
      result = false;
      if (!transitions[*i][j].empty()) {
        if (!nfaPatterns[j].labelset.empty() // check labels
         || !nfaPatterns[j].relatedConditions.empty()) {
          labelResult = labelsMatch(j);
        }
        else { // no label specified in unit pattern or conditions
          labelResult = true;
        }
        if (!nfaPatterns[j].intervalset.empty() // check intervals
         || !nfaPatterns[j].relatedConditions.empty()) {
           timeResult = timesMatch(j);
        }
        else { // no interval specified in unit pattern or conditions
          timeResult = true;
        }
        result = labelResult & timeResult;
        if (result) { // insert the new states
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
5.9 Function ~storeMatch~

*/
void NFA::storeMatch(int state) { // TODO: shorten this
  size_t fromLabel, toLabel;
  int fromState, toState;
  set<size_t>::iterator it;
  if (nfaPatterns[state].wildcard.empty()) {
    matchings[state].insert(curULabelId);
    cardsets[state].insert(1); // cardinality is 1 without wildcard
    if ((state > 0) && !nfaPatterns[state - 1].wildcard.empty()) {
      if (state == 1) { // wildcard at 0, match(es) at 1
        cardsets[0].insert(*(matchings[1].rbegin()));
      }
      else {
        int j = state - 2; // search previous matching position
        if (matchings[j].empty()) {
          j--;
          while ((j >= 0) && matchings[j].empty()) {
            j--;
          }
          toLabel = *(matchings[state].rbegin()) - *(matchings[j].begin()) - 1;
          fromLabel = 0;
          fromState = j + 1;
          toState = state - 1;
          for (int k = fromState; k <= toState; k++) {
            for (size_t i = fromLabel; i <= toLabel; i++) {
              if ((i > 0) || !nfaPatterns[k].wildcard.compare("*")) {
                cardsets[k].insert(i); // do not insert 0 when wildcard is +
              }
            }
          }
        }
        else { // matching in state - 2
          it = matchings[state - 2].begin();
          while ((*it < curULabelId) && (it != matchings[state - 2].end())) {
            if ((curULabelId - *it - 1 > 0) // card != 0 for wildcard +
              || !nfaPatterns[state - 1].wildcard.compare("*")) {
              cardsets[state - 1].insert(curULabelId - *it - 1);
            }
            it++;
          }
        }
      }
    }
  }
  else if ((state == numOfStates - 2) // last state
        && !nfaPatterns[state].wildcard.empty()) {
    int j = state - 1; // search previous matching position
    if ((j >= 0) && matchings[j].empty()) {
      j--;
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
            if ((i > 0) || !nfaPatterns[k].wildcard.compare("*")) {
              cardsets[k].insert(i);
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
5.10 Function ~timesMatch~
Checks whether the current ULabel interval is completely enclosed in the
interval specified in the pattern.

*/
bool NFA::timesMatch(int pos) {
  bool result(false), elementOk(false);
  set<int>::iterator i;
  set<string>::iterator j;
  string varKey, currentLabelString;
  Instant *pStart = new DateTime(instanttype);
  Instant *pEnd = new DateTime(instanttype);
  SecInterval *pInterval = new SecInterval(0);
  SecInterval *uInterval = new SecInterval(curULabel.timeInterval);
  if (nfaPatterns[pos].intervalset.empty()) { // no interval specified
    result = true;
  }
  else {
    for (j = nfaPatterns[pos].intervalset.begin();
         j != nfaPatterns[pos].intervalset.end(); j++) {
      if (((*j)[0] > 96) && ((*j)[0] < 123)) { // 1st case: semantic date/time
        elementOk = checkSemanticDate(*j, *uInterval);
      }
      else if (((*j).find('-') == string::npos) // 2nd case: 19:09~22:00
            && (((*j).find(':') < (*j).find('~')) // on each side of [~],
                || ((*j)[0] == '~')) // there has to be either xx:yy or nothing
            && (((*j).find(':', (*j).find('~')) != string::npos)
                || (*j)[(*j).size() - 1] == '~')) {
        elementOk = checkDaytime(*j, *uInterval);
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
        pInterval->Set(*pStart, *pEnd, true, true);
        elementOk = pInterval->Contains(*uInterval);
      }
      if (elementOk) { // one matching interval is sufficient
        result = true;
      }
    }
  } // pattern intervals finished
  uInterval->DeleteIfAllowed();
  pInterval->DeleteIfAllowed();
  pStart->DeleteIfAllowed();
  pEnd->DeleteIfAllowed();
  return result;
}


/*
5.11 Function ~labelsMatch~
Checks whether the current ULabel label matches the unit pattern labelset at
position pos (if specified).

*/
bool NFA::labelsMatch(int pos) {
  bool result = false;
  set<string>::iterator k;
  set<int>::iterator i;
  string currentLabelString;
  if (nfaPatterns[pos].labelset.empty()) {
    result = true;
  }
  else { // check labels of the unit pattern
    for (k = nfaPatterns[pos].labelset.begin();
         k != nfaPatterns[pos].labelset.end(); k++) {
      CcString *label = new CcString(true, *k);
      if (curULabel.Passes(*label)) { // look for a matching label
        result = true;
      }
      label->DeleteIfAllowed();
    }
  }
  return result;
}

/*
5.12 Function ~buildSequences~
Derives all possible ulabel sequences from the cardinality candidates. Only
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
5.13 Function ~conditionsMatch~
Checks whether the specified conditions are fulfilled. The result is true, if
and only if there is (at least) one cardinality sequence that matches every
condition.

*/
bool NFA::conditionsMatch(MLabel const &ml) {
  bool goToNextCond(false);
  set<vector<size_t> >::iterator it;
  if (nfaConditions.empty()) {
    return true;
  }
  for (unsigned int i = 0; i < nfaConditions.size(); i++) {
    it = sequences.begin();
    goToNextCond = false;
    while ((it != sequences.end()) && !goToNextCond) {
      goToNextCond = false;
      if (!nfaConditions[i].keys.empty()) {
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
5.14 Function ~buildCondMatchings~

*/
void NFA::buildCondMatchings(unsigned int condId, vector<size_t> sequence) {
  bool necessary(false);
  condMatchings.clear();
  int patternId;
  size_t totalSize = 1;
  vector<size_t> condMatching;
  set<int> consideredIds;
  set<int>::iterator it;
  size_t size;
  sequence.push_back(maxLabelId + 1); // easier for last pattern
  for (unsigned int i = 0; i < nfaConditions[condId].keys.size(); i++) {
    patternId = nfaConditions[condId].patternIds[i];
    if ((nfaConditions[condId].keys[i] < 4) // only for label, time, start, end
      && !consideredIds.count(patternId)) { // no doubles
      totalSize *= sequence[patternId + 1] - sequence[patternId];
      consideredIds.insert(patternId);
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
5.15 Function ~evaluateCond~
This function is invoked by ~conditionsMatch~ and checks whether a sequence of
cardinalities matches a certain condition.

*/
bool NFA::evaluateCond(MLabel const &ml, unsigned int condId,
                       vector<size_t> sequence) {
  bool success(false);
  string varKey, condStr, subst;
  size_t varKeyPos = string::npos;
  condStr.assign(nfaConditions[condId].condition);
  for (unsigned int j = 0; j < nfaConditions[condId].keys.size(); j++) {
    if (nfaConditions[condId].keys[j] == 4) { // card
      int patternId = nfaConditions[condId].patternIds[j];
      if (patternId == numOfStates - 2) {
        subst.assign(int2Str(maxLabelId - sequence[patternId] + 1));
      }
      else {
        subst.assign(int2Str(sequence[patternId + 1] - sequence[patternId]));
      }
      varKey.assign(nfaConditions[condId].variables[j]);
      varKey.append(nfaConditions[condId].types
                    [nfaConditions[condId].keys[j]].type);
      varKeyPos = condStr.find(varKey);
      if (varKeyPos != string::npos) {
        cout << "var.key " << varKey << " found at pos " << varKeyPos << endl;
        condStr.replace(varKeyPos, varKey.size(), subst);
      }
      else {
        cout << "var.key " << varKey << " not found" << endl;
        return false;
      }
    }
  }
  if (condMatchings.empty()) {
    if (!evaluate(condStr, true)) {
      cout << "no cardinality match" << endl;
      return false;
    }
    else {
      return true;
    }
  }
  while (!condMatchings.empty()) {
    for (unsigned int j = 0; j < nfaConditions[condId].keys.size(); j++) {
      cout << "consider condition #" << condId << "; variable "
           << nfaConditions[condId].variables[j] << endl;
      if (nfaConditions[condId].keys[j] < 4) { // label, time, start, end
        subst.assign(getNextSubst(ml, nfaConditions[condId].keys[j], j));
      }
      varKey.assign(nfaConditions[condId].variables[j]);
      varKey.append(nfaConditions[condId].types
                    [nfaConditions[condId].keys[j]].type);
      varKeyPos = condStr.find(varKey);
      if (varKeyPos != string::npos) {
        cout << "var.key " << varKey << " found at pos " << varKeyPos << endl;
        condStr.replace(varKeyPos, varKey.size(), subst);
      }
      else {
        cout << "var.key " << varKey << " not found" << endl;
        return false;
      }
    }
    cout << "evaluate !!!:::... " << condStr << " ...:::!!!" << endl;
    if (!evaluate(condStr, true)) {
      cout << "negative" << endl;
      return false; // one false evaluation is enough to yield ~false~
    }
    else {
      success = true;
    }
  }
  return success;
}

/*
5.16 Function ~getNextSubst~

*/
string NFA::getNextSubst(MLabel const &ml, Key key, unsigned int pos) {
  string result;
  set<vector<size_t> >::iterator it = condMatchings.begin();
  vector<size_t> positions = *it;
  condMatchings.erase(it);
  if ((positions)[pos] > maxLabelId) {
    cout << "PROBLEM: " << (positions)[pos] << endl;
  }
  ml.Get(positions[pos], curULabel);
  switch (key) {
    case 0: // label
      result.assign("\"");
      result.append(curULabel.constValue.GetValue());
      result.append("\"");
      break;
    default:
      break;
  }
  return result;
}

ListExpr textToPatternMap(ListExpr args) {
  NList type(args);
  if (type.first() == NList(FText::BasicType())) {
    return NList(Pattern::BasicType()).listExpr();
  }
  return NList::typeError("Expecting a text!");
}

int patternFun(Word* args, Word& result, int message, Word& local, Supplier s){
  FText* patternText = static_cast<FText*>(args[0].addr);
  result = qp->ResultStorage(s);
  //Pattern* ppp = static_cast<Pattern*>(result.addr);
  //string pt = patternText->GetValue().c_str();
  cout << "parse with new pattern" << endl;
  Pattern* p = new Pattern();
  if (!p->getPattern(patternText->toText(), &p)) {
    cout << "failed" << endl;
  }
  cout << "done" << endl; 
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

//---------------------------------------------------------------------------

ListExpr matchesTypeMap(ListExpr args) {
  NList type(args);
  const string errMsg = "Expecting a mlabel and a pattern "
                "or a mlabel and a text";
  if (type == NList(MLabel::BasicType(), Pattern::BasicType())) {
    return NList(CcBool::BasicType()).listExpr();
  }
  if (type == NList(MLabel::BasicType(), FText::BasicType())) {
    return NList(CcBool::BasicType()).listExpr();
  }
  return NList::typeError(errMsg);
}

int matchesSelect(ListExpr args) {
  NList type(args);
  return (type.second().isSymbol(Pattern::BasicType())) ? 1 : 0;
}

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

int matchesFun_MT (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  MLabel* mlabel = static_cast<MLabel*>(args[0].addr);
  FText* patternText = static_cast<FText*>(args[1].addr);
  Pattern *pattern = new Pattern();
  result = qp->ResultStorage(s); //query processor has provided a CcBool
  CcBool* b = static_cast<CcBool*>(result.addr); // instance for the result
  if (!pattern->getPattern(patternText->toText(), &pattern)) {
    b->SetDefined(false);
    cout << "invalid pattern" << endl;
    delete pattern;
    return 0;
  }
  pattern->verifyConditions();
  bool res = pattern->matches(*mlabel);
  delete pattern;
  b->Set(true, res); //the first argument says the boolean value is defined,
                     //the second is the real boolean value
  return 0;
}

struct matchesInfo : OperatorInfo {
  matchesInfo() {
    name      = "matches";
    signature = MLabel::BasicType() + " x " + Pattern::BasicType() + " -> "
    + CcBool::BasicType();
    // overloaded operator => alternative signature appended
    appendSignature(MLabel::BasicType() + " x Text -> " + CcBool::BasicType());
    syntax    = "_ matches _";
    meaning   = "Match predicate.";
  }
};

//--------------------------------------------------------------------------

ListExpr sintstreamType( ListExpr args ) {
  string err = "int x int expected";
  if(!nl->HasLength(args,2))
    return listutils::typeError(err);
  if(!listutils::isSymbol(nl->First(args)) ||
     !listutils::isSymbol(nl->Second(args)))
    return listutils::typeError(err);
  return nl->TwoElemList(nl->SymbolAtom(Stream<CcInt>::BasicType()),
                         nl->SymbolAtom(CcInt::BasicType()));
}

int sintstreamFun (Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  // An auxiliary type which keeps the state of this
  // operation during two requests
  struct Range {
    int current;
    int last;
    Range(CcInt* i1, CcInt* i2) {
      // Do a proper initialization even if one of the
      // arguments has an undefined value
      if (i1->IsDefined() && i2->IsDefined()) {
        current = i1->GetIntval();
        last = i2->GetIntval();
      }
      else {
// this initialization will create an empty stream
        current = 1;
        last = 0;
      }
    }
  };
  Range* range = static_cast<Range*>(local.addr);
  switch( message )
  {
    case OPEN: { // initialize the local storage
      CcInt* i1 = static_cast<CcInt*>( args[0].addr );
      CcInt* i2 = static_cast<CcInt*>( args[1].addr );
      range = new Range(i1, i2);
      local.addr = range;
      return 0;
    }
    case REQUEST: { // return the next stream element
      if (range->current <= range->last ) {
        CcInt* elem = new CcInt(true, range->current++);
        result.addr = elem;
        return YIELD;
      }
      else {
// you should always set the result to null
// before you return a CANCEL
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: { // free the local storage
      if (range != 0) {
        delete range;
        local.addr = 0;
      }
      return 0;
    }
    default: {
      /* should never happen */
      return -1;
    }
  }
}

struct sintstreamInfo : OperatorInfo
{
  sintstreamInfo() : OperatorInfo()
  {
    name      = "sintstream";
    signature = CcInt::BasicType() + " x " + CcInt::BasicType()
                + " -> stream(int)";
    syntax    = "_ sintstream _";
    meaning   = "Creates a stream of integers containing the numbers "
                "between the first and the second argument.";
  }
};


//***********************************************************************//

  
class SymbolicTrajectoryAlgebra : public Algebra {
  public:
    SymbolicTrajectoryAlgebra() : Algebra() {
      
      // 5.2 Registration of Types
      AddTypeConstructor( &labelTC );
      AddTypeConstructor( &intimelabel );
      AddTypeConstructor( &unitlabel );
      AddTypeConstructor( &movinglabel );

      movinglabel.AssociateKind( Kind::TEMPORAL() );
      movinglabel.AssociateKind( Kind::DATA() );

      AddTypeConstructor( &labelsTC );
      AddTypeConstructor( &patternTC );
//       AddTypeConstructor( &ruleTC );

      // 5.3 Registration of Operators
      AddOperator(&temporalatinstantext);
      AddOperator(patternInfo(), patternFun, textToPatternMap);

      ValueMapping matchesFuns[] = {matchesFun_MT, matchesFun_MP, 0};
      AddOperator(matchesInfo(), matchesFuns, matchesSelect, matchesTypeMap);

      //    AddOperator( consumeMLabelsInfo(), consumeMLabelsFun,
      //      consumeMLabelsTypeMap );

//       ValueMapping applyFuns[] = {applyFun_MT, applyFun_MP, 0};
//       AddOperator(applyInfo(), applyFuns, applySelect, applyTypeMap);

      //    AddOperator( sintstreamInfo(), sintstreamFun, sintstreamType );
    }
    ~SymbolicTrajectoryAlgebra() {}
};

// SymbolicTrajectoryAlgebra SymbolicTrajectoryAlgebra;

} // end of namespace ~stj~

extern "C"
Algebra*
InitializeSymbolicTrajectoryAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  return new stj::SymbolicTrajectoryAlgebra;
}


