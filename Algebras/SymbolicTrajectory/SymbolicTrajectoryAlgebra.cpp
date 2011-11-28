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
#include "SymbolicTrajectoryPattern.h"
#include "SymbolicTrajectoryTools.h"
#include "SymbolicTrajectoryDateTime.h"



extern NestedList* nl;
extern QueryProcessor *qp;

#include <string>
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
  
  static Word     In( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct );

  static ListExpr Out( ListExpr typeInfo, Word value );

  static Word     Create( const ListExpr typeInfo );


  static void     Delete( const ListExpr typeInfo, Word& w );

  static void     Close( const ListExpr typeInfo, Word& w );

  static Word     Clone( const ListExpr typeInfo, const Word& w );

  static bool     KindCheck( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObj();

  static ListExpr Property();  
  
  
  // name of the type constructor
  
  static const string BasicType() { return "label"; }

  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
  

 private:
   
//  Label() {}
  
//  string text;
  char text[MAX_STRINGSIZE+1];
};  
  
Label::Label( string Text ) 
{
  strncpy(text, Text.c_str(), MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';  
}

Label::Label( char* Text ) 
{
  strncpy(text, Text, MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';
}
 

Label::Label( const Label& rhs ) {
  strncpy(text, rhs.text, MAX_STRINGSIZE); 
}

Label::~Label() {}

string Label::GetText() const 
{ 
  string str = text;
  return str;   
}

void Label::SetText(string &Text) 
{ 
  strncpy(text, Text.c_str(), MAX_STRINGSIZE);
  text[MAX_STRINGSIZE] = '\0';    
}


Word
Label::In( const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Word result = SetWord(Address(0));
  correct = false;
  
  NList list(instance);  
  
  if ( list.isAtom() )
  {
    if ( list.isString() )
    {
      string text = list.str();
     
      correct = true;
      result.addr = new Label( text );
    }
    else
    { 
      correct = false;
      cmsg.inFunError("Expecting a string!");
    }      
  }
  else 
  { 
    correct = false;
    cmsg.inFunError("Expecting one string atom!");
  }
  
  return result;
}


ListExpr
Label::Out( ListExpr typeInfo, Word value )
{
  Label* label = static_cast<Label*>( value.addr );
  NList element ( label->GetText(), true );

  return element.listExpr();
}



Word
Label::Create( const ListExpr typeInfo )
{
  return (SetWord( new Label( 0 ) ));
}
 
 
void
Label::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Label*>( w.addr );
  w.addr = 0;
}

void
Label::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Label*>( w.addr );
  w.addr = 0;
}

Word
Label::Clone( const ListExpr typeInfo, const Word& w )
{
  Label* label = static_cast<Label*>( w.addr );
  return SetWord( new Label(*label) );
}

int
Label::SizeOfObj()
{
  return sizeof(Label);
}

bool
Label::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Label::BasicType() ));
}



ListExpr
Label::Property()
{
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




//**********************************************************************************************************



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
ListExpr
ILabel::IntimeLabelProperty()
{
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
bool
ILabel::CheckIntimeLabel( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Intime<Label>::BasicType() ));
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

class ULabel : public UString {
public:  
  static const string BasicType() { return "ulabel"; }
  static ListExpr ULabelProperty();  
  static bool CheckULabel( ListExpr type, ListExpr& errorInfo );  
};

/*
5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
ULabel::ULabelProperty()
{
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
bool
ULabel::CheckULabel( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, ULabel::BasicType() ));  
}


/*
5.4 Creation of the type constructor ~uint~

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

class MLabel : public MString {
public:  
  static const string BasicType() { return "mlabel"; }
  static ListExpr MLabelProperty();  
  static bool CheckMLabel( ListExpr type, ListExpr& errorInfo );  
};


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
ListExpr
MLabel::MLabelProperty()
{
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
bool
MLabel::CheckMLabel( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, MLabel::BasicType() ));    
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
    Supplier s )
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
    
    
int
MovingExtSimpleSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == MLabel::BasicType() )
        return 0;

    return -1; // This point should never be reached
}    
 
 
ListExpr
MovingInstantExtTypeMapIntime( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg2, Instant::BasicType() ) )
        {
            if( nl->IsEqual( arg1, MLabel::BasicType() ) )
                return nl->SymbolAtom( Intime<Label>::BasicType() );
        }
    }
    return nl->SymbolAtom( Symbol::TYPEERROR() );
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
    MovingInstantExtTypeMapIntime );  

//*********************************************************************************************************



//*********************************************************************************************************



//**************************************************************************************************//

enum LabelsState { partial, complete };


class Labels : public Attribute
{

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
ostream& operator<<(ostream& os, const Label& lb)
{
  os << "(" << lb << ")";
  return os;
}



ostream& operator<<(ostream& os, const Labels& lbs)
{
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
  labels( n ),
  state( partial )
{
  SetDefined(true);
  if( n > 0 )
  {
    for( int i = 0; i < n; i++ )
    {
      Append( lb[i] );
    }
    Complete();
  }
}

/*
2.3.2 Copy Constructor

*/
Labels::Labels(const Labels& src):
  Attribute(src.IsDefined()),
  labels(src.labels.Size()),state(src.state){
  labels.copyFrom(src.labels);
}

/*

2.3.2 Destructor.

*/
Labels::~Labels()
{
}

Labels& Labels::operator=(const Labels& src){
  this->state = src.state;
  labels.copyFrom(src.labels);
  return *this;
}


/*
2.3.3 NumOfFLOBs.


*/
int Labels::NumOfFLOBs() const
{
  return 1;
}

/*
2.3.4 GetFLOB


*/
Flob *Labels::GetFLOB(const int i)
{
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
  assert( state == complete );
  Labels *lbs = new Labels( *this );
  return lbs;
}

void Labels::CopyFrom(const Attribute* right){
  *this = *( (Labels*) right);
}

/*
2.3.8 Sizeof

*/
size_t Labels::Sizeof() const
{
  return sizeof( *this );
}

/*
2.3.8 Print

*/
ostream& Labels::Print( ostream& os ) const
{
  return (os << *this);
}

/*
2.3.9 Append

Appends a label ~lb~ at the end of the DBArray labels.

*Precondition* ~state == partial~.

*/
void Labels::Append( const Label& lb )
{
  assert( state == partial );
  labels.Append( lb );
}

/*
2.3.10 Complete

Turns the element labels into the ~complete~ state.

*Precondition* ~state == partial~.

*/
void Labels::Complete()
{
  assert( state == partial );
  state = complete;
}

/*
2.3.11 Correct

Not yet implemented.

*/
bool Labels::Correct()
{
  return true;
}

/*
2.3.13 Destroy

Turns the element labels into the ~closed~ state destroying the
labels DBArray.

*Precondition* ~state == complete~.

*/
void Labels::Destroy()
{
  assert( state == complete );
  labels.destroy();
}



/*
2.3.14 NoLabels

Returns the number of labels of the DBArray labels.

*Precondition* ~state == complete~.

*/
int Labels::GetNoLabels() const
{
  return labels.Size();
}



/*
2.3.15 GetLabel

Returns a label indexed by ~i~.

*Precondition* ~state == complete \&\& 0 <= i < noLabels~.

*/
Label Labels::GetLabel( int i ) const
{
  assert( state == complete );
  assert( 0 <= i && i < GetNoLabels() );

  Label lb;
  labels.Get( i, &lb );
  return lb;
}


/*
2.3.16 GetState

Returns the state of the element labels in string format.

*/
string Labels::GetState() const
{
  switch( state )
  {
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
const bool Labels::IsEmpty() const
{
  assert( state == complete );
   
//  int i = GetNoLabels();
  
   return GetNoLabels() == 0;
//  return i == 0;
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


//****************************************************************************************************//




//**********************************************************************************************************
//~pattern~

class Pattern
{
 public:
//  Pattern() {};   
  Pattern( string const &text );
  inline Pattern( string const &text , PatParser const &patParser) { SetText(text); SetPatParser(patParser) ;}
  Pattern( const Pattern& rhs );
  ~Pattern();
  
  inline string GetText() const { return text;}
  void SetText( string const &Text );
  inline void SetPatParser( PatParser const &patParser ) { Pattern::patParser = patParser ;}  
  bool Matches(MLabel const &mlabel);
//  bool parserSet() { return parserSet;}
  
  Pattern* Clone();
  
  
  // algebra support functions
  
  static Word     In( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct );

  static ListExpr Out( ListExpr typeInfo, Word value );

  static Word     Create( const ListExpr typeInfo );


  static void     Delete( const ListExpr typeInfo, Word& w );

  static void     Close( const ListExpr typeInfo, Word& w );

  static Word     Clone( const ListExpr typeInfo, const Word& w );

  static bool     KindCheck( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObj();

  static ListExpr Property();  
  
  
  // name of the type constructor
  
  static const string BasicType() { return "pattern"; }

  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
  

 private:
   
  Pattern() {};
  vector<SinglePattern> getPattern(string const &text);
  
  string text;
  
//  vector<SinglePattern> s_pattern;   
  PatParser patParser;
//  bool parserSet;
};  


Pattern::Pattern( string const &Text ) 
{
 
  text = Text;
  PatParser patParser(text);
  SetPatParser(patParser); 
}


Pattern::Pattern( const Pattern& rhs ) {
  text = rhs.text;  
  PatParser patParser(text);
  SetPatParser(patParser);   
}

Pattern::~Pattern() {}


void Pattern::SetText( string const &Text ) 
{ 
  text = Text;
 
  PatParser patParser(text);
  SetPatParser(patParser); 
}


Word
Pattern::In( const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Word result = SetWord(Address(0));
  correct = false;
  
  NList list(instance);  
  
  if ( list.isAtom() )
  {
    if ( list.isText() )
    {
      string text = list.str();
      
      PatParser patParser( text );
      
      if ( patParser.isValid() ) 
      {
        correct = true;          
        result.addr = new Pattern( text, patParser );
      }	
      else
      {
        correct = false;
        cmsg.inFunError( patParser.getErrMsg() );	
      }

    }
    else
    { 
      correct = false;
      cmsg.inFunError("Expecting a text!");
    }      
    
    
  }
  else 
  { 
    correct = false;
    cmsg.inFunError("Expecting one text atom!");
  }
  
  return result;
}


ListExpr
Pattern::Out( ListExpr typeInfo, Word value )
{
  Pattern* pattern = static_cast<Pattern*>( value.addr );
  NList element ( pattern->GetText(), true, true );

  return element.listExpr();
}



Word
Pattern::Create( const ListExpr typeInfo )
{
  return (SetWord( new Pattern( "" ) ));
}
 
 
void
Pattern::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Pattern*>( w.addr );
  w.addr = 0;
}

void
Pattern::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Pattern*>( w.addr );
  w.addr = 0;
}

Word
Pattern::Clone( const ListExpr typeInfo, const Word& w )
{
  Pattern* pattern = static_cast<Pattern*>( w.addr );
  return SetWord( new Pattern(*pattern) );
}

int
Pattern::SizeOfObj()
{
  return sizeof(Pattern);
}

bool
Pattern::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Pattern::BasicType() ));
}



ListExpr
Pattern::Property()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
               nl->StringAtom("Example Type List"),
               nl->StringAtom("List Rep"),
               nl->StringAtom("Example List"),
               nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
               nl->StringAtom(Pattern::BasicType()),
               nl->StringAtom("<pattern>"),
               nl->TextAtom("\' (monday at_home) X (_ _) // X.start = 01.01.2011 \'"),
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



vector<SinglePattern> Pattern::getPattern(string const &text)
{
  vector<SinglePattern> s_pattern;
    
//  PatParser patParser(text);  
   
  return s_pattern;
}

bool Pattern::Matches(MLabel const &mlabel) 
{
  bool result = false;
  vector<SinglePattern> s_pattern;  
  
  cout << mlabel.GetNoComponents() << endl;
  
  
  
  
  return result; 
}


//-----------------------------------------------------------------------

ListExpr
textToPatternMap( ListExpr args )
{
  NList type(args);

  if ( type.first() == NList( FText::BasicType() ) ) {
     return NList(Pattern::BasicType()).listExpr();     
  }  

  return NList::typeError("Expecting a text!");
}


int
patternFun (Word* args, Word& result, int message,
              Word& local, Supplier s)
{

  FText* patternText = static_cast<FText*>( args[0].addr );
  
  result = qp->ResultStorage(s);
                                //query processor has provided
                                //a Pattern instance for the result
  
  Pattern* p = static_cast<Pattern*>( result.addr );
  p->SetText( patternText->GetValue() );
  
  return 0;
}


struct patternInfo : OperatorInfo {

  patternInfo()
  {
    name      = "stjpattern";
    signature = " Text -> " + Pattern::BasicType();
    syntax    = "_ stjpattern";
    meaning   = "Create Pattern from Text.";
  }

};


//---------------------------------------------------------------------------------

ListExpr
matchesTypeMap( ListExpr args )
{
  NList type(args);
  const string errMsg = "Expecting a mlabel and a pattern "
	                "or a mlabel and a text";

  // first alternative: mlabel x pattern -> bool
  if ( type == NList(MLabel::BasicType(), Pattern::BasicType()) ) {
    return NList(CcBool::BasicType()).listExpr();
  }

  // second alternative: mlabel x text -> bool
  if ( type == NList(MLabel::BasicType(), FText::BasicType()) ) {  
//  if ( type.first() == NList(MLabel_BasicType()) && type.second().isText() ) {
    return NList(CcBool::BasicType()).listExpr();
  }

  return NList::typeError(errMsg);
}


int
matchesSelect( ListExpr args )
{
  NList type(args);
  if ( type.second().isSymbol( Pattern::BasicType() ) )
    return 1;
  else
    return 0;
}

int
matchesFun_MP (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  MLabel* mlabel = static_cast<MLabel*>( args[0].addr );
  Pattern* pattern = static_cast<Pattern*>( args[1].addr );

  result = qp->ResultStorage(s);
                                //query processor has provided
                                //a CcBool instance for the result

  CcBool* b = static_cast<CcBool*>( result.addr );

  bool res = ( pattern->Matches( *mlabel ) );

  b->Set(true, res); //the first argument says the boolean
                     //value is defined, the second is the
                     //real boolean value)
  return 0;
}

int
matchesFun_MT (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  MLabel* mlabel = static_cast<MLabel*>( args[0].addr );
  FText* patternText = static_cast<FText*>( args[1].addr );
  Pattern pattern( patternText->GetValue() );

  result = qp->ResultStorage(s);
                                //query processor has provided
                                //a CcBool instance for the result

  CcBool* b = static_cast<CcBool*>( result.addr );

  bool res = ( pattern.Matches( *mlabel ) );

  b->Set(true, res); //the first argument says the boolean
                     //value is defined, the second is the
                     //real boolean value)
  return 0;
}

struct matchesInfo : OperatorInfo {

  matchesInfo()
  {
    name      = "matches";

    signature = MLabel::BasicType() + " x " + Pattern::BasicType() + " -> "
    + CcBool::BasicType();
    // since this is an overloaded operator we append
    // an alternative signature here
    appendSignature( MLabel::BasicType() + " x Text -> " + CcBool::BasicType() );
    syntax    = "_ matches _";
    meaning   = "Match predicate.";
  }
};



//****************************************************************************************************//



  
class SymbolicTrajectoryAlgebra : public Algebra
{
  public:
    SymbolicTrajectoryAlgebra() : Algebra()
    {
      
     // 5.2 Registration of Types

     AddTypeConstructor( &labelTC );
     AddTypeConstructor( &intimelabel );
     AddTypeConstructor( &unitlabel );
     AddTypeConstructor( &movinglabel );    

     movinglabel.AssociateKind( Kind::TEMPORAL() );
     movinglabel.AssociateKind( Kind::DATA() );
     
     AddTypeConstructor( &labelsTC );

     AddTypeConstructor( &patternTC );     
     
     // 5.3 Registration of Operators
    AddOperator( &temporalatinstantext );
     
    AddOperator( patternInfo(), patternFun, textToPatternMap );     
    
    ValueMapping matchesFuns[] = { matchesFun_MT, matchesFun_MP, 0 };
    AddOperator( matchesInfo(), matchesFuns, matchesSelect, matchesTypeMap );    
     
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


