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
//#include "SymbolicTrajectoryPattern.h"
#include "SymbolicTrajectoryTools.h"
//#include "SymbolicTrajectoryDateTime.h"
#include "SymbolicTrajectoryAlgebra.h"
#include "Stream.h"

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

//**********************************************************************



//**********************************************************************



//**********************************************************************

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


//**********************************************************************




//**********************************************************************
//~pattern~

inline Pattern::Pattern( string const &text , PatParser const &patParser) {
  SetText(text); SetPatParser(patParser);
}
inline string Pattern::GetText() const {
  return text;
}
inline void Pattern::SetPatParser( PatParser const &patParser ) {
  Pattern::patParser = patParser;
}
inline bool Pattern::isValid() {
  return patParser.isValid();
}
inline string Pattern::getErrMsg() {
  return patParser.getErrMsg();
}


// name of the type constructor

const string Pattern::BasicType() {
  return "pattern";
}

const bool Pattern::checkType(const ListExpr type){
  return listutils::isSymbol(type, BasicType());
}

bool Pattern::checkStartValues() {
  if ((currentULabel < 0) || (currentULabel >= maxULabel))
    return false;
  if ((currentPattern < 0) || (currentPattern >= s_pattern.size()))
    return false;
  else
    return true;
}

void Pattern::setStartEnd() {
  startDate = interval.start;
  if (!interval.lc)
    startDate.Add(dt);
  endDate = interval.end;
  if (!interval.rc)
    endDate.Minus(dt);
}

bool Pattern::checkLabels() {
  CcString *label = new CcString(true, pattern.lbs[0]);
  bool result = ul.Passes(*label) ? true : false;
  label->DeleteIfAllowed();
  return result;
}

bool Pattern::checkTimeRanges() {
  duration = endDate - startDate;
  if (duration.GetDay() > 0)
    return false; // duration > 24 h
  for (size_t j = 0; j < pattern.trs.size(); ++j) {
    startTimeRange = getStartTimeFromRange(pattern.trs[j]);
    endTimeRange = getEndTimeFromRange(pattern.trs[j]);
    startTime.Set(startDate.GetYear(), startDate.GetMonth(),
                  startDate.GetGregDay(), getHourFromTime(startTimeRange),
                  getMinuteFromTime(startTimeRange),
                  getSecondFromTime(startTimeRange),
                  getMillisecondFromTime(startTimeRange));
    startDate2 = startDate;
    if (!isPositivTimeRange(pattern.trs[j]))
      startDate2.Add(new DateTime(0, 1, durationtype));
    endTime.Set(startDate2.GetYear(), startDate2.GetMonth(),
                startDate2.GetGregDay(), getHourFromTime(endTimeRange),
                getMinuteFromTime(endTimeRange),
                getSecondFromTime(endTimeRange),
                getMillisecondFromTime(endTimeRange) );
    if ((startDate < startTime) || (endDate > endTime))
      return false;
  }
  return true;
}

bool Pattern::checkDateRanges() {
  for (size_t j = 0; j < pattern.dtrs.size(); ++j) {
    startPatternDateTime.ReadFrom(getSecDateTimeString(
                         getStartDateTimeFromRange(pattern.dtrs[j])));
    endPatternDateTime.ReadFrom(getSecDateTimeString(
                       getEndDateTimeFromRange(pattern.dtrs[j])));
    if ((startDate < startPatternDateTime) || (endDate > endPatternDateTime))
      return false;
  }
  return true;
}

      // 3 = weekday
      // 4 = day-time
      // 5 = month
bool Pattern::checkSemanticRanges() {
  for (size_t j = 0; j < pattern.sts.size(); ++j) {
    switch (pattern.sts[j].type) {
      case 3:
        if ((startDate.ToString()).substr(0,10) !=
           (endDate.ToString()).substr(0,10))  // not the same day
          return false;
        if (startDate.GetWeekday() != pattern.sts[j].value - 1)
          return false;
        break;
      case 4:
        if ((startDate.ToString()).substr(0,10) !=
            (endDate.ToString()).substr(0,10))  // not the same day
          return false;
        timeRange = getDayTimeRangeString(pattern.sts[j].value);
        startTimeRange = getStartTimeFromRange(timeRange);
        endTimeRange = getEndTimeFromRange(timeRange);
        startTime.Set(startDate.GetYear(), startDate.GetMonth(),
                      startDate.GetGregDay(),
                      getHourFromTime(startTimeRange),
                      getMinuteFromTime(startTimeRange),
                      getSecondFromTime(startTimeRange),
                      getMillisecondFromTime(startTimeRange));
        startDate2 = startDate;
        if (!isPositivTimeRange(timeRange))
          startDate2.Add(dt);
        endTime.Set(startDate2.GetYear(), startDate2.GetMonth(),
                    startDate2.GetGregDay(),
                    getHourFromTime(endTimeRange),
                    getMinuteFromTime(endTimeRange),
                    getSecondFromTime(endTimeRange),
                    getMillisecondFromTime(endTimeRange));
        if ((startDate < startTime) || (endDate > endTime))
          return false;
        break;
      case 5:
        if ((startDate.ToString()).substr(0,7) !=
            (endDate.ToString()).substr(0,7)) // not the same month/year
          return false;
        if (startDate.GetMonth() != pattern.sts[j].value)
          return false;
        break;
    }
  }
  return true;
}

bool Pattern::checkConditions() {
  for (size_t j = 0; j < pattern.conditions.size(); ++j) {
    patEquation = pattern.conditions[j];
    // key : 1="lb/lbs"; 3="start"; 4="end"; 6="card"
    // op : 1="="; 2="<"; 4=">"
    switch (patEquation.key) {
      case 1:
        if (patEquation.op & 1) {
          CcString *valueString = new CcString(true, patEquation.value);
          if (!ul.Passes(*valueString)) {
            valueString->DeleteIfAllowed();
            return false;
          }
          valueString->DeleteIfAllowed();
        }
        break;
      case 3:
        startPatternDateTime.ReadFrom(getSecDateTimeString(getFullDateTime(
                                      patEquation.value)));
        if (!(((patEquation.op & 1) && (startDate == startPatternDateTime))
         || ((patEquation.op & 2) && (startDate < startPatternDateTime))
         || ((patEquation.op & 4) && (startDate > startPatternDateTime))))
          return false;
        break;
      case 4:
        endPatternDateTime.ReadFrom(getSecDateTimeString(getFullDateTime(
                                    patEquation.value)));
        if (!(((patEquation.op & 1) && (endDate == endPatternDateTime))
         || ((patEquation.op & 2) && (endDate < endPatternDateTime))
         || ((patEquation.op & 4) && (endDate > endPatternDateTime))))
          return false;
        break;
    }
  }
  return true;
}

size_t Pattern::checkCardinalities() {
  bool result = false;
  for (size_t i = 0; i < matchings.size(); i++)
    if (matchings[i].hasCardinalityCondition)
      for (size_t j = 0;
           j < s_pattern[matchings[i].patternPos].conditions.size(); j++) {
        patEquation = s_pattern[matchings[i].patternPos].conditions[j];
        size_t equationValue = str2Int(patEquation.value);
        switch (patEquation.op) {
          case 1: // "="
            if (i == 0) // wildcard with card. cond. in the beginning
              result = (equationValue == matchings[i+1].labelPos);
            else if (i == matchings.size() - 1) // at the end
              result = (equationValue == maxULabel - matchings[i].labelPos);
            else
              result = (equationValue ==
                        matchings[i+1].labelPos - matchings[i-1].labelPos - 1);
            break;
          case 2: // "<"
            if (i == 0) // wildcard with card. cond. in the beginning
              result = (equationValue > matchings[i+1].labelPos);
            else if (i == matchings.size() - 1) // at the end
              result = (equationValue > maxULabel - matchings[i].labelPos);
            else
              result = (equationValue >
                        matchings[i+1].labelPos - matchings[i-1].labelPos - 1);
            break;
          case 4: // ">"
            if (i == 0) // wildcard with card. cond. in the beginning
              result = (equationValue < matchings[i+1].labelPos);
            else if (i == matchings.size() - 1) // at the end
              result = (equationValue < maxULabel - matchings[i].labelPos);
            else
              result = (equationValue <
                        matchings[i+1].labelPos - matchings[i-1].labelPos - 1);
            break;
          default: // should not occur
            cout << patEquation.op << " is an illegal operator" << endl;
            return false;
        }
        if (!result)
          return i;
      }
  return matchings.size();
}

void Pattern::setMatching(bool isW) {
  matching.labelPos = currentULabel;
  matching.patternPos = currentPattern;
  matching.isWildcard = isW;
  matching.hasCardinalityCondition = false;
  if (isW && !pattern.conditions.empty())
    for (size_t i = 0; i < pattern.conditions.size(); i++)
      if (pattern.conditions[i].key == 6)
        matching.hasCardinalityCondition = true;
}

void Pattern::matchingsToString() {
  for (size_t i = 0; i < matchings.size(); i++)
    cout << (matchings[i].isWildcard ? "wildcard at " : "   match at ") <<
             matchings[i].labelPos << "  " << matchings[i].patternPos <<
             (matchings[i].hasCardinalityCondition ? " *" : "") << "\n\n";
}

size_t Pattern::lastWildcardPosition(size_t skip) {
  size_t counter = 0;
  for (size_t i = matchings.size() - 1; i >= 0; i--)
    if (matchings[i].isWildcard) {
      counter ++;
      if (counter > skip)
        return i;        // if skip equals 2, the position of the 3rd last
                         // wildcard is returned
    }
  return matchings.size();
}

size_t Pattern::prepareBacktrack(size_t position) {
  size_t nextStartL = 0;
  matchesToDelete = matchings.size() - position;
  for (size_t j = 0; j < matchesToDelete; j++) { // delete old matchings
    if (!matchings.back().isWildcard)
      nextStartL = matchings.back().labelPos + 1;
    else {
      pattern = s_pattern[matchings.back().patternPos];
      if (!checkConditions())
        return maxULabel; // finish if wildcard breaks condition(s)
    }
    matchings.pop_back();
  }
  return result;
}

size_t Pattern::countWildcards() {
  size_t result = 0;
  for (size_t i = 0; i < matchings.size(); i++)
    if (matchings[i].isWildcard)
      result ++;
  return result;
}

bool Pattern::completeBacktrack(MLabel const &ml) {
  bool result = false;
  while (currentULabel < maxULabel) {
    matchingsToString();
    lastWildcardPos = lastWildcardPosition(0);
    nextStartLabel = prepareBacktrack(lastWildcardPos);
    cout << nextStartLabel << " " << lastWildcardPos << endl;
    result = SuffixMatch(ml, nextStartLabel, lastWildcardPos);
  }
  return result;
}

Pattern::Pattern(string const &Text) {
  text = Text;
  PatParser patParser(text);
  SetPatParser(patParser);
}


Pattern::Pattern(const Pattern& rhs) {
  text = rhs.text;  
  PatParser patParser(text);
  SetPatParser(patParser);
}

Pattern::~Pattern() {}


void Pattern::SetText(string const &Text) { 
  text = Text;
  PatParser patParser(text);
  SetPatParser(patParser); 
}


Word
Pattern::In( const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct ) {
  Word result = SetWord(Address(0));
  correct = false;
  NList list(instance);  
  if (list.isAtom()) {
    if (list.isText()) {
      string text = list.str();
      PatParser patParser(text);
      if (patParser.isValid()) {
        correct = true;          
        result.addr = new Pattern( text, patParser );
      }
      else {
        correct = false;
        cmsg.inFunError( patParser.getErrMsg() );
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
Pattern::Delete( const ListExpr typeInfo, Word& w ) {
  delete static_cast<Pattern*>( w.addr );
  w.addr = 0;
}

void
Pattern::Close( const ListExpr typeInfo, Word& w ) {
  delete static_cast<Pattern*>( w.addr );
  w.addr = 0;
}

Word
Pattern::Clone( const ListExpr typeInfo, const Word& w ) {
  Pattern* pattern = static_cast<Pattern*>( w.addr );
  return SetWord( new Pattern(*pattern) );
}

int
Pattern::SizeOfObj() {
  return sizeof(Pattern);
}

bool
Pattern::KindCheck( ListExpr type, ListExpr& errorInfo ) {
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


vector<SinglePattern> Pattern::getPattern()
{
  return patParser.getPattern(); 
}

bool Pattern::SingleMatch() {
  if (!pattern.lbs.empty())
    if (!checkLabels())
      return false;
  if (!pattern.trs.empty())
    if (!checkTimeRanges())
      return false;
  if (!pattern.dtrs.empty())
    if (!checkDateRanges())
      return false;
  if (!pattern.sts.empty())
    if (!checkSemanticRanges())
      return false;
  if (!pattern.conditions.empty()) {
    if (!checkConditions())
      return false;
    size_t cardCheck = checkCardinalities();
    if (cardCheck == matchings.size())
      cout << "cardinalities ok" << endl;
    else {
      cout << "cardinality mismatch: "
           << s_pattern[matchings[cardCheck].patternPos].toString() << endl;
      return false;
    }
  }
  return true;
}

bool Pattern::TotalMatch(MLabel const &ml) {
  endDate.SetType(datetime::instanttype);
  duration.SetType(datetime::durationtype);
  startTime.SetType(datetime::instanttype);
  endTime.SetType(datetime::instanttype);
  startDate2.SetType(datetime::instanttype);
  startPatternDateTime.SetType(datetime::instanttype);
  endPatternDateTime.SetType(datetime::instanttype);
  startDate.SetType(datetime::instanttype);
  maxULabel = ml.GetNoComponents();
  wildcard = false;
  s_pattern = getPattern();
  bool totalMatch;
  dt = new DateTime(0, 1, durationtype);
  bool match = SuffixMatch(ml, 0, 0);
  numberOfWildcards = countWildcards();
  matchingsToString();
  if (match) {
    if (matchings.back().isWildcard) {
      totalMatch = true; // (_ at_home) (_ at_university) *
      cout << "matches because last non-wildcard pattern matches a prefix of "
           << "the mlabel and is followed just by a wildcard" << endl;
    }
    else if (currentULabel == maxULabel) {
      totalMatch = true;
      cout << "matches because the ends match without wildcard" << endl;
    }
    else if (numberOfWildcards > 0) {
      totalMatch = completeBacktrack(ml);
      cout << (totalMatch ? "matches after backtracking" : "no match") << endl;
    }
    else { // no wildcard
      totalMatch = false;
      cout << "too short match without wildcard - no match" << endl;
    }
  }
  else if (numberOfWildcards == 0) {
    totalMatch = false;
    cout << "no match, no wildcard - no chance" << endl;
  }
  else { // no match && lastWildcardPosition() < matchings.size() - 1
    totalMatch = completeBacktrack(ml);
    cout << (totalMatch ? "matches after backtracking" : "no match") << endl;
  }
  if (dt)
    delete dt;
  return totalMatch;
}

bool Pattern::SuffixMatch(MLabel const &ml, size_t firstULabel,
                          size_t firstPattern) {
  currentULabel = firstULabel;
  currentPattern = firstPattern;
  if (!checkStartValues())
    return false;
  while (currentPattern < s_pattern.size()) {
    if (currentULabel >= maxULabel)
      return false;
    pattern = s_pattern[currentPattern];
    ml.Get(currentULabel, ul);
    interval.CopyFrom(ul.timeInterval);
    setStartEnd();
    result = true;
    if (pattern.wildcard == '*') {
      wildcard = true;
      setMatching(true);
      matchings.push_back(matching);
      if (!pattern.conditions.empty())
        if (!checkConditions())
          return false;
    }
    else {
      if (!SingleMatch() && !wildcard)
        return false;
      if (!SingleMatch())
        result = false;
      if (result) {
        setMatching(false);
        matchings.push_back(matching);
      }
      if (wildcard)
        if ((wildcard = !result))
          --currentPattern;
      currentULabel ++;
    }
    currentPattern ++;
  }
  return result;
}



//***********************************************************************
//~rule~

class Rule
{
 public:
  Rule() : defined(false) {};   
  Rule( string const &text );
  inline Rule(string const &text, RuleParser const &ruleParser) :
    defined(true) {
      SetText(text);
      SetRuleParser(ruleParser);
      
  }
  Rule( const Rule& rhs );
  ~Rule();
  
  inline string GetText() const { return text;}
  void SetText( string const &Text );
  inline void SetRuleParser( RuleParser const &ruleParser ) {
    Rule::ruleParser = ruleParser ;}  
//  bool Matches(MLabel const &mlabel);
  inline bool isValid() { return ruleParser.isValid() ;}
  inline string getErrMsg() { return ruleParser.getErrMsg() ;}
  inline bool IsDefined() {return defined;}
  
  Rule* Clone();
  
  
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
  
  static const string BasicType() { return "rule"; }

  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }
  

 private:
   
//  Rule() {};
//  vector<SinglePattern> getRule();
  
  string text;
  bool defined;
 
  RuleParser ruleParser;
};  


Rule::Rule( string const &Text )  : defined(true)
{
 
  text = Text;
  RuleParser ruleParser(text);
  SetRuleParser(ruleParser); 
}


Rule::Rule( const Rule& rhs )  : defined(true) {
  text = rhs.text;  
  RuleParser ruleParser(text);
  SetRuleParser(ruleParser);  
}

Rule::~Rule() {}


void Rule::SetText( string const &Text ) 
{ 
  text = Text;
 
  RuleParser ruleParser(text);
  SetRuleParser(ruleParser);; 
}


Word
Rule::In( const ListExpr typeInfo, const ListExpr instance,
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
      
      RuleParser ruleParser( text );
      
      if ( ruleParser.isValid() ) 
      {
        correct = true;          
        result.addr = new Rule( text, ruleParser );
      }
      else
      {
        correct = false;
        cmsg.inFunError( ruleParser.getErrMsg() );
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
Rule::Out( ListExpr typeInfo, Word value )
{
  Rule* rule = static_cast<Rule*>( value.addr );
  NList element ( rule->GetText(), true, true );

  return element.listExpr();
}



Word
Rule::Create( const ListExpr typeInfo )
{
  return (SetWord( new Rule( "" ) ));
}
 
 
void
Rule::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Rule*>( w.addr );
  w.addr = 0;
}

void
Rule::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Rule*>( w.addr );
  w.addr = 0;
}

Word
Rule::Clone( const ListExpr typeInfo, const Word& w )
{
  Rule* rule = static_cast<Rule*>( w.addr );
  return SetWord( new Rule(*rule) );
}

int
Rule::SizeOfObj()
{
  return sizeof(Rule);
}

bool
Rule::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Rule::BasicType() ));
}



ListExpr
Rule::Property()
{
  return (nl->TwoElemList(
    nl->FiveElemList(nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List"),
      nl->StringAtom("Remarks")),
    nl->FiveElemList(nl->StringAtom("-> DATA"),
      nl->StringAtom(Rule::BasicType()),
      nl->StringAtom("<rule>"),
      nl->TextAtom("\' X (_ a) // X.start = 01.01.2011 => X // X.label = b \'"),
      nl->StringAtom("<rule> must be a text."))));
}


TypeConstructor ruleTC(
  Rule::BasicType(),                          // name of the type in SECONDO
  Rule::Property,                // property function describing signature
  Rule::Out, Rule::In,         // Out and In functions
  0, 0,                            // SaveToList, RestoreFromList functions
  Rule::Create, Rule::Delete,  // object creation and deletion
  0, 0,                            // object open, save
  Rule::Close, Rule::Clone,    // close, and clone
  0,                               // cast function
  Rule::SizeOfObj,               // sizeof function
  Rule::KindCheck );             // kind checking function


/*
vector<SinglePattern> Rule::getRule()
{
  return patParser.getRule(); 
}

*/



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
    meaning   = "Creates a Pattern from a Text.";
  }

};


//---------------------------------------------------------------------------


ListExpr
consumeMLabelsTypeMap( ListExpr args )
{
  NList type(args);

  if (type.first() == NList(Stream<MLabel>::BasicType(), MLabel::BasicType())){
     return NList(MLabel::BasicType()).listExpr();     
  }  

  return NList::typeError("Expecting a stream of mlabels");
}


int
consumeMLabelsFun (Word* args, Word& result, int message,
              Word& local, Supplier s)
{
  qp->Open(args[0].addr); // open the argument stream

  MLabel* mlabel = new MLabel();  
  Stream<MLabel> stream(args[0]);
  stream.open();

  MLabel* next = stream.request();
  ULabel ul;
  
cout << "cons1" << endl;  

    mlabel = new MLabel(*next);


  while(next != 0){

cout << "cons2" << endl;    
    


  
    
    for (int j = 0; j < next->GetNoComponents(); ++j) {
      
cout << "cons3" << endl;      
      next->Get(j, ul);
      
//cout << ul.GetValue() << endl;      

//      mlabel->Add(ul);
    }  

cout << "cons4" << endl;

    next->DeleteIfAllowed();
    next = stream.request();
  }
  stream.close();

cout << "cons5" << endl;  
  
  // Assign a value to the operations result object which is provided
  // by the query processor
  result = qp->ResultStorage(s);
//  static_cast<MLabel*>(result.addr)->Add( mlabel );
  result.addr = mlabel;
  
  return 0;  
}


struct consumeMLabelsInfo : OperatorInfo {

  consumeMLabelsInfo()
  {
    name      = "consumemlabelstream";
    signature = " stream(mlabel) -> " + MLabel::BasicType();
    syntax    = "_ consumemlabelstream";
    meaning   = "Creates one MLabel from a stream of MLabel.";
  }

};

//---------------------------------------------------------------------------


ListExpr
applyTypeMap( ListExpr args )
{
  NList type(args);
  const string errMsg = "Expecting a mlabel and a rule "
                "or a mlabel and a text";
 
  // first alternative: mlabel x rule -> stream(mlabel)
  if ( type == NList(MLabel::BasicType(), Rule::BasicType()) ) {
    return NList(Stream<MLabel>::BasicType(), MLabel::BasicType()).listExpr();
//    return NList(Stream<MLabel>::BasicType()).listExpr();    
  }

  // second alternative: mlabel x text -> stream(mlabel)
  if ( type == NList(MLabel::BasicType(), FText::BasicType()) ) {  
//    return NList(Stream<MLabel>::BasicType()).listExpr();
    return NList(Stream<MLabel>::BasicType(), MLabel::BasicType()).listExpr();
  }

  return NList::typeError(errMsg);
}


int
applySelect( ListExpr args )
{
  NList type(args);
cout << "select" << endl;  
  
  if ( type.second().isSymbol( Rule::BasicType() ) ) {
cout << "select2" << endl;     
    return 1;
  }  
  else {
cout << "select3" << endl;     
    return 0;
  }  
}


int
applyFun_MP (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  
cout << "apply1" << endl;  
  
  struct Container {
    int pos;
    MLabel *mlabel;
    Rule *rule;

    Container(int position, MLabel *mlabel, Rule *rule) {

      // Do a proper initialization even if one of the
      // arguments has an undefined value
      if (mlabel->IsDefined() && rule->IsDefined())
      {
        pos = 0;
Container::mlabel = mlabel;
Container::rule = rule;
      }
      else
      {
// this initialization will create an empty stream
        pos = -1;
      }
    }
  };

cout << "apply1b" << endl;  
  Container* container = static_cast<Container*>(local.addr);
cout << "apply1c" << endl;
  
  switch( message )
  {
    case OPEN: { // initialize the local storage

cout << "open1" << endl;
  
      MLabel* mlabel = static_cast<MLabel*>( args[0].addr );
      Rule* rule = static_cast<Rule*>( args[1].addr );      
      container = new Container(0, mlabel, rule);
      local.addr = container;

cout << "open2" << endl;       
      
      return 0;
    }
    case REQUEST: { // return the next stream element


cout << "req1" << endl;

      if ( container->pos != -1 )
      {
container->pos = -1;
        MLabel elem(*container->mlabel);
        result.addr = &elem;
//        result.addr = &container->mlabel;

cout << "req2" << endl;
        return YIELD;
      }
      else
      {
cout << "req3" << endl;

result.addr = 0;
        return CANCEL;
      }
      
    }
    case CLOSE: { // free the local storage

cout << "clos1" << endl;

      if (container != 0) {
        delete container;
        local.addr = 0;
      }
      
cout << "clos2" << endl;

      return 0;
    }
    default: {
      /* should never happen */
      return -1;
    }
  }   
}


int
applyFun_MT (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  struct Container {
    int pos;
    MLabel mlabel;
    Rule rule;

    Container(int position, MLabel *mlabel, Rule *rule) {

      // Do a proper initialization even if one of the
      // arguments has an undefined value
      if (mlabel->IsDefined() && rule->IsDefined())
      {
        pos = 0;
Container::mlabel = *mlabel;
Container::rule = *rule;
      }
      else
      {
// this initialization will create an empty stream
        pos = -1;
      }
    }
  };

  Container* container = static_cast<Container*>(local.addr);

  switch( message )
  {
    case OPEN: { // initialize the local storage
  
      MLabel* mlabel = static_cast<MLabel*>( args[0].addr );
      FText* ruleText = static_cast<FText*>( args[1].addr );
      Rule rule( ruleText->GetValue() );       
      container = new Container(0, mlabel, &rule);
      local.addr = container;

      return 0;
    }
    case REQUEST: { // return the next stream element

      if ( container->pos != -1 )
      {
container->pos = -1;
        MLabel* elem = &container->mlabel;
//        MLabel* elem = new MLabel();
        result.addr = elem;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: { // free the local storage

      if (container != 0) {
        delete container;
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


struct applyInfo : OperatorInfo {

  applyInfo()
  {
    name      = "apply";

    signature = MLabel::BasicType() + " x " + Rule::BasicType()
                + " -> stream(mlabel)";

    appendSignature( MLabel::BasicType() + " x Text -> stream(mlabel)" );
    syntax    = " apply (_, _)";
    meaning   = "Creates a stream of MLabels values applying the rule.";
  }
};


//---------------------------------------------------------------------------

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

  bool res = (pattern->TotalMatch(*mlabel));

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

  if (!pattern.isValid()) {
    b->SetDefined(false);
    cerr << pattern.getErrMsg() << endl;
    return 0;
  }

  bool res = (pattern.TotalMatch(*mlabel));

  b->Set(true, res); //the first argument says the boolean
                     //value is defined, the second is the
                     //real boolean value)
  
  if (!pattern.isValid()) {
    b->SetDefined(false);
    cerr << pattern.getErrMsg() << endl;
  }
  
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
    appendSignature(MLabel::BasicType() + " x Text -> " + CcBool::BasicType());
    syntax    = "_ matches _";
    meaning   = "Match predicate.";
  }
};

//--------------------------------------------------------------------------

ListExpr
sintstreamType( ListExpr args ) {
  string err = "int x int expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!listutils::isSymbol(nl->First(args)) ||
     !listutils::isSymbol(nl->Second(args))){
    return listutils::typeError(err);
  }  
  return nl->TwoElemList(nl->SymbolAtom(Stream<CcInt>::BasicType()),
                         nl->SymbolAtom(CcInt::BasicType()));
}


int
sintstreamFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  // An auxiliary type which keeps the state of this
  // operation during two requests
  struct Range {
    int current;
    int last;

    Range(CcInt* i1, CcInt* i2) {

      // Do a proper initialization even if one of the
      // arguments has an undefined value
      if (i1->IsDefined() && i2->IsDefined())
      {
        current = i1->GetIntval();
        last = i2->GetIntval();
      }
      else
      {
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

      if ( range->current <= range->last )
      {
        CcInt* elem = new CcInt(true, range->current++);
        result.addr = elem;
        return YIELD;
      }
      else
      {
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
     AddTypeConstructor( &ruleTC );
     
     // 5.3 Registration of Operators
    AddOperator( &temporalatinstantext );
     
    AddOperator( patternInfo(), patternFun, textToPatternMap );     
    
    ValueMapping matchesFuns[] = { matchesFun_MT, matchesFun_MP, 0 };    
    AddOperator( matchesInfo(), matchesFuns, matchesSelect, matchesTypeMap );
    
//    AddOperator( consumeMLabelsInfo(), consumeMLabelsFun,
//      consumeMLabelsTypeMap );     
    
//    ValueMapping applyFuns[] = { applyFun_MT, applyFun_MP, 0 }; 
//    AddOperator( applyInfo(), applyFuns, applySelect, applyTypeMap );
        
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


