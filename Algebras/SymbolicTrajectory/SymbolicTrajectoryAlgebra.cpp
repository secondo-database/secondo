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
//#include "SymbolicTrajectoryAlgebra.h"
#include "Stream.h"
#include "SecParser.h"
#include "Pattern.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include <string>
#include <vector>

using namespace std;


namespace stj {

class Label
{
 public:
  Label() {};
  Label( string text );
  Label( char* Text );  
  Label( const Label& rhs );
  ~Label();
  
  string GetText() const;
  void SetText( string &text );
   
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
  Label* label = static_cast<Label*>( value.addr );
  NList element ( label->GetText(), true );
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

ListExpr
Labels::Out( ListExpr typeInfo, Word value )
{
  Labels* labels = static_cast<Labels*>(value.addr);
  
  if( !labels->IsDefined() )
  {
//   cout << "out undefined" << endl;    
    return  (NList( Symbol::UNDEFINED() )).listExpr(); 
  }
  if( labels->IsEmpty() )
  {
//   cout << "empty" << endl;    
    return (NList()).listExpr();       
  }
  else
  {
    NList element("", true);
   
    for( int i = 0; i < labels->GetNoLabels(); i++ )
    {
      element.append( NList( (labels->GetLabel(i)).GetText(), true ) );      
    }  
    
    return element.listExpr();    
  } // else
}



Word
Labels::In( const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Word result = SetWord(Address(0));
  correct = false;
 
  NList list(instance); 
    
  Labels* labels = new Labels( 0 );
  labels->SetDefined(true);

  while( !list.isEmpty() )
  {
    
    if ( !list.isAtom() && list.first().isAtom() && list.first().isString() ) {
    
      labels->Append( Label( list.first().str() ) );
      correct = true;       
    }
    else 
    {     
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

ListExpr
Labels::Property()
{
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
bool
Labels::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Labels::BasicType() ));
}

/*

3.5 ~Create~-function

*/
Word Labels::Create(const ListExpr typeInfo)
{
  Labels* labels = new Labels( 0 );
  return ( SetWord(labels) );
}

/*
3.6 ~Delete~-function

*/
void Labels::Delete(const ListExpr typeInfo, Word& w)
{
  Labels* labels = (Labels*)w.addr;

  labels->Destroy();
  delete labels;
}

/*
3.6 ~Open~-function

*/
bool
Labels::Open( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  Labels *lbs = (Labels*)Attribute::Open( valueRecord, offset, typeInfo );
  value.setAddr( lbs );
  return true;
}

/*
3.7 ~Save~-function

*/
bool
Labels::Save( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  Labels *lbs = (Labels *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, lbs );
  return true;
}

/*
3.8 ~Close~-function

*/
void Labels::Close(const ListExpr typeInfo, Word& w)
{
  Labels* labels = (Labels*)w.addr;
  delete labels;
}

/*
3.9 ~Clone~-function

*/
Word Labels::Clone(const ListExpr typeInfo, const Word& w)
{
  return SetWord( ((Labels*)w.addr)->Clone() );
}

/*
3.9 ~SizeOf~-function

*/
int Labels::SizeOfObj()
{
  return sizeof(Labels);
}

/*
3.10 ~Cast~-function

*/
void* Labels::Cast(void* addr)
{
  return (new (addr) Labels);
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

//**********************************************************************
//~pattern~

string Pattern::GetText() const {
  stringstream text;
  set<string>::iterator j;
  text << "~~~~~~pattern~~~~~~" << endl;
  for (unsigned int i = 0; i < patterns.size(); i++) {
    text << "[" << i << "] " << patterns[i].variable << " | "
         << patterns[i].interval << " | ";
    if (patterns[i].labelset.size() > 1) {
      text << "{";
    }
    for (j = patterns[i].labelset.begin();
         j != patterns[i].labelset.end(); j++) {
      if (j != patterns[i].labelset.begin()) {
        text << ", ";
      }
      text << *j;
    }
    if (patterns[i].labelset.size() > 1) {
      text << "}";
    }
    text << " | " << patterns[i].wildcard << endl;
  }
  text << "~~~~~~conditions~~~~~~" << endl;
  for (unsigned int i = 0; i < conditions.size(); i++) {
    text << "[" << i << "] " << conditions[i].condition << endl;
    text << "[subst] " << conditions[i].condsubst << endl;
    for (unsigned int j = 0; j < conditions[i].variables.size(); j++) {
      text << "[[" << j << "]] " << conditions[i].variables[j]
           << "." << conditions[i].keys[j] << endl;
    }
  }
  text << "~~~~~~results~~~~~~" << endl;
  for (unsigned int i = 0; i < results.size(); i++) {
    text << "[" << i << "] " << results[i].variable << " | "
         << results[i].interval << " | ";
    if (results[i].labelset.size() > 1) {
        text << "{";
    }
    if (!results[i].labelset.empty()) {
      text << *(results[i].labelset.begin());
    }
    for (j = results[i].labelset.begin();
         j != results[i].labelset.end(); j++) {
      text << ", " << *j;
    }
    if (results[i].labelset.size() > 1) {
      text << "}";
    }
    text << " | " << results[i].wildcard << endl;
  }
  text << "~~~~~~assignments~~~~~~" << endl;
  for (unsigned int i = 0; i < assignments.size(); i++) {
    text << "[" << i << "] " << assignments[i].variable << " | "
         << assignments[i].interval << " | ";
    if (assignments[i].labelset.size() > 1) {
        text << "{";
    }
    if (!assignments[i].labelset.empty()) {
      text << *(assignments[i].labelset.begin());
    }
    for (j = assignments[i].labelset.begin();
         j != assignments[i].labelset.end(); j++) {
      text << ", " << *j;
    }
    if (assignments[i].labelset.size() > 1) {
      text << "}";
    }
    text << " | "  << assignments[i].wildcard << endl;
  }
  text << endl;
  return text.str();
}

// name of the type constructor

const string Pattern::BasicType() {
  return "pattern";
}

const bool Pattern::checkType(const ListExpr type){
  return listutils::isSymbol(type, BasicType());
}

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

ListExpr Pattern::Out(ListExpr typeInfo, Word value) {
  Pattern* pattern = static_cast<Pattern*>(value.addr);
  NList element(pattern->GetText(), true, true);
  return element.listExpr();
}

Word Pattern::Create(const ListExpr typeInfo) {
  return (SetWord(new Pattern()));
}

void Pattern::Delete(const ListExpr typeInfo, Word& w) {
  delete static_cast<Pattern*>(w.addr);
  w.addr = 0;
}

void Pattern::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<Pattern*>(w.addr);
  w.addr = 0;
}

Word Pattern::Clone(const ListExpr typeInfo, const Word& w) {
  Pattern* pattern = static_cast<Pattern*>(w.addr);
  return SetWord(new Pattern(*pattern));
}

int Pattern::SizeOfObj() {
  return sizeof(Pattern);
}

bool Pattern::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, Pattern::BasicType()));
}

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


TypeConstructor patternTC(
  Pattern::BasicType(),                          // name of the type in SECONDO
  Pattern::Property,                // property function describing signature
  Pattern::Out, Pattern::In,         // Out and In functions
  0, 0,                            // SaveToList, RestoreFromList functions
  Pattern::Create, Pattern::Delete,  // object creation and deletion
  0, 0,                            // object open, save
  Pattern::Close, Pattern::Clone,    // close, and clone
  0,                               // cast function
  Pattern::SizeOfObj,               // sizeof function
  Pattern::KindCheck );             // kind checking function


void NFA::buildNFA(Pattern p) {
  cout << "start building NFA" << endl;
  nfaPatterns = p.patterns;
  for (int i = 0; i < numberOfStates - 1; i++) {
    // solve epsilon-transitions
    if (!(nfaPatterns[i].wildcard.compare("*"))) { // reading '*'
      transitions[i - 1][i - 1].insert(i + 1);
      nfaPatterns[i].wildcard.assign("+");
      if (i < numberOfStates - 2) {
        transitions[i][i + 1].insert(i + 2);
        if (nfaPatterns[i + 1].wildcard.compare("*")) { // handle '* (a 1) * *'
          int j = i + 2;
          while ((j < numberOfStates - 1)
                  && !(nfaPatterns[j].wildcard.compare("*"))) {
            transitions[i][i + 1].insert(j + 1);
            j++;
          }
        }
        else { // handle '* * ... *'
          int j = i + 1;
          while ((j < numberOfStates - 1)
                  && !(nfaPatterns[j].wildcard.compare("*"))) {
            transitions[i][i].insert(j + 1);
            j++;
          }
        }
      }
      else {
        transitions[i][i + 1].insert(numberOfStates - 1);
      }
    }
    else { // not reading '*'
      int j = i + 1; // handle '(a 1) * * ... *'
      while ((j < numberOfStates - 1)
              && !(nfaPatterns[j].wildcard.compare("*"))) {
        transitions[i][i].insert(j + 1);
        j++;
      }
    }
    if (!(nfaPatterns[i].wildcard.compare("+"))) {
      transitions[i][i].insert(i);
      transitions[i][i + 1].insert(i);
      transitions[i][i + 1].insert(i + 1);
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

bool NFA::match(MLabel const &ml) {
  for (size_t i = 0; i < 3/*(size_t)ml.GetNoComponents()*/; i++) {
    ml.Get(i, currentLabel);
    cout << "unit label #" << i << " is processed now" << endl;
    updateStates();
    if (currentStates.empty()) {
      cout << "no current state" << endl;
      return false;
    }
  }
  printCurrentStates();
  return (currentStates.count(numberOfStates - 1) > 0) ? true : false;
}

void NFA::printCurrentStates() {
  set<int>::iterator i;
  cout << "the set of active states is {";
  for (i = currentStates.begin(); i != currentStates.end(); i++) {
    cout << *i << " ";
  }
  cout << "}" << endl;
}

void NFA::updateStates() {
  /*bool result = false;
  set<int>::iterator i;
  set<string>::iterator k;
  for (i = currentStates.begin(); i != currentStates.end(); i++) {
    cout << "examine state #" << *i << endl;
    for (int j = 0; j < numberOfStates - 1; j++) {
      if (!transitions[*i][j].empty()) {
        cout << "there are " << transitions[*i][j].size()
             << " possible transitions" << endl;
        if (!nfaPatterns[j].labelset.empty()) {
          for (k = nfaPatterns[j].labelset.begin();
               k != nfaPatterns[j].labelset.end(); k++) {
            cout << "checking label " << *k << endl;
            CcString *label = new CcString(true, *k);
            if (currentLabel.Passes(*label)) { // look for a matching label
              result = true;
            }
            label->DeleteIfAllowed();
          }
        }
        else { // no label specified in unit pattern
          result = true;
        }
      }
      currentStates.erase(i);
      cout << "current state erased" << endl;
      if (result) {
        // TODO: insert the new states
                cout << "try to insert" << endl;
        set<int>::iterator it = transitions[*i][j].begin();

        cout << "I\'d like to insert, e.g. " << *it << endl;
        cout << "there are " << currentStates.size() << " current states now"
             << endl;
      }
    }
  }
  */
}

string NFA::toString() {
  cout << "start output, " << numberOfStates << " states" << endl;
  stringstream nfa;
  set<int>::iterator k;
  for (int i = 0; i < numberOfStates - 1; i++) {
    for (int j = i; j < numberOfStates - 1; j++) {
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

void Pattern::checkConditions() {
  int condSize = conditions.size();
  SecParser condParser;
  string queryString;
  ListExpr queryList;
  bool condOk = true;
  int removedConds = 0;
  bool correct = false;
  bool evaluable = false;
  bool defined = false;
  bool isFunction = false;
  OpTree tree;
  ListExpr resultType;
  for (int i = 0; i < condSize; i++) {
    cout << "there " << (condSize > 1 ? "are" : "is") << " still " << condSize
         << " condition" << (condSize > 1 ? "s" : "") << endl;
    conditions[i].condsubst.insert(0, "query ");
    switch (condParser.Text2List(conditions[i].condsubst, queryString)) {
      case 0:
        if (!nl->ReadFromString(queryString, queryList)) {
          cout << "ReadFromString error" << endl;
          condOk = false;
        }
        else {
          if (nl->IsEmpty(nl->Rest(queryList))) {
            cout << "Rest of list is empty" << endl;
            condOk = false;
          }
          else {
            cout << nl->ToString(nl->First(nl->Rest(queryList))) << endl;
            qp->Construct(nl->First(nl->Rest(queryList)), correct, evaluable,
                          defined, isFunction, tree, resultType);
            if (!correct) {
              cout << "type error" << endl;
              condOk = false;
            }
            else if (!evaluable) {
              cout << "not evaluable" << endl;
              condOk = false;
            }
            else if (nl->ToString(resultType).compare("bool")) {
              cout << "wrong result type " << nl->ToString(resultType) << endl;
              condOk = false;
            }
            else {
              condOk = true;
            }
          }
        }
        break;
      case 1:
        cout << "String cannot be converted to list" << endl;
        condOk = false;
        break;
      case 2:
        cout << "stack overflow" << endl;
        condOk = false;
        break;
      default: // should not occur
        condOk = false;
        break;
    }
    if (!condOk) {
      conditions.erase(conditions.begin() + i);
      cout << "condition deleted" << endl;
      removedConds++;
      i--;
      condSize = conditions.size();
    }
  }
  if (tree) {
    qp->Destroy(tree, true);
  }
  if (removedConds) {
    cout << removedConds << " invalid condition"
         << ((removedConds > 1) ? "s" : "") << " removed" << endl;
  }
}

bool Pattern::getPattern(string input, Pattern** p) {
  input.append("\n");
  cout << input << endl;
  const char *patternChar = input.c_str();
  return parseString(patternChar, p);
}

bool Pattern::matches(MLabel const &ml) {
  NFA *nfa = new NFA(patterns.size() + 1);
  nfa->buildNFA(*this);
  cout << nfa->toString() << endl;
  bool result = nfa->match(ml);
  delete nfa;
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
  result = qp->ResultStorage(s); //query processor has provided
                                 //a CcBool instance for the result
  CcBool* b = static_cast<CcBool*>(result.addr);
  if (!pattern->getPattern(patternText->toText(), &pattern)) {
    b->SetDefined(false);
    cout << "invalid pattern" << endl;
    return 0;
  }
  pattern->checkConditions();
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


