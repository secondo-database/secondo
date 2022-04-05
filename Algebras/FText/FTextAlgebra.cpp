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

//[_] [\_]

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] FText Algebra

March - April 2003 Lothar Sowada

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

April 2006, M. Spiekermann. Output format of type text changed!
Now it will be only a text atom instead of a one element list 
containing a text atom.

The algebra ~FText~ provides the type constructor ~text~ and two operators:

(i) ~contains~, which search text or string in a text.

(ii) ~length~ which give back the length of a text.


March 2008, Christian D[ue]ntgen added operators ~getcatalog~, $+$, ~substr~,
~subtext~, ~isempty~, $<$, $<=$, $=$, $>=$, $>$, \#, ~find~, ~evaluate~,
~getTypeNL~, ~getValueNL~, ~replace~, ~tostring~, ~totext~, ~tolower~,
~toupper~, ~chartext~.

October 2008, Christian D[ue]ntgen added operators ~sendtextUDP~ and
~receivetextUDP~

1 Preliminaries

1.1 Includes

*/

#include "RegExPattern.h"
#include "RegExPattern2.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <functional>
#include <sstream>
#include <cctype>

#include <queue>

#ifdef RECODE
#include <recode.h>
#endif



#include "FTextAlgebra.h"
#include "Attribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "SecondoInterface.h"
#include "DerivedObj.h"
#include "NList.h"
#include "DiceCoeff.h"
#include "SecParser.h"
#include "StopWatch.h"
#include "Symbols.h"
#include "SecondoSMI.h"
#include "SocketIO.h"
#include "Crypt.h"
#include "ListUtils.h"
#include "blowfish.h"
#include "md5.h"
#include "StringUtils.h"
#include "SecMath.h" 
#include <time.h>
#include <sys/timeb.h>
#include <limits>
#include "Stream.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "PinyinTable.h"
#include "DisplayTTY.h"
#include "SecParser.h"


#include "Profiles.h"

// includes for debugging
#define LOGMSG_OFF
#include "LogMsg.h"
#undef LOGMSG
#define LOGMSG(a, b)

// Declaration of external (global) objects
extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;
extern SecondoInterface* si;

using namespace std;

/*
2 Type Constructor ~text~

2.1 Data Structure - Class ~FText~

*/
size_t FText::HashValue() const
{
  if(traces)
    cout << '\n' << "Start HashValue" << '\n';

  if(!IsDefined())
    return 0;

  unsigned long h = 0;
  // Assuming text fits into memory
  char* s1 = Get();
  char*s = s1;
  while(*s != 0)
  {
    h = 5 * h + *s;
    s++;
  }
  delete[] s1;
  return size_t(h);
}

void FText::CopyFrom( const Attribute* right )
{
  if(traces)
    cout << '\n' << "Start CopyFrom" << '\n';

  const FText* r = (const FText*)right;
  string val = "";
  if(r->IsDefined())
  {
    val = r->GetValue();
  }
  Set( r->IsDefined(), val );
}

int FText::Compare( const Attribute *arg ) const
{
  if(traces)
    cout << '\n' << "Start Compare" << '\n';

  if(!IsDefined() && !arg->IsDefined())
    return 0;

  if(!IsDefined())
    return -1;

  if(!arg->IsDefined())
    return 1;

  const FText* f = (const FText* )(arg);

  if ( !f )
    return -2;

  char* s1 =  f->Get();
  char* s2 =  this->Get();

  int res = strcmp( s2, s1 );
  delete [] s1;
  delete [] s2;
  return res;
}

ostream& FText::Print(ostream &os) const
{
  if(!IsDefined())
  {
    return (os << "TEXT: UNDEFINED");
  }
  char* t = Get();
  string s(t);
  delete [] t;
  SHOW(theText)
  size_t len = theText.getSize();
  if(TextLength() > 65)
  {
    return (os << "TEXT: (" <<len <<") '" << s.substr(0,60) << " ... '" );
  }
  return (os << "TEXT: (" <<len << ") '" << s << "'" );
}


bool FText::Adjacent(const Attribute *arg) const
{
  if(traces)
    cout << '\n' << "Start Adjacent" << '\n';

  const FText* farg = static_cast<const FText*>(arg);

  char a[ theText.getSize() ];
  char b[ farg->theText.getSize() ];

  theText.read(a, theText.getSize() );
  farg->theText.read(b, farg->theText.getSize() );

  if( strcmp( a, b ) == 0 )
    return true;

  if( strlen( a ) == strlen( b ) )
  {
    if( strncmp( a, b, strlen( a ) - 1 ) == 0 )
    {
      char cha = (a)[strlen(a)-1],
           chb = (b)[strlen(b)-1];
      return( cha == chb + 1 || chb == cha + 1 );
    }
  }
  else if( strlen( a ) == strlen( b ) + 1 )
  {
    return( strncmp( a, b, strlen( b ) ) == 0 &&
            ( (a)[strlen(a)-1] == 'a' || (a)[strlen(a)-1] == 'A' ) );
  }
  else if( strlen( a ) + 1 == strlen( b ) )
  {
    return( strncmp( a, b, strlen( a ) ) == 0 &&
            ( (b)[strlen(b)-1] == 'a' || (b)[strlen(b)-1] == 'A' ) );
  }

  return false;
}

// This function writes the object value to a string ~dest~.
// The key must be shorter than SMI_MAX_KEYLEN and is 0-terminated
// Needed to create an index key
void FText::WriteTo( char *dest ) const
{
  string str = GetValue().substr(0, SMI_MAX_KEYLEN-2);
  string::size_type length = str.length();
  str.copy(dest,string::npos);
  dest[length] = 0;
}

// This function reads the object value from a string ~src~.
// Needed to create a text object from an index key
void FText::ReadFrom( const char *src )
{
  string myStr(src);
  Set( true, myStr);
}

// This function returns the number of bytes of the object's string
//   representation.
// Needed for transformation to/from an index key
SmiSize FText::SizeOfChars() const
{
  return (SmiSize) (GetValue().substr(0, SMI_MAX_KEYLEN-2).length()+1);
}


namespace ftext{

/*

2.3 Functions for using ~text~ in tuple definitions

The following Functions must be defined if we want to use ~text~ as an
attribute type in tuple definitions.

*/

Word CreateFText( const ListExpr typeInfo )
{
  return (SetWord(new FText( false )));
}

void DeleteFText( const ListExpr typeInfo, Word& w )
{
  if(traces)
    cout << '\n' << "Start DeleteFText" << '\n';
  FText *f = (FText *)w.addr;

  f->Destroy();
  delete f;
  w.addr = 0;
}


/*
2.8 ~Open~-function

*/
bool OpenFText( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
  FText *ft =
    (FText*)Attribute::Open( valueRecord, offset, typeInfo );
  value.setAddr( ft );
  return true;
}

/*
2.9 ~Save~-function

*/
bool SaveFText( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  FText *ft = (FText *)value.addr;

  // This Save function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to save objects
  Attribute::Save( valueRecord, offset, typeInfo, ft );
  return true;
}


void CloseFText( const ListExpr typeInfo, Word& w )
{
  if(traces)
    cout << '\n' << "Start CloseFText" << '\n';
  delete (FText*) w.addr;
  w.addr = 0;
}

Word CloneFText( const ListExpr typeInfo, const Word& w )
{
  if(traces)
    cout << '\n' << "Start CloneFText" << '\n';
  return SetWord( ((FText *)w.addr)->Clone() );
}

int SizeOfFText()
{
  return sizeof( FText );
}

void* CastFText( void* addr )
{
  if(traces)
    cout << '\n' << "Start CastFText" << '\n';
  return (new (addr) FText);
}


/*

2.4 ~In~ and ~Out~ Functions

*/

ListExpr OutFText( ListExpr typeInfo, Word value )
{
  if(traces)
    cout << '\n' << "Start OutFText" << '\n';
  FText* pftext;
  pftext = (FText*)(value.addr);

  if(traces)
    cout <<"pftext->Get()='"<< pftext->GetValue() <<"'\n";

  ListExpr res;
  if(pftext->IsDefined()){
     res=nl->TextAtom(pftext->GetValue());
  } else {
     res = nl->SymbolAtom(Symbol::UNDEFINED());
  }
  //nl->AppendText( TextAtomVar, pftext->Get() );

  if(traces)
    cout <<"End OutFText" << '\n';
  return res;
}


Word InFText( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(traces)
    cout << '\n' << "Start InFText with ListLength "
         << nl->ListLength( instance );
  ListExpr First;
  if (nl->ListLength( instance ) == 1)
    First = nl->First(instance);
  else
    First = instance;

  if ( nl->IsAtom( First ) && nl->AtomType( First ) == SymbolType
       && listutils::isSymbolUndefined(First) )
  {
    string buffer = "";
    FText* newftext = new FText( false, buffer.c_str() );
    correct = true;

    if(traces)
      cout << "End InFText with undefined Text '"<<buffer<<"'\n";
    return SetWord(newftext);
  }

  if ( nl->IsAtom(First) && nl->AtomType(First) == TextType)
  {
    string buffer;
    nl->Text2String(First, buffer);
    FText* newftext = new FText( true, buffer.c_str() );
    correct = true;

    if(traces)
      cout << "End InFText with Text '"<<buffer<<"'\n";
    return SetWord(newftext);
  }

  correct = false;
  if(traces)
    cout << "End InFText with errors!" << '\n';
  return SetWord(Address(0));
}


/*
2.5 Function describing the signature of the type constructor

*/

ListExpr FTextProperty()
{
  return
  (
  nl->TwoElemList
    (
      nl->FourElemList
      (
        nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List")
      ),
      nl->FourElemList
      (
        nl->StringAtom("-> DATA\n-> INDEXABLE"),
        nl->StringAtom(FText::BasicType()),
        nl->StringAtom("<text>writtentext</text--->"),
        nl->StringAtom("<text>This is an example.</text--->")
      )
    )
  );
}


ListExpr SVGProperty()
{
  return
  (
  nl->TwoElemList
    (
      nl->FourElemList
      (
        nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List")
      ),
      nl->FourElemList
      (
        nl->StringAtom("-> DATA"),
        nl->StringAtom(SVG::BasicType()),
        nl->StringAtom("<text>svg description</text--->"),
        nl->StringAtom("<text><svg> ... </svg></text--->")
      )
    )
  );
}


/*
2.6 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/

bool CheckFText( ListExpr type, ListExpr& errorInfo )
{
  if(traces)
    cout <<'\n'<<"Start CheckFText"<<'\n';
  bool bresult=(nl->IsEqual( type, typeName ));
  if(traces)
  {
    if(bresult)
      cout <<"End CheckFText with type=='"<<typeName<<"'"<<'\n';
    else
      cout <<"End CheckFText with type!='"<<typeName<<"'"<<'\n';
  }
  return bresult;
}


bool CheckSVG( ListExpr type, ListExpr& errorInfo )
{
  return nl->IsEqual( type, SVG::BasicType());
}



/*

2.7 Creation of the type constructor instances

*/

TypeConstructor ftext(
  typeName,                     //name of the type
  FTextProperty,                //property function describing signature
  OutFText,    InFText,         //Out and In functions
  0,           0,               //SaveToList and RestoreFromList functions
  CreateFText, DeleteFText,     //object creation and deletion
  OpenFText, SaveFText,
  CloseFText, CloneFText,       //close, and clone
  CastFText,                    //cast function
  SizeOfFText,                  //sizeof function
  CheckFText );                 //kind checking function

TypeConstructor svg(
  SVG::BasicType(),                     //name of the type
  SVGProperty,                //property function describing signature
  OutFText,    InFText,         //Out and In functions
  0,           0,               //SaveToList and RestoreFromList functions
  CreateFText, DeleteFText,     //object creation and deletion
  OpenFText, SaveFText,
  CloseFText, CloneFText,       //close, and clone
  CastFText,                    //cast function
  SizeOfFText,                  //sizeof function
  CheckSVG );                 //kind checking function


GenTC<RegExPattern> regexPattern;


TypeConstructor regexPattern2(
  RegExPattern2::BasicType(),   //name of the type
  RegExPattern2::Property,      //property function describing signature
  RegExPattern2::Out,           // out function
  RegExPattern2::In,           //In function
  0,           0,               //SaveToList and RestoreFromList functions
  RegExPattern2::Create,     //object creation and deletion
  RegExPattern2::Delete,
  RegExPattern2::Open,
  RegExPattern2::Save,
  RegExPattern2::Close,
  RegExPattern2::Clone,       //close, and clone
  RegExPattern2::Cast,                    //cast function
  RegExPattern2::SizeOf,                  //sizeof function
  RegExPattern2::Check );                 //kind checking function



/*

3 Creating Operators

3.1 Type Mapping Functions

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol ~typeerror~.

*/

/*
Typemapping text x text [->] bool

*/
ListExpr TypeMap_Text_Text__Bool( ListExpr args )
{
  if(traces)
  {
    cout <<'\n'<<"Start TypeMap_Text_Text__Bool"<<'\n';
    nl->WriteToFile("/dev/tty",args);
  }
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, typeName) && nl->IsEqual(arg2, typeName) )
    {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }

  if(traces)
    cout <<"End TypeMap_Text_Text__Bool with typeerror"<<'\n';
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
Typemapping {text|string} x {text|string} [->] bool

*/
ListExpr TypeMap_TextString_TextString__Bool( ListExpr args )
{
  if(nl->ListLength(args)!=2){
    return listutils::typeError("Expected 2 arguments.");
  }
  ListExpr arg1 = nl->First(args);
  if(   !listutils::isSymbol(arg1,FText::BasicType())
     && !listutils::isSymbol(arg1,CcString::BasicType())){
    return listutils::typeError("{text|string} x {text|string} expected.");
  }
  ListExpr arg2 = nl->Second(args);
  if(   !listutils::isSymbol(arg2,FText::BasicType())
     && !listutils::isSymbol(arg2,CcString::BasicType())){
      return listutils::typeError("{text|string} x {text|string} expected.");
    }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
Typemapping: text x string [->] bool

*/
ListExpr TypeMap_Text_String__Bool( ListExpr args )
{
  if(traces)
    {
    cout <<'\n'<<"Start TypeMap_Text_String__Bool"<<'\n';
    nl->WriteToFile("/dev/tty",args);
    }
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, typeName)
      && nl->IsEqual(arg2, CcString::BasicType()) )
    {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }

  if(traces)
    cout <<"End TypeMap_Text_String__Bool with typeerror"<<'\n';
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
Typemapping text [->] int

*/

ListExpr TypeMap_Text__Int( ListExpr args )
{
  if(traces)
    cout << '\n' << "Start TypeMap_Text__Int" << '\n';
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1= nl->First(args);
    if ( nl->IsEqual(arg1, typeName))
    {
      if(traces)
        cout << '\n' << "Start TypeMap_Text__Int" << '\n';
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  if(traces)
    cout <<"End TypeMap_Text__Int with typeerror"<<'\n';
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
Typemap: text [->] stream(string)

*/

ListExpr TypeMap_text__stringstream( ListExpr args ){
  ListExpr arg;
  string nlchrs;

  if ( nl->ListLength(args) == 1 )
  {
    arg = nl->First(args);
    if ( nl->IsEqual(arg, typeName) )
      return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                              nl->SymbolAtom(CcString::BasicType()));
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
Typemap: text [->] stream(text)

*/
ListExpr TypeMap_text__textstream( ListExpr args ){
  ListExpr arg;

  if ( nl->ListLength(args) == 1 )
  {
    arg = nl->First(args);
    if ( nl->IsEqual(arg, typeName) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(FText::BasicType()));
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
Typemap: int x text x text [->] real

*/
ListExpr TypeMap_int_text_text__real(ListExpr args){
  if(nl->ListLength(args)!=3){
       ErrorReporter::ReportError("three arguments required");
       return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if(!nl->IsEqual(nl->First(args),CcInt::BasicType())){
     ErrorReporter::ReportError("first argument must be an integer");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  if(  (nl->AtomType(arg2)!=SymbolType ) ||
       (nl->AtomType(arg3)!=SymbolType)){
     ErrorReporter::ReportError("only simple types allowed");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  string t2 = nl->SymbolValue(arg2);
  string t3 = nl->SymbolValue(arg3);

  // first version, only texts, later extend to string
  if(t2!=FText::BasicType() || t3!=FText::BasicType()){
     ErrorReporter::ReportError("text as second and third argument expected");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  return nl->SymbolAtom(CcReal::BasicType());
}


/*

Type Mapping for operator ~getCatalog~

*/
ListExpr TypeGetCatalog( ListExpr args )
{
  NList type(args);

  if ( type.hasLength(0) ){
    NList resTupleType = NList(NList("ObjectName"),
                       NList(CcString::BasicType())).enclose();
    resTupleType.append(NList(NList("Type"),NList(FText::BasicType())));
    resTupleType.append(NList(NList("TypeExpr"),NList(FText::BasicType())));
    NList resType =
        NList(NList(NList(Symbol::STREAM()),
            NList(NList(Tuple::BasicType()),resTupleType)));
    return resType.listExpr();
  }
  return NList::typeError( "No argument expected!");
}

/*
Type Mapping for operators ~substr~: {text|string} x int x int [->] string

*/
ListExpr TypeFTextSubstr( ListExpr args )
{
  NList type(args);

  if ( !type.hasLength(3) ){
    return NList::typeError( "Three arguments expected");
  }
  if (    type.second() != NList(CcInt::BasicType())
       || type.third()  != NList(CcInt::BasicType())
     )
  {
    return NList::typeError( "Boundary arguments must be of type int.");
  }
  if ( type.first() == NList(FText::BasicType()) )
  {
    return NList(CcString::BasicType()).listExpr();
  }
  return NList::typeError( "Expected text as first argument type.");
}

/*

Type Mapping for operators ~subtext~: text x int x int [->] text

*/
ListExpr TypeFTextSubtext( ListExpr args )
{
  NList type(args);

  if ( !type.hasLength(3) ){
    return NList::typeError( "Three arguments expected");
  }
  if (    type.second() != NList(CcInt::BasicType())
          || type.third()  != NList(CcInt::BasicType())
     )
  {
    return NList::typeError( "Boundary arguments must be of type int.");
  }
  if ( type.first() == NList(FText::BasicType()) )
  {
    return NList(FText::BasicType()).listExpr();
  }
  return NList::typeError( "Expected text as first argument type.");
}

/*

Type Mapping for operator ~find~: {string|text} x {string|text} [->] stream(int)

*/
ListExpr TypeMap_textstring_textstring__intstream( ListExpr args )
{
  NList type(args);

  if ( type.hasLength(2) &&
       (
          (type == NList(CcString::BasicType(), CcString::BasicType()))
       || (type == NList(FText::BasicType(),   FText::BasicType()  ))
       || (type == NList(CcString::BasicType(), FText::BasicType()  ))
       || (type == NList(FText::BasicType(),   CcString::BasicType()))
       )
     )
  {
    return NList(Symbol::STREAM(), CcInt::BasicType()).listExpr();
  }
  return NList::typeError("Expected {text|string} x {text|string}.");
}


/*

Type Mapping: text [->] bool

*/
ListExpr TypeMap_text__bool( ListExpr args )
{
  NList type(args);

  if ( type.hasLength(1)
       &&( (type.first() == NList(FText::BasicType())) )
     )
  {
    return NList(CcBool::BasicType()).listExpr();
  }
  return NList::typeError("Expected single text argument.");
}

/*

Type Mapping:  text [->] text

*/
ListExpr TypeMap_text__text( ListExpr args )
{
  if(nl->ListLength(args)!=1){
    return listutils::typeError("single text expected");
  }
  ListExpr arg = nl->First(args);
  if(!listutils::isSymbol(arg,FText::BasicType())){
    return listutils::typeError("text expected");
  }
  return nl->SymbolAtom(FText::BasicType());
}

/*

Type Mapping Function for operator ~+~

----
        text x {text | string} --> text
        {text | string} x text --> text
----

*/
ListExpr FTextTypeMapPlus( ListExpr args )
{
  NList type(args);

  if ( !type.hasLength(2) )
  {
    return NList::typeError("Expected two arguments.");
  }
  NList first = type.first();
  NList second = type.second();
  if(    (type == NList(CcString::BasicType(), FText::BasicType()  ))
      || (type == NList(FText::BasicType(),   FText::BasicType()  ))
      || (type == NList(FText::BasicType(),   CcString::BasicType()))
    )
  {
    return NList(FText::BasicType()).listExpr();
  }
  return NList::typeError("Expected (text x {text|string}) "
      "or ({text|string} x text).");
}



/*
Type Mapping Function for comparison predicates 
~$<$, $<=$, $=$, $>=$, $>$, $\neq$~

----
    <, <=, =, >=, >, #: {string|text} x {string|text} --> bool
----

*/
ListExpr FTextTypeMapComparePred( ListExpr args )
{
  NList type(args);

  if ( !type.hasLength(2) )
  {
    return NList::typeError("Expected two arguments.");
  }
  NList first = type.first();
  NList second = type.second();
  if(    (type == NList(CcString::BasicType(), FText::BasicType()  ))
          || (type == NList(FText::BasicType(),   FText::BasicType()  ))
          || (type == NList(FText::BasicType(),   CcString::BasicType()))
    )
  {
    return NList(CcBool::BasicType()).listExpr();
  }
  return NList::typeError("Expected (text x {text|string}) "
      "or ({text|string} x text).");
}

/*
Type Mapping Function for operator ~evaluate~

----
    text -> stream(tuple((CmdStr text)       // copy of the evaluated command
                         (Success bool)      // TRUE iff execution succeded
                         (ResultType text)      // result type expression
                         (Result text)          // result as nested list expr
                         (ErrorMessage text)    // Error messages
                         (ElapsedTimeReal real) // The execution time in sec
                         (ElapsedTimeCPU real)  // The CPU time in sec
                        )
                  )
----

*/
ListExpr FTextTypeMapEvaluate( ListExpr args )
{
  NList type(args);
  NList st(Symbol::STREAM());
  NList tu(Tuple::BasicType());
  NList resTupleType = NList(NList("CmdStr"),
                             NList(FText::BasicType())).enclose();
  resTupleType.append(NList(NList("Success"),NList(CcBool::BasicType())));
  resTupleType.append(NList(NList("Correct"),NList(CcBool::BasicType())));
  resTupleType.append(NList(NList("Evaluable"),NList(CcBool::BasicType())));
  resTupleType.append(NList(NList("Defined"),NList(CcBool::BasicType())));
  resTupleType.append(NList(NList("IsFunction"),NList(CcBool::BasicType())));
  resTupleType.append(NList(NList("ResultType"),NList(FText::BasicType())));
  resTupleType.append(NList(NList("Result"),NList(FText::BasicType())));
  resTupleType.append(NList(NList("ErrorMessage"),NList(FText::BasicType())));
  resTupleType.append(NList(NList("ElapsedTimeReal"),
                            NList(CcReal::BasicType())));
  resTupleType.append(NList(NList("ElapsedTimeCPU"),
                            NList(CcReal::BasicType())));

  NList resulttype(st,NList(tu,resTupleType));

  if (    type.hasLength(2)
       && (type.first()  == FText::BasicType())
       && (type.second() == CcBool::BasicType())
     )
  {
    return resulttype.listExpr();
  }
  else if(    type.hasLength(1)
           && (type.first() == FText::BasicType())
         )
  {
    NList resType1 =
        NList( NList(Symbol::APPEND()),
               NList(false, false).enclose(), resulttype );
    return resType1.listExpr();
  }
  else
  {
    return NList::typeError("Expected 'text' as first, and 'bool' as "
        "optional second argument.");
  }
}

/*
Type Mapping Function for operator ~FTextTypeTextData\_Data~

----
      {text|string} x T --> T
----

*/

ListExpr FTextTypeTextData_Data( ListExpr args )
{
  NList type(args);
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  if(     type.hasLength(2)
       && ((type.first()  == FText::BasicType()) ||
       (type.first()  == CcString::BasicType()))
       && (am->CheckKind(Kind::DATA(),type.second().listExpr(),errorInfo))
    )
  {
    return type.second().listExpr();
  }
  // error
  return NList::typeError("Expected {text|string} x T, where T in DATA.");
}

/*
Type Mapping Function for operator ~replace~

----
      {text|string} x {text|string} x {text|string} --> text
      {text|string} x int    x int  x {text|string} --> text
----

*/

ListExpr FTextTypeReplace( ListExpr args )
{
  NList type(args);
  // {text|string} x {text|string} x {text|string} --> text
  if(     type.hasLength(3)
          && ((type.first()  == FText::BasicType()) ||
          (type.first()  == CcString::BasicType()))
          && ((type.second() == FText::BasicType()) ||
          (type.second() == CcString::BasicType()))
          && ((type.third()  == FText::BasicType()) ||
              (type.third()  == CcString::BasicType()))
    )
  {
    return NList(FText::BasicType()).listExpr();
  }
  // {text|string} x int    x int  x {text|string} --> text
  if(     type.hasLength(4)
          && ((type.first()  == FText::BasicType()) ||
          (type.first()  == CcString::BasicType()))
          && ((type.second() == CcInt::BasicType() ) ||
          (type.second() == CcInt::BasicType()   ))
          && ((type.third()  == CcInt::BasicType() ) ||
          (type.third()  == CcInt::BasicType()   ))
          && ((type.fourth() == FText::BasicType()) ||
          (type.fourth() == CcString::BasicType()))
    )
  {
    return NList(FText::BasicType()).listExpr();
  }
  // error
  return NList::typeError("Expected ({text|string} x {text|string} x "
      "{text|string}) or ({text|string} x int x int x {text|string}).");

}

/*
Type Mapping for ~isDBObject~:

---- string --> bool
----

*/
ListExpr TypeMap_string__bool( ListExpr args )
{
  NList type(args);
  if(type.hasLength(1) && (type.first()  == CcString::BasicType()))
  {

    NList restype(CcBool::BasicType(), false);
    return restype.listExpr();
  }
  return NList::typeError("Expected 'string' as single argument.");
}

/*
Type Mapping for ~getTypeNL~:

---- TypeExpr --> text @text
----

*/

ListExpr FTextTypeMapGetTypeNL( ListExpr args )
{
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  if(listutils::isSymbol(nl->First(args), Symbols::TYPEERROR())){
    return listutils::typeError("typeerror received");
  }
  //return listutils::basicSymbol<FText>();
  return nl->ThreeElemList( 
             nl->SymbolAtom(Symbols::APPEND()),
             nl->OneElemList(nl->TextAtom(nl->ToString(nl->First(args)))),
             listutils::basicSymbol<FText>()); 
}

/*
Type Mapping for ~getValueNL~:

----
     (stream TypeExpr) --> (stream text) @text
     Expr --> text @text
----

*/

ListExpr FTextTypeMapGetValueNL( ListExpr args )
{
  NList type(args);
  NList resulttype = NList(Symbols::TYPEERROR());
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  if( !type.hasLength(1) )
  { // too many arguments
    return NList::typeError("Expected any Expression as single argument.");
  }
  type = type.first();
  if( IsStreamDescription(type.listExpr()) )
  { // tuplestream
    string myType    = type.second().convertToString();
    NList streamtype = NList(Symbol::STREAM(), FText::BasicType());
    NList typeExpr   = NList(myType, true, true).enclose();
    resulttype = NList( NList(Symbol::APPEND()),
                        NList(myType, true, true).enclose(),
                        streamtype
                      );
  }
  else if (    type.hasLength(2)
            && (type.first() == Symbol::STREAM())
            && am->CheckKind(Kind::DATA(),type.second().listExpr(),errorInfo)
          )
  { // datastream
    string myType    = type.second().convertToString();
    NList streamtype = NList(Symbol::STREAM(), FText::BasicType());
    NList typeExpr   = NList(myType, true, true).enclose();
    resulttype = NList( NList(Symbol::APPEND()),
                        NList(myType, true, true).enclose(),
                        streamtype
                      );
  }
  else
  { // non-stream expression
    string myType = type.convertToString();
    NList typeExpr = NList(myType, true, true).enclose();
    resulttype = NList( NList(Symbol::APPEND()),
                        NList(myType, true, true).enclose(),
                        NList(FText::BasicType())
                      );
  }
//   cout << __PRETTY_FUNCTION__ << ": result = " << resulttype << endl;
  return resulttype.listExpr();
}

/*
Type Mapping Function for ~chartext~

----
     int --> text
----

*/

ListExpr TypeMap_int__text( ListExpr args )
{
  NList type(args);
  if( type.hasLength(1) && (type.first() == CcInt::BasicType()) )
  {
    return NList(FText::BasicType()).listExpr();
  }
  return NList::typeError("Expected 'int'.");
}


/*
Type Mapping Function for ~tostring~

----
     text --> string
----

*/

ListExpr TypeMap_text__string( ListExpr args )
{
  NList type(args);
  if( type.hasLength(1) && (type.first() == FText::BasicType()) )
  {
    return NList(CcString::BasicType()).listExpr();
  }
  return NList::typeError("Expected 'text'.");
}

/*
Type Mapping Function for ~totext~

----
      string --> text
----

*/

ListExpr TypeMap_string__text( ListExpr args )
{
  NList type(args);
  if( type.hasLength(1) && (type.first() == CcString::BasicType()) )
  {
    return NList(FText::BasicType()).listExpr();
  }
  return NList::typeError("Expected 'string'.");
}

/*
Type Mapping Function for ~sendtextUDP~

----
      {string|text}^n -> text, 3 <= n <= 5.
----

*/
ListExpr FTextTypeSendTextUDP( ListExpr args )
{
  NList type(args);
  int noargs = nl->ListLength(args);
  if(noargs < 3 || noargs > 5){
    return NList::typeError("Expected {string|text}^n, 3 <= n <= 5.");
  }

  for(int i = 1; i<=noargs; i++){
    string argtype;
    nl->WriteToString(argtype,nl->Nth(i,args));
    if((argtype != CcString::BasicType()) && (argtype != FText::BasicType())){
      return NList::typeError("Expected {string|text}^n, 3 <= n <= 5.");
    }
  }
  return NList(FText::BasicType()).listExpr();
}

/*
Type Mapping Function for ~receivetextUDP~

----
     {string|text} x {string|text} x real ->
     {stream(tuple((Ok bool)
                   (Msg text)
                   (ErrMsg string)
                   (SenderIP string)
                   (SenderPort string)
                   (SenderIPversion string)
                  )
            )

----

*/
ListExpr FTextTypeReceiveTextUDP( ListExpr args )
{
  NList type(args);
  int noargs = nl->ListLength(args);
  if(    (noargs != 3)
      || (type.first()  != FText::BasicType() &&
      type.first()  != CcString::BasicType())
      || (type.second() != FText::BasicType() &&
      type.second() != CcString::BasicType())
      || (type.third()  != CcReal::BasicType()) ){
    return NList::typeError("Expected {string|text} x {string|text} x real.");
  }
  NList resTupleType = NList(NList("Ok"),
                             NList(CcBool::BasicType())).enclose();
  resTupleType.append(NList(NList("Msg"),NList(FText::BasicType())));
  resTupleType.append(NList(NList("ErrMsg"),NList(CcString::BasicType())));
  resTupleType.append(NList(NList("SenderIP"),NList(CcString::BasicType())));
  resTupleType.append(NList(NList("SenderPort"),NList(CcString::BasicType())));
  resTupleType.append(NList(NList("SenderIPversion"),
                            NList(CcString::BasicType())));
  NList resType =
      NList(NList(NList(Symbol::STREAM()),
            NList(NList(Tuple::BasicType()),resTupleType)));
  return resType.listExpr();
}

/*
Type Mapping Function for ~receivetextUDP~

----
{string|text} x {string|text} x real x real->
{stream(tuple((Ok bool)
                   (Msg text)
                   (ErrMsg string)
                   (SenderIP string)
                   (SenderPort string)
                   (SenderIPversion string)
                  )
            )

----

*/
ListExpr FTextTypeReceiveTextStreamUDP( ListExpr args )
{
  NList type(args);
  int noargs = nl->ListLength(args);
  if(    (noargs !=4 )
      || (type.first()  != FText::BasicType() &&
      type.first()  != CcString::BasicType())
      || (type.second() != FText::BasicType() &&
      type.second() != CcString::BasicType())
      || (type.third()  != CcReal::BasicType())
      || (type.fourth() != CcReal::BasicType()) ) {
    return NList::typeError("Expected {string|text} x "
                            "{string|text} x real x real.");
  }
  NList resTupleType = NList(NList("Ok"),NList(CcBool::BasicType())).enclose();
  resTupleType.append(NList(NList("Msg"),NList(FText::BasicType())));
  resTupleType.append(NList(NList("ErrMsg"),NList(CcString::BasicType())));
  resTupleType.append(NList(NList("SenderIP"),NList(CcString::BasicType())));
  resTupleType.append(NList(NList("SenderPort"),NList(CcString::BasicType())));
  resTupleType.append(NList(NList("SenderIPversion"),
                            NList(CcString::BasicType())));
  NList resType =
      NList(NList(NList(Symbol::STREAM()),
            NList(NList(Tuple::BasicType()),resTupleType)));
  return resType.listExpr();
}


/*
2.50.1 ~TypeMap\_text\_\_svg~

*/
ListExpr TypeMap_text__svg(ListExpr args){
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("One argument expected");
      return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),FText::BasicType())){
      return nl->SymbolAtom(SVG::BasicType());
   }
   ErrorReporter::ReportError("text expected");
   return nl->TypeError();
}


/*
2.50.2 ~TypeMap\_svg\_\_text~

*/
ListExpr TypeMap_svg__text(ListExpr args){
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("One argument expected");
      return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),SVG::BasicType())){
      return nl->SymbolAtom(FText::BasicType());
   }
   ErrorReporter::ReportError("svg expected");
   return nl->TypeError();

}

/*
2.51 ~crypt~

---- t1 [x t2] -> string , t1, t2 in {string, text}
----

*/

ListExpr cryptTM(ListExpr args){
int l = nl->ListLength(args);

string err = "t1 [x t2]  , t1, t2 in {string, text} expected";
if((l!=1) && (l!=2)){
  ErrorReporter::ReportError(err);
  return nl->TypeError();
}
while(!nl->IsEmpty(args)){
  ListExpr first = nl->First(args);
  args = nl->Rest(args);
  if(nl->AtomType(first)!=SymbolType){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  string v = nl->SymbolValue(first);
  if( (v!=CcString::BasicType()) && (v!=FText::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
}
return nl->SymbolAtom(CcString::BasicType());
}

/*
checkpw: {string | text} x {string | text} [->] bool

*/
ListExpr checkpwTM(ListExpr args){
  string err = "{string, text} x {string, text} expected";
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  while(!nl->IsEmpty(args)){
    ListExpr first = nl->First(args);
    args = nl->Rest(args);
    if(nl->AtomType(first)!=SymbolType){
       ErrorReporter::ReportError(err);
       return nl->TypeError();
    }
    string v = nl->SymbolValue(first);
    if( (v!=CcString::BasicType()) && (v!=FText::BasicType())){
       ErrorReporter::ReportError(err);
       return nl->TypeError();
    }
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
2.53 ~md5~

---- t1 [x t2] -> text , t1, t2 in {string, text}
----

*/

ListExpr md5TM(ListExpr args){
int l = nl->ListLength(args);

string err = "t1 [x t2]  , t1, t2 in {string, text} expected";
if((l!=1) && (l!=2)){
  ErrorReporter::ReportError(err);
  return nl->TypeError();
}
while(!nl->IsEmpty(args)){
  ListExpr first = nl->First(args);
  args = nl->Rest(args);
  if(nl->AtomType(first)!=SymbolType){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  string v = nl->SymbolValue(first);
  if( (v!=CcString::BasicType()) && (v!=FText::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
}
return nl->SymbolAtom(CcString::BasicType());
}


/*
2.54 ~blowfish~

---- t1 x t2 -> text , t1, t2 in {string, text}
----

*/

ListExpr blowfish_encodeTM(ListExpr args){
int l = nl->ListLength(args);

string err = "t1 x t2, t1, t2 in {string, text} expected";
if(l!=2){
  ErrorReporter::ReportError(err);
  return nl->TypeError();
}
while(!nl->IsEmpty(args)){
  ListExpr first = nl->First(args);
  args = nl->Rest(args);
  if(nl->AtomType(first)!=SymbolType){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  string v = nl->SymbolValue(first);
  if( (v!=CcString::BasicType()) && (v!=FText::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
}
return nl->SymbolAtom(FText::BasicType());
}

ListExpr blowfish_decodeTM(ListExpr args){
   return blowfish_encodeTM(args);
}

/*
2.54 ~letObject~

---- {string|text} x {string|text} x bool --> text
----

*/

ListExpr StringtypeStringtypeBool2TextTM(ListExpr args){
  NList type(args);
  int noargs = nl->ListLength(args);
  if(    (noargs  != 3)
      || ((type.first() != CcString::BasicType()) &&
      (type.first() != FText::BasicType()))
      || ((type.second()!= CcString::BasicType()) &&
      (type.second()!= FText::BasicType()))
      || ( type.third() != CcBool::BasicType()) ) {
    return NList::typeError("Expected {string|text} x {string|text} x bool.");
  }
  return NList(FText::BasicType()).listExpr();
}


/*
2.55 ~getDatabaseName~: () [->] string

*/

ListExpr TypeMap_empty__string(ListExpr args){
  NList type(args);
  int noargs = nl->ListLength(args);
  if( noargs != 0 ) {
    return NList::typeError("Expected no argument.");
  }
  return NList(CcString::BasicType()).listExpr();
}


/*
2.55 ~TypeMap\_textstring\_\_text~

---- {text | string} --> text
----

Used by ~deleteObject~, ~getObjectValueNL~, ~getObjectTypeNL~.

*/

ListExpr TypeMap_textstring__text(ListExpr args){
  NList type(args);
  int noargs = nl->ListLength(args);
  if(    (noargs != 1)
      || ((type.first()!=CcString::BasicType()) &&
      (type.first() != FText::BasicType()))) {
    return NList::typeError("Expected {string|text}.");
  }
  return NList(FText::BasicType()).listExpr();
}


/*
2.56 TypeMap ~matchingOperatorsNames~

---- any -> stream(string)
----

*/

ListExpr matchingOperatorNamesTM(ListExpr args){
    ListExpr args2 = args;
    while(!nl->IsEmpty(args2)){
      if(listutils::isSymbol(nl->First(args2),Symbols::TYPEERROR())){
        return listutils::typeError("TypeError in ArgumentList");
      }
      args2 = nl->Rest(args2);
    }

    ListExpr res =  nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                    nl->SymbolAtom(CcString::BasicType()));
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
               nl->OneElemList(nl->TextAtom(nl->ToString(args))),
               res);
}

/*
2.57 TypeMap ~matchingOperators~

----
  ANY -> stream(tuple(( OperatorName: string,
                        OperatorId: int,
                        AlgebraName : string,
                        AlgebraId: int,
                        ResultType : text,
                        Signature  : text,
                        Syntax : text,
                        Meaning : text,
                        Example : text,
                        Remark : text))) @text
----

*/

ListExpr matchingOperatorsTM(ListExpr args){
    ListExpr args2 = args;
    while(!nl->IsEmpty(args2)){
      if(listutils::isSymbol(nl->First(args2),Symbols::TYPEERROR())){
        return listutils::typeError("TypeError in ArgumentList");
      }
      args2 = nl->Rest(args2);
    }

    ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                           nl->SymbolAtom("OperatorName"),
                           nl->SymbolAtom(CcString::BasicType())));
    ListExpr last = attrList;
    last = nl->Append(last, nl->TwoElemList(
            nl->SymbolAtom("OperatorId"),nl->SymbolAtom(CcInt::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("AlgebraName"),
                              nl->SymbolAtom(CcString::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                            nl->SymbolAtom("AlgebraId"),
                            nl->SymbolAtom(CcInt::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("ResultType"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Signature"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Syntax"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Meaning"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Example"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Remark"),
                              nl->SymbolAtom(FText::BasicType())));

    ListExpr res = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                     nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),
                                      attrList));
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                       nl->OneElemList(nl->TextAtom(nl->ToString(args))),
                             res);
}

/*
2.57 TypeMap ~sys\_getMatchingOperators~

----
  int x ANY -> R,
  {string|text} x int -> R,
     where R = stream(tuple(( OperatorName: string,
                              OperatorId: int,
                              AlgebraName : string,
                              AlgebraId: int,
                              ResultType : text,
                              Signature  : text,
                              Syntax : text,
                              Meaning : text,
                              Example : text,
                              Remark : text))) @text
----

*/
ListExpr sysgetMatchingOperatorsTM(ListExpr args){

    int noargs = nl->ListLength(args);
    string errmsg = "Expected (int x ...) or ({string|text} x int).";
    if( noargs < 1 ) {
      return NList::typeError(errmsg);
    }

    ListExpr first = nl->First(args);
    if(! listutils::isSymbol(first,CcInt::BasicType())){
      if(   !listutils::isSymbol(first,CcString::BasicType())
         && !listutils::isSymbol(first,FText::BasicType())){
        return listutils::typeError(errmsg);
      } else {
        if(    noargs!=2
            || !listutils::isSymbol(nl->Second(args),CcInt::BasicType())) {
          return listutils::typeError(errmsg);
        }
      }
    }

    ListExpr attrList = nl->OneElemList(nl->TwoElemList(
                           nl->SymbolAtom("OperatorName"),
                           nl->SymbolAtom(CcString::BasicType())));
    ListExpr last = attrList;
    last = nl->Append(last, nl->TwoElemList(
            nl->SymbolAtom("OperatorId"),nl->SymbolAtom(CcInt::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("AlgebraName"),
                              nl->SymbolAtom(CcString::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                            nl->SymbolAtom("AlgebraId"),
                            nl->SymbolAtom(CcInt::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("ResultType"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Signature"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Syntax"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Meaning"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Example"),
                              nl->SymbolAtom(FText::BasicType())));
    last = nl->Append(last, nl->TwoElemList(
                              nl->SymbolAtom("Remark"),
                              nl->SymbolAtom(FText::BasicType())));

    ListExpr res = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                     nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),
                                      attrList));
    if(listutils::isSymbol(first,CcInt::BasicType())) {
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                            nl->OneElemList(nl->TextAtom(nl->ToString(args))),
                            res);
    } else {
      return res;
    }
}

/*
2.58 ~int2stringTM~ TypeMap (e.g. Operator ~sys\_getAlgebraName~

---- int --> string
----

*/
ListExpr int2stringTM(ListExpr args){
  string errmsg = "Expected (int).";
  if(    (nl->ListLength(args)==1)
      && listutils::isSymbol(nl->First(args),CcInt::BasicType())
    ){
    return nl->SymbolAtom(CcString::BasicType());
  }
  return listutils::typeError(errmsg);
}

/*
2.58 ~stringORtext2intTM~ TypeMap (e.g. Operator ~sys\_getAlgebraId~

---- {string|text} --> int
----

*/
ListExpr stringORtext2intTM(ListExpr args){
  string errmsg = "Expected (string) or (text).";
  if(    (nl->ListLength(args)==1)
      && (    listutils::isSymbol(nl->First(args),CcString::BasicType())
           || listutils::isSymbol(nl->First(args),FText::BasicType())
         )
    ){
    return nl->SymbolAtom(CcInt::BasicType());
  }
  return listutils::typeError(errmsg);
}


/*
2.58 ~checkOperatorTypeMap~

----  string x any x any x any ... -> text
----

Checks whether an operator exists having the name given as the first 
argument and whether this
operator can applied to the remaining arguments. The result contains 
the result type of the
first applyable operator or is undefined if no such operator exists.

*/


ListExpr CheckOperatorTypeMapTM(ListExpr args){
   if(!nl->HasMinLength(args,1)){
      return listutils::typeError(" string x any x any x ... expected");
   }
   ListExpr first = nl->First(args);
   if(!listutils::isSymbol(first, CcString::BasicType())){
      return listutils::typeError(" string x any x any x ... expected");
   }
   ListExpr rest = nl->Rest(args);
   return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->TextAtom(nl->ToString(rest))),
                             nl->SymbolAtom(FText::BasicType()));
}


/*
2.58 ~checkOperatorTypeMap2~

----  string x text | string x string -> text
----

Checks whether an operator exists having the name given as the first 
argument and whether this
operator can be applied to the type given as a nested list within a 
string or text.
The result contains the result type of the first applyable operator or 
is undefined if no
such operator exists.

*/
ListExpr CheckOperatorTypeMap2TM(ListExpr args){
   if(!nl->HasLength(args,2)){
      return listutils::typeError(" string x {string | text} expected");
   }
   ListExpr first = nl->First(args);
   ListExpr second = nl->Second(args);
   if(!listutils::isSymbol(first, CcString::BasicType()) ||
      (!listutils::isSymbol(second,FText::BasicType())&&
       !listutils::isSymbol(second,CcString::BasicType()))){
      return listutils::typeError(" string x {string | text} expected");
   }
   return  nl->SymbolAtom(FText::BasicType());
}


/*
2.59 ~strequalTM~

This operator checks two strings (texts) for equality.
An additional boolean parameter constrols whether the
comparison should be case sensitive.

*/
ListExpr strequalTM(ListExpr args){
  string err = "{string, text} x {string,text} x bool expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  ListExpr third = nl->Third(args);

  if( !listutils::isSymbol(first,CcString::BasicType())  &&
      !listutils::isSymbol(first,FText::BasicType())){
      return listutils::typeError(err);
   }
  if( !listutils::isSymbol(second,CcString::BasicType())  &&
      !listutils::isSymbol(second,FText::BasicType())){
      return listutils::typeError(err);
   }

  if( !listutils::isSymbol(third,CcBool::BasicType())){
    return listutils::typeError(err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}



/*
2.60 ~tokenizeTM~

This operator splits a text at delimiters. The mapping is:

---- text x string -> stream(text)
----

*/
ListExpr tokenizeTM(ListExpr args){
   string err = "text x string expected";
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }

   if(!listutils::isSymbol(nl->First(args), FText::BasicType()) ||
      !listutils::isSymbol(nl->Second(args), CcString::BasicType())){
      return listutils::typeError(err);
   }
   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(FText::BasicType()));
}


/*
2.61 ~sendtextstreamTCP\_TM~

----
stream({string|text}) x {string|text} x {string|text} x {int|real} x {int|real}
    -> stream(tuple((Ok bool) (Msg text) (ErrMsg string)))
----

*/
ListExpr sendtextstreamTCP_TM (ListExpr args ){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected stream({string|text}) x {string|text} x "
                  "{string|text} x {int|real} x {int|real}.";
  if( (noargs!=5) ){
    return listutils::typeError(errmsg);
  }
  // check first argument: must be stream(text) or stream(string)
  ListExpr first = nl->First(args);
  if(!listutils::isDATAStream(first)){
    return listutils::typeError("Expected stream(text) or stream(string) as "
                                "1st argument.");
  }
  ListExpr elemtype = nl->Second(first);
  if(    !listutils::isSymbol(elemtype,CcString::BasicType())
      && !listutils::isSymbol(elemtype,FText::BasicType()) ){
    return listutils::typeError("Expected stream(text) or stream(string) as "
    "1st argument.");
  }
  ListExpr iptype = nl->Second(args);
  if(    !listutils::isSymbol(iptype,CcString::BasicType())
      && !listutils::isSymbol(iptype,FText::BasicType()) ){
    return listutils::typeError("Expected '"+CcString::BasicType()+"' or '"
                +FText::BasicType()+"' as 2nd argument.");
  }
  ListExpr porttype = nl->Third(args);
  if(    !listutils::isSymbol(porttype,CcString::BasicType())
      && !listutils::isSymbol(porttype,FText::BasicType()) ){
    return listutils::typeError("Expected '"+CcString::BasicType()+"' or '"
    +FText::BasicType()+"' as 3rd argument.");
  }
  ListExpr timeouttype = nl->Fourth(args);
  if(    !listutils::isSymbol(timeouttype,CcInt::BasicType())
      && !listutils::isSymbol(timeouttype,CcReal::BasicType()) ){
    return listutils::typeError("Expected '"+CcReal::BasicType()+"' or '"
    +CcInt::BasicType()+"' as 4th argument.");
  }
  ListExpr retriestype = nl->Fifth(args);
  if(    !listutils::isSymbol(retriestype,CcInt::BasicType())
    && !listutils::isSymbol(retriestype,CcReal::BasicType()) ){
    return listutils::typeError("Expected '"+CcReal::BasicType()+"' or '"
    +CcInt::BasicType()+"' as 5th argument.");
  }
  NList resTupleType = NList(NList("Ok"),NList(CcBool::BasicType())).enclose();
  resTupleType.append(NList(NList("Msg"),NList(FText::BasicType())));
  resTupleType.append(NList(NList("ErrMsg"),NList(CcString::BasicType())));
  NList resType =
    NList(NList(NList(Symbol::STREAM()),
                NList(NList(Tuple::BasicType()),resTupleType)));
  return resType.listExpr();
}


/*
2.62 ~Int2Text\_TM~

For operator ~charToText~.

---- int -> text
----

*/
ListExpr Int2Text_TM( ListExpr args ){
  int noargs = nl->ListLength(args);
  if( (noargs != 1) ){
    return listutils::typeError("Expected (int).");
  }
  if(!listutils::isSymbol(nl->First(args),CcInt::BasicType())){
    return listutils::typeError("Expected (int).");
  }
  return nl->SymbolAtom(FText::BasicType());
}


ListExpr attr2textTM(ListExpr args){
   string err = "DATA expected";
   if(nl->ListLength(args)!=1){
     return listutils::typeError(err);
   }

   if(listutils::isDATA(nl->First(args))){
      return nl->SymbolAtom(FText::BasicType());
   }
   return listutils::typeError(err);
}


ListExpr isValidIDTM(ListExpr args){
  string err =" expected string [ x bool]";

  int len = nl->ListLength(args);
  if((len !=1) && (len!=2)){
    return listutils::typeError(err);
  }

  if(!CcString::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(len == 2){
    if(!CcBool::checkType(nl->Second(args))){
        return listutils::typeError(err);
    }
    return nl->SymbolAtom(CcBool::BasicType());
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->BoolAtom(false)),
                           nl->SymbolAtom(CcBool::BasicType())); 

}


/*
2.63 Operator trimAll

This operator trims all occurences of text / string within a stream

Type Mappings are:

   stream(text) -> stream(text)
   stream(string) -> stream(string)
   stream(tuple(X)) -> stream(tuple(X))


*/
ListExpr trimAllTM(ListExpr args){
  string err = "stream(string), stream(text), or stream(tuple(X)) expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " (wrong number of arguments");
  }
  ListExpr arg = nl->First(args);

  if(Stream<CcString>::checkType(arg)){
    return arg;
  }
  if(Stream<FText>::checkType(arg)){
    return arg;
  }
  if(!Stream<Tuple>::checkType(arg)){
    return listutils::typeError(err);
  }
  // append the indexes of string/text elements
  ListExpr attrList = nl->Second(nl->Second(arg));
  ListExpr appendList=nl->TheEmptyList();
  ListExpr last = appendList;
  bool first = true;
  int pos = -1;
  while(!nl->IsEmpty(attrList)){
    pos++;
    ListExpr attr = nl->First(attrList);
    attrList = nl->Rest(attrList);
    ListExpr type = nl->Second(attr);
    if(CcString::checkType(type)){
      if(first){
        appendList = nl->OneElemList(nl->IntAtom(pos));
        last = appendList;
        first = false;
      } else {
        last = nl->Append(last, nl->IntAtom(pos));
      }
      last = nl->Append(last, nl->BoolAtom(false));
    }
    if(FText::checkType(type)){
      if(first){
        appendList = nl->OneElemList(nl->IntAtom(pos));
        last = appendList;
        first = false;
      } else {
        last = nl->Append(last, nl->IntAtom(pos));
      }
      last = nl->Append(last, nl->BoolAtom(true));
    }
  }

  if(nl->IsEmpty(appendList)){
    return arg;
  } 

  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                            appendList,
                            arg);
}








/*
3.3 Value Mapping Functions

*/

/*
Value Mapping for the ~contains~ operator

*/
template<class T1, class T2>
int ValMapContains( Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
  if(traces){
    cout <<'\n'<<"Start ValMapContains"<<'\n';
  }

  T1* text    = static_cast<T1*>(args[0].addr);
  T2* pattern = static_cast<T2*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  if(!text->IsDefined() || !pattern->IsDefined()){
    res->SetDefined(false);
  } else {
    string t = text->GetValue();
    string p = pattern->GetValue();
    res->Set( true, (t.find(p) != string::npos) );
  }

  if(traces){
    cout <<"End ValMapContains"<<'\n';
  }
  return 0;
}

ValueMapping FText_VMMap_Contains[] =
{
  ValMapContains<CcString, FText>,    //  0
  ValMapContains<FText, CcString>,    //  1
  ValMapContains<FText, FText>,       //  2
  ValMapContains<CcString, CcString>  //  3
};


/*
Value Mapping for the ~length~ operator with a text and a string operator .

*/

int ValMapTextInt(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
  if(traces)
    cout <<'\n'<<"Start ValMapTextInt"<<'\n';
  FText* ftext1= ((FText*)args[0].addr);

  result = qp->ResultStorage(s); //query processor has provided
          //a CcBool instance to take the result
  ((CcInt*)result.addr)->Set(true, ftext1->TextLength());
          //the first argument says the boolean
          //value is defined, the second is the
          //length value)
  if(traces)
    cout <<"End ValMapTextInt"<<'\n';
  return 0;
}

/*
4.21 Value mapping function of operator ~keywords~

The following auxiliary function ~trim~ removes any kind of space
characters from the end of a string.

*/

int trimstr (char s[])
{
  int n;

  for(n = strlen(s) - 1; n >= 0; n--)
   if ( !isspace(s[n]) ) break;
  s[n+1] = '\0';
  return n;
}

// NonStopCharakters are alphanumeric characters, german umlauts and
// the hyphen/minus sign
bool IsNonStopCharacter(const char c)
{
  const unsigned char uc = static_cast<const unsigned char>(c);
  return ( isalnum(c)
      || c == '-' || (uc > 191 && uc <215)
      || (uc > 215 && uc <247) || (uc > 247));
}

int
ValMapkeywords (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Creates a stream of strings of a text given as input parameter,
Recognized strings contains letters, digits and the '-' character.
The length of a string is three characters or more.

*/
{
  struct TheText {
     int start,
     nochr,
     strlength;
     char* subw;
  }* thetext;

  int textcursor, stringcursor, state;
  string tmpstr;
  STRING_T outstr;
  char c;
  CcString* mystring;

  switch( message )
  {
    case OPEN:
      //cout << "open ValMapkeywords" << endl;
      thetext = new TheText;
      thetext->start = 0;
      thetext->nochr = 0;
      thetext->subw = ((FText*)args[0].addr)->Get();
      thetext->strlength = ((FText*)args[0].addr)->TextLength();
      local.addr = thetext;
      return 0;

    case REQUEST:
      //cout << "request ValMapkeywords" << endl;
      thetext = ((TheText*) local.addr);
      textcursor = thetext->start;
      stringcursor = 0;
      state = 0;
      if(thetext->start>=thetext->strlength){
         return CANCEL;
      }
      while (true) {
        switch ( state ) {
          case 0 : c = thetext->subw[textcursor];
                   if ( IsNonStopCharacter( c ) )
                   {
                     outstr[stringcursor] = c;
                     stringcursor++;
                     state = 1;
                   }
                   else {
                     state = 2;
                     stringcursor = 0;
                   }
                   if ( c == '\0' ) { return CANCEL; }
                   textcursor++;
                   break;
          case 1 : c = thetext->subw[textcursor];
                   //cout << c << " state 1 " << endl;
                   if (IsNonStopCharacter( c ))
                   {
                     outstr[stringcursor] = c;
                     stringcursor++;
                     state = 3;
                   }
                   else {
                     state = 2;
                     stringcursor = 0;
                   }
                   if ( c == '\0' ) { return CANCEL; }
                   textcursor++;
                   break;
          case 2 : c = thetext->subw[textcursor];
                   //cout << c << " state 2 " << endl;
                   if (IsNonStopCharacter( c ))
                   {
                     outstr[stringcursor] = c;
                     stringcursor++;
                     state = 1;
                   }
                   else {
                     state = 2;
                     stringcursor = 0;
                   }
                   if ( c == '\0' ) { return CANCEL; }
                   textcursor++;
                   break;
          case 3 : c = thetext->subw[textcursor];
                   //cout << c << " state 3 " << endl;
                   if (IsNonStopCharacter( c ))
                   {
                     outstr[stringcursor] = c;
                     stringcursor++;
                     state = 4;
                   }
                   else {
                     state = 2;
                     stringcursor = 0;
                   }
                   if ( c == '\0' ) { return CANCEL; }
                   textcursor++;
                   break;
        case 4 : c = thetext->subw[textcursor];
                 //cout << c << " state 4 " << endl;
                 if ( IsNonStopCharacter( c ) &&
                      (stringcursor == MAX_STRINGSIZE) ) {
                  state = 5;
                  stringcursor = 0;
                 }
                 else if ( IsNonStopCharacter( c ) &&
                           (stringcursor < MAX_STRINGSIZE) ) {
                   outstr[stringcursor] = c;
                   stringcursor++;
                   state = 4;
                 }
                 else {
                   //if ( c == '\0' )
                   //{ outstr[stringcursor] = c; stringcursor ++; }
                   if ( textcursor == thetext->strlength )
                   { outstr[stringcursor] = c; stringcursor++; };
                   outstr[stringcursor] = '\0';
                   stringcursor = 0;
                   mystring = new CcString(true,&outstr);
                   result.setAddr(mystring);
                   thetext->start = ++textcursor;
                   local.addr = thetext;
                   return YIELD;
                 }
                 textcursor++;
                 break;
        case 5 : c = thetext->subw[textcursor];
                 //cout << c << " state 5 " << endl;
                 if ( isalnum(c) || c == '-' ) {
                   state = 5;
                   stringcursor = 0;
                 }
                 else {
                   state = 0;
                   stringcursor = 0;
                 }
                 if ( textcursor == thetext->strlength ) { return CANCEL; }
                 textcursor++;
                 break;

      }
    }

    case CLOSE:
      //cout << "close ValMapkeywords" << endl;
      if(local.addr)
      {
        thetext = ((TheText*) local.addr);
        delete [] thetext->subw;
        delete thetext;
        local.setAddr(0);
      }
      return 0;
  }
  /* should not happen */
  return -1;
}

int ValMapsentences (Word* args, Word& result, int message,
                     Word& local, Supplier s)
{
  struct TheText {int start, strlength; char* subw;}* thetext;
  int textcursor = 0, state = 0;
  string tmpstr = "";
  char c = 0;
  FText* returnsentence = 0;

  switch( message )
  {
    case OPEN:
      //cout << "open ValMapsentences" << endl;
      thetext = new TheText;
      thetext->start = 0;
      thetext->strlength = ((FText*)args[0].addr)->TextLength();
      thetext->subw = ((FText*)args[0].addr)->Get();
      local.addr = thetext;
      return 0;

    case REQUEST:
      //cout << "request ValMapsentences" << endl;
      thetext = ((TheText*) local.addr);
      textcursor = thetext->start;
      tmpstr = "";
      state = 0;

      while (true) {
        switch ( state ) {
          case 0 : c = thetext->subw[textcursor];
                   if ( (c == '\0') || (textcursor > thetext->strlength) )
                   { return CANCEL; }
                   if ( c == ',' || c == ';' || c ==':' || c ==' '
                                 || c == '\n' || c == '\t' )
                   {
                     state = 0;
                   }
                   else { if ( c == '.' || c == '!' || c =='?' )
                     {
                       tmpstr += c;
                       state = 3;
                     }
                     else  {
                       tmpstr += c;
                       state = 1;
                     }
                   }
                   textcursor++;
                   break;
          case 1 : c = thetext->subw[textcursor];
                   if ( (c == '\0') || (textcursor > thetext->strlength) )
                   { return CANCEL; }
                   if ( c == ',' || c == ';' || c ==':' )
                   {
                     tmpstr += c;
                     tmpstr += " ";
                     state = 0;
                   }
                   else { if ( c == ' ' || c == '\n' || c == '\t' )
                     state = 2;
                     else { if ( c == '.' || c == '!' || c =='?' )
                       {
                         tmpstr += c;
                         state = 3;
                       }
                       else {
                         tmpstr += c;
                         state = 1;
                       }
                     }
                   }
                   textcursor++;
                   break;
          case 2 : c = thetext->subw[textcursor];
                   if ( (c == '\0') || (textcursor > thetext->strlength) )
                   { return CANCEL; }
                   if ( c == ',' || c == ';' || c ==':' )
                   {
                     tmpstr += c;
                     tmpstr += " ";
                     state = 0;
                   }
                   else { if ( c == ' ' || c == '\n' || c == '\t' )
                     state = 2;
                     else { if ( c == '.' || c == '!' || c =='?' )
                       {
                         tmpstr += c;
                         state = 3;
                       }
                       else {
                         tmpstr += ' ';
                         tmpstr += c;
                         state = 1;
                       }
                     }
                   }
                   textcursor++;
                   break;
          case 3 : if ( (c == '\0') || (textcursor > thetext->strlength) )
                   { return CANCEL; }
                   returnsentence = new FText(true, (char*)tmpstr.c_str());
                   result.setAddr(returnsentence);
                   thetext->start = textcursor;
                   local.addr = thetext;
                   return YIELD;
        }
      }
    case CLOSE:
      //cout << "close ValMapsentences" << endl;
      if(local.addr)
      {
        thetext = ((TheText*) local.addr);
        delete [] thetext->subw;
        delete thetext;
        local.setAddr(0);
      }
      return 0;
  }
  /* should not happen */
  return -1;
}



int ValMapDice_t_t(Word* args, Word& result, int message,
                   Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcInt* arg1 = (CcInt*) args[0].addr;
  FText* arg2 = (FText*) args[1].addr;
  FText* arg3 = (FText*) args[2].addr;
  int n = arg1->GetIntval();
  if(n<=0){ // ensure a minimum of 1
     n = 1;
  }
  DiceTree* dt = new DiceTree(n);
  char* s1 = arg2->Get();
  char* s2 = arg3->Get();
  dt->appendText(s1,true);
  dt->appendText(s2,false);
  double res = dt->getCoeff();
  delete dt;
  delete[] s2;
  delete[] s1;
  ((CcReal*)result.addr)->Set(true,res);
  return 0;
}

/*
4.26 Operator ~getCatalog~


*/

struct GetCatalogLocalInfo{
  NList myCatalog;
  bool finished;
};

int ValMapGetCatalog( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  GetCatalogLocalInfo* li   = 0;
  bool foundValidEntry      = true;
  Tuple *newTuple           = 0;
  CcString *objectNameValue = 0;
  FText *typeValue          = 0;
  FText *typeExprValue      = 0;
  TupleType *resultTupleType = 0;

  switch( message )
  {
    case OPEN:
      // cout << "open" << endl;
      li = new GetCatalogLocalInfo;
      li->finished = true;
      li->myCatalog = NList(SecondoSystem::GetCatalog()->ListObjects());
      if(!li->myCatalog.isEmpty() && li->myCatalog.isList())
      {
        li->myCatalog.rest(); // ignore 'OBJECTS'
        li->finished = false;
      }
      local.setAddr( li );
      return 0;

    case REQUEST:
      //  cout << "request" << endl;
      if (local.addr == 0)
        return CANCEL;
      li = (GetCatalogLocalInfo*) local.addr;
      if( li->finished )
        return CANCEL;
      if(li->myCatalog.isEmpty())
      {
        li->finished = true;
        return CANCEL;
      }
      foundValidEntry = false;
      while( !foundValidEntry )
      {
        // Get head of li->myCatalog
        NList currentEntry = li->myCatalog.first();
        if(    currentEntry.isList()
               && currentEntry.hasLength(4)
               && currentEntry.first().isSymbol("OBJECT")
               && currentEntry.second().isSymbol()
               && currentEntry.third().isList()
               && currentEntry.fourth().isList()
          )
        {
          currentEntry.rest(); // ignore 'OBJECT'
          objectNameValue =
              new CcString(true, currentEntry.first().convertToString());
          typeValue =
              new FText(true, currentEntry.second().isEmpty() ? "" :
              currentEntry.second().first().convertToString().c_str());
          typeExprValue =
              new FText(true, currentEntry.third().isEmpty() ? "" :
              currentEntry.third().first().convertToString().c_str());
          resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
          newTuple = new Tuple( resultTupleType );
          newTuple->PutAttribute(  0,(Attribute*)objectNameValue);
          newTuple->PutAttribute(  1,(Attribute*)typeValue);
          newTuple->PutAttribute(  2,(Attribute*)typeExprValue);
          result.setAddr(newTuple);
          resultTupleType->DeleteIfAllowed();
          foundValidEntry = true;
        } else
        {
          cerr << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
              << "): Malformed Catalog Entry passed:" << endl
              << "\tcurrentEntry.isList() = "
              << currentEntry.isList() << endl
              << "\tcurrentEntry.hasLength(4) = "
              << currentEntry.hasLength(4) << endl
              << "\tcurrentEntry.first().isSymbol(\"OBJECT\") = "
              << currentEntry.first().isSymbol("OBJECT") << endl
              << "\tcurrentEntry.second().isSymbol() "
              << currentEntry.second().isSymbol() << endl
              << "\tcurrentEntry.third().isList() = "
              << currentEntry.third().isList() << endl
              << "\tcurrentEntry.fourth().isList()"
              << currentEntry.fourth().isList() << endl
              << "\tcurrentEntry is: "
              << currentEntry.convertToString() << endl;
        }
        li->myCatalog.rest();
      }
      if(foundValidEntry)
        return YIELD;
      li->finished = true;
      return CANCEL;

    case CLOSE:
      // cout << "close" << endl;
      if (local.addr != 0){
        li = (GetCatalogLocalInfo*) local.addr;
        delete li;
        local.addr = 0;
      }
      return 0;
  }
  /* should not happen */
  return -1;
}


/*
4.27 Operator ~substr~


*/
int ValMapSubstr( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  FText *Ctxt    = (FText*)args[0].addr;
  CcInt *Cbegin  = (CcInt*)args[1].addr;
  CcInt *Cend    = (CcInt*)args[2].addr;
  CcString *CRes = reinterpret_cast<CcString*>(result.addr);

  if ( !Ctxt->IsDefined() || !Cbegin->IsDefined() || !Cend->IsDefined() )
  {
    CRes->Set( false, string("") );
    return 0;
  }
  int begin  = Cbegin->GetIntval();
  int end    = Cend->GetIntval();
  int txtlen = Ctxt->TextLength();
  if( (begin < 1) || (begin > end) || (begin > txtlen) )
  {
    CRes->Set( false, string("") );
    return 0;
  }
  int n = min(min( (end-begin), (txtlen-begin) ),
              static_cast<int>(MAX_STRINGSIZE));
  string mytxt =  Ctxt->GetValue();
//   cout << "mytxt=\"" << mytxt << "\"" << endl;
  string mysubstring = mytxt.substr(begin-1, n+1);
//   cout << "mysubstring=\"" << mysubstring << "\"" << endl;
  CRes->Set( true, mysubstring );
  return 0;
}

/*
4.28 Operator ~subtext~


*/
int ValMapSubtext( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText *CRes   = (FText*)(result.addr);
  FText *Ctxt   = (FText*)args[0].addr;
  CcInt *Cbegin = (CcInt*)args[1].addr;
  CcInt *Cend   = (CcInt*)args[2].addr;

  if ( !Ctxt->IsDefined() || !Cbegin->IsDefined() || !Cend->IsDefined() )
  {
    CRes->Set( false, string("") );
    return 0;
  }
  int begin   = Cbegin->GetIntval();
  int end     = Cend->GetIntval();
  string mTxt = Ctxt->GetValue();
  int txtlen  = mTxt.length();
  if( (begin < 1) || (begin > end) || (begin > txtlen) )
  {
    CRes->Set( false, string("") );
    return 0;
  }
  int n = min( (end - begin)+1, txtlen);
  string myRes = mTxt.substr(begin-1, n);
  CRes->Set( true, myRes );
  return 0;
}

/*
4.28 Operator ~crypt~

*/
template<class T>
int cryptVM(Word* args, Word& result, int message,
            Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);
  T* arg = static_cast<T*>(args[0].addr);
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string a = arg->GetValue();

  char s1 = (rand() / ( RAND_MAX / 63 + 1 ))+46;
  char s2 = (rand() / ( RAND_MAX / 63 + 1 ))+46;

  if(s1>57){
    s1 += 7;
  }
  if(s1>90){
    s1 += 6;
  }
  if(s2>57){
    s2 += 7;
  }
  if(s2>90){
    s2 += 6;
  }
  char salt[] = {  s1, s2 };
  string b = (Crypt::crypt(a.c_str(),salt));
  res->Set(true,b);
  return 0;
}

template<class T1, class T2>
int cryptVM(Word* args, Word& result, int message,
            Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);
  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string a = arg1->GetValue();
  string b = arg2->GetValue();
  while(b.length()<2){
    b += "X";
  }

  string c (Crypt::crypt(a.c_str(),b.c_str()));
  res->Set(true,c);
  return 0;
}

 // value mapping array

ValueMapping cryptvm[] = {
      cryptVM<CcString>, cryptVM<FText>,
      cryptVM<CcString, CcString>, cryptVM<CcString, FText>,
      cryptVM<FText, CcString>, cryptVM<FText, FText>
  };

 // Selection function
 int cryptSelect(ListExpr args){
   int l = nl->ListLength(args);
   string s1 = nl->SymbolValue(nl->First(args));
   if(l==1){
     if(s1==CcString::BasicType()) return 0;
     if(s1==FText::BasicType()) return 1;
   } else {
     string s2 = nl->SymbolValue(nl->Second(args));
     if(s1==CcString::BasicType() && s2==CcString::BasicType()) return 2;
     if(s1==CcString::BasicType() && s2==FText::BasicType()) return 3;
     if(s1==FText::BasicType() && s2==CcString::BasicType()) return 4;
     if(s1==FText::BasicType() && s2==FText::BasicType()) return 5;
   }
   return -1; // type mapping and selection are not compatible
 }

/*
4.27 Operator checkpw

*/
template<class T1, class T2>
int checkpwVM(Word* args, Word& result, int message,
              Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string a = arg1->GetValue();
  string b = arg2->GetValue();
  res->Set(true, Crypt::validate(a.c_str(),b.c_str()));
  return 0;
}

ValueMapping checkpwvm[] = {
   checkpwVM<CcString, CcString>,
   checkpwVM<CcString, FText>,
   checkpwVM<FText, CcString>,
   checkpwVM<FText, FText>
};


int checkpwSelect(ListExpr args){
  string s1 = nl->SymbolValue(nl->First(args));
  string s2 = nl->SymbolValue(nl->Second(args));
  if(s1==CcString::BasicType() && s2==CcString::BasicType()) return 0;
  if(s1==CcString::BasicType() && s2==FText::BasicType()) return 1;
  if(s1==FText::BasicType() && s2==CcString::BasicType()) return 2;
  if(s1==FText::BasicType() && s2==FText::BasicType()) return 3;
  return -1; // type mapping and selection are not compatible
 }



/*
4.28 Operator ~md5~

*/
template<class T>
int md5VM(Word* args, Word& result, int message,
          Word& local, Supplier s ) {

   result = qp->ResultStorage(s);
   CcString* res = static_cast<CcString*>(result.addr);
   T* arg = static_cast<T*>(args[0].addr);
   if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string a = arg->GetValue();
   unsigned char digest[16];
   MD5::md5(a.c_str(),digest);
   string r(MD5::toString(digest));
   res->Set(true, r);
   return 0;
}

template<class T1, class T2>
int md5VM(Word* args, Word& result, int message,
            Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);
  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string a = arg1->GetValue();
  string b = arg2->GetValue();
  while(b.length()<2){
    b += "X";
  }

  char*  c = (MD5::unix_encode(a.c_str(),b.c_str()));

  unsigned char *c1 = reinterpret_cast<unsigned char*>(c);

  res->Set(true,MD5::toString(c1));
  return 0;
}

 // value mapping array

ValueMapping md5vm[] = {
      md5VM<CcString>, md5VM<FText>,
      md5VM<CcString, CcString>, md5VM<CcString, FText>,
      md5VM<FText, CcString>, md5VM<FText, FText>
  };

 // Selection function
 int md5Select(ListExpr args){
   int l = nl->ListLength(args);
   string s1 = nl->SymbolValue(nl->First(args));
   if(l==1){
     if(s1==CcString::BasicType()) return 0;
     if(s1==FText::BasicType()) return 1;
   } else {
     string s2 = nl->SymbolValue(nl->Second(args));
     if(s1==CcString::BasicType() && s2==CcString::BasicType()) return 2;
     if(s1==CcString::BasicType() && s2==FText::BasicType()) return 3;
     if(s1==FText::BasicType() && s2==CcString::BasicType()) return 4;
     if(s1==FText::BasicType() && s2==FText::BasicType()) return 5;
   }
   return -1; // type mapping and selection are not compatible
 }

template<class T1, class T2>
int blowfish_encodeVM(Word* args, Word& result, int message,
                      Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string a = arg1->GetValue();
  string b = arg2->GetValue();

  CBlowFish bf;
  unsigned char* passwd = (unsigned char*)(a.c_str());
  unsigned char* text = (unsigned char*)(b.c_str());

  bf.Initialize(passwd, a.length());
  int ol = bf.GetOutputLength(b.length());
  unsigned char out[ol];
  int l = bf.Encode(text, out, b.length());
  ostringstream ss;
  ss << std::hex;
  for(int i=0;i<l;i++){
    if(out[i]<16){
      ss << '0';
    }
    ss << (short)out[i];
  }
  res->Set(true, ss.str());
  return 0;
}


 // value mapping array

ValueMapping blowfish_encodevm[] = {
         blowfish_encodeVM<CcString, CcString>,
         blowfish_encodeVM<CcString, FText>,
         blowfish_encodeVM<FText, CcString>,
         blowfish_encodeVM<FText, FText>
};

 // Selection function
int blowfish_encodeSelect(ListExpr args){
   string s1 = nl->SymbolValue(nl->First(args));
   string s2 = nl->SymbolValue(nl->Second(args));
   if(s1==CcString::BasicType() && s2==CcString::BasicType()) return 0;
   if(s1==CcString::BasicType() && s2==FText::BasicType()) return 1;
   if(s1==FText::BasicType() && s2==CcString::BasicType()) return 2;
   if(s1==FText::BasicType() && s2==FText::BasicType()) return 3;
   return -1; // type mapping and selection are not compatible
}

/*

blowfish[_]decode

*/

int fromHex(unsigned char s){
  if(s>='0' && s<='9'){
     return s - '0';
  }
  if(s>='a' && s<='f'){
     return s -'a'+10;
  }
  if(s>='A' && s<='F'){
     return s -'A'+10;
  }
  return -1;
}

template<class T1, class T2>
int blowfish_decodeVM(Word* args, Word& result, int message,
                      Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string a = arg1->GetValue();
  string b = arg2->GetValue();

  CBlowFish bf;
  unsigned char* passwd = (unsigned char*)(a.c_str());
  unsigned char* text = (unsigned char*)(b.c_str());

  bf.Initialize(passwd, a.length());
  unsigned char orig[b.length()/2+1];
  // read the coded block from hex-text
  for(unsigned int i=0;i<b.length()-1;i+=2){
     int p1 = fromHex(text[i]);
     int p2 = fromHex(text[i+1]);
     if(p1<0 || p2<0){
       res->SetDefined(false);
       return 0;
     }
     orig[i/2] = (unsigned char)(p1*16+p2);
  }
  orig[b.length()/2]=0;
  bf.Decode(orig, orig, b.length()/2);
  res->Set(true,string((char*)orig));
  return 0;
}


 // value mapping array

ValueMapping blowfish_decodevm[] = {
         blowfish_decodeVM<CcString, CcString>,
         blowfish_decodeVM<CcString, FText>,
         blowfish_decodeVM<FText, CcString>,
         blowfish_decodeVM<FText, FText>
};

 // Selection function
int blowfish_decodeSelect(ListExpr args){
   return blowfish_encodeSelect(args);
}

/*
4.29 Operator ~find~


*/

struct ValMapFindLocalInfo{
  string text;
  string pattern;
  size_t textlen;
  size_t patternlen;
  size_t lastPosFound;
  bool finished;
};

template<class T1, class T2>
int FTextValMapFind( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  T1 *Ctext = 0;
  T2 *Cpattern = 0;
  ValMapFindLocalInfo *li;

  switch( message )
  {
    case OPEN:

      Ctext    = static_cast<T1*>(args[0].addr);
      Cpattern = static_cast<T2*>(args[1].addr);
      li = new ValMapFindLocalInfo;
      li->finished = true;
      li->text = "";
      li->pattern = "";
      li->textlen = 0;
      li->patternlen = 0;
      li->lastPosFound = string::npos;
      if( Ctext->IsDefined() && Cpattern->IsDefined() )
      {
        li->text = Ctext->GetValue();
        li->pattern = Cpattern->GetValue();
        li->lastPosFound = 0;
        li->textlen      = li->text.length();
        li->patternlen   = li->pattern.length();
        li->finished = (    (li->patternlen > li->textlen)
                         || (li->patternlen == 0)
                       );
      }
      local.setAddr(li);
      return 0;

    case REQUEST:
      if(local.addr == 0)
      {
        result.addr = 0;
        return CANCEL;
      }
      li = (ValMapFindLocalInfo*) local.addr;
      if(     li->finished
          || (li->lastPosFound == string::npos)
          || (li->lastPosFound + li->patternlen > li->textlen)
        )
      {
        result.addr = 0;
        return CANCEL;
      }
      li->lastPosFound = li->text.find(li->pattern, li->lastPosFound);
      if(li->lastPosFound != string::npos)
      {
        CcInt* res = new CcInt(true, ++(li->lastPosFound));
        result.addr = res;
        return YIELD;
      }
      li->finished = false;
      result.addr = 0;
      return CANCEL;

    case CLOSE:
      if(local.addr != 0)
      {
        li = (ValMapFindLocalInfo*) local.addr;
        delete li;
        local.setAddr( Address(0) );
      }
      return 0;
  }
  return 0;
}

ValueMapping FText_VMMap_Find[] =
{
  FTextValMapFind<CcString, FText>,    //  0
  FTextValMapFind<FText, CcString>,    //  1
  FTextValMapFind<FText, FText>,       //  2
  FTextValMapFind<CcString, CcString>  //  3
};


int SVG2TEXTVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
   FText* arg = static_cast<FText*>(args[0].addr);
   result = qp->ResultStorage(s);
   FText* res = static_cast<FText*>(result.addr);
   res->CopyFrom(arg);
   return 0;
}



int SelectFun_TextString_TextString( ListExpr args )
{
  NList type(args);
  if( (type.first() == NList(CcString::BasicType())) &&
      (type.second() == NList(FText::BasicType())) )
    return 0;
  if( (type.first() == NList(FText::BasicType())) &&
      (type.second() == NList(CcString::BasicType())) )
    return 1;
  if( (type.first() == NList(FText::BasicType())) &&
      (type.second() == NList(FText::BasicType())) )
    return 2;
  if( (type.first() == NList(CcString::BasicType())) &&
      (type.second() == NList(CcString::BasicType())) )
    return 3;
  // else: ERROR
  return -1;
}

/*
4.30 Operator ~isempty~


*/
int FTextValMapIsEmpty( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *CRes = reinterpret_cast<CcBool*>(result.addr);
  FText *Ctxt    = (FText*)args[0].addr;
  CRes->Set( true, ( !Ctxt->IsDefined() || Ctxt->TextLength() == 0) );
  return 0;
}

/*
4.30 Operator ~trim~

Removes whitespaces from a text at the start and the end of the text.

4.30.1 Type Mapping

*/

ListExpr trimTM(ListExpr args) {
 string err = "string or text expected";
 if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
 }
 ListExpr arg = nl->First(args);
 if(FText::checkType(arg) || CcString::checkType(arg)){
   return arg;
 }
 return  listutils::typeError(err);
}

template <class T>
int FTextValMapTrim( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  T* res = static_cast<T*>(result.addr);
  T* src = static_cast<T*>(args[0].addr);
  if(!src->IsDefined()){
     res->SetDefined(false);
  } else {
    string str = src->GetValue();
    string drop = " \t\n\r";
    str = str.erase(str.find_last_not_of(drop)+1);
    str = str.erase(0,str.find_first_not_of(drop));
    res->Set(true,str);
  }
  return 0;
}

ValueMapping trimVM[] = {FTextValMapTrim<FText>, FTextValMapTrim<CcString>};

int trimSelect(ListExpr args){
  if(FText::checkType(nl->First(args))){
     return 0;
  }
  if(CcString::checkType(nl->First(args))){
     return 1;
  }
  return -1;
}





/*
4.30 Operator ~+~

*/
template<class T1, class T2>
int FTextValMapPlus( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText *CRes = reinterpret_cast<FText*>(result.addr);

  T1 *Ctxt1    = (T1*)args[0].addr;
  T2 *Ctxt2    = (T2*)args[1].addr;

  if( !Ctxt1->IsDefined() || !Ctxt2->IsDefined() )
  {
    CRes->SetDefined( false );
    return 0;
  }

  string mTxt1 = string(Ctxt1->GetValue());
  string mTxt2 = string(Ctxt2->GetValue());
  string myRes = mTxt1 + mTxt2;

  CRes->Set( true, myRes );
  return 0;
}

ValueMapping FText_VMMap_Plus[] =
{
  FTextValMapPlus<CcString, FText>,    //  0
  FTextValMapPlus<FText, CcString>,    //  1
  FTextValMapPlus<FText, FText>        //  2
};

int FTextSelectFunPlus( ListExpr args )
{
  NList type(args);
  if( (type.first() == NList(CcString::BasicType())) &&
      (type.second() == NList(FText::BasicType())) )
    return 0;
  if( (type.first() == NList(FText::BasicType())) &&
      (type.second() == NList(CcString::BasicType())) )
    return 1;
  if( (type.first() == NList(FText::BasicType())) &&
      (type.second() == NList(FText::BasicType())) )
    return 2;
  // else: ERROR
  return -1;
}

/*
4.30 Operators  ~$<$, $<=$, $=$, $>=$, $>$, $\neq$~

----
    <, <=, =, >=, >, #: {string|text} x {string|text} --> bool
OP: 0,  1, 2,  3, 4, 5
----

*/
template<class T1, class T2, int OP>
int FTextValMapComparePred( Word* args, Word& result, int message,
                            Word& local, Supplier s )
{
  assert( (OP >=0) && (OP <=5) );

  result = qp->ResultStorage( s );
  CcBool *CRes = (CcBool*)(result.addr);
  bool res = false;
  T1 *Ctxt1 = (T1*)args[0].addr;
  T2 *Ctxt2 = (T2*)args[1].addr;
  if( !Ctxt1->IsDefined() || !Ctxt2->IsDefined() )
  {
    CRes->Set( false, false );
    return 0;
  }
  string mTxt1 = string(Ctxt1->GetValue());
  string mTxt2 = string(Ctxt2->GetValue());
  int cmp = mTxt1.compare(mTxt2);
  switch( OP )
  {
    case 0: res = (cmp <  0); break; // <
    case 1: res = (cmp <= 0); break; // <=
    case 2: res = (cmp == 0); break; //  =
    case 3: res = (cmp >= 0); break; // >=
    case 4: res = (cmp >  0); break; // >
    case 5: res = (cmp != 0); break; //  #
    default: assert( false);  break; // illegal mode of operation
  }
  CRes->Set( true, res );
  return 0;
}

ValueMapping FText_VMMap_Less[] =
{
  FTextValMapComparePred<CcString, FText, 0>,    //
  FTextValMapComparePred<FText, CcString, 0>,    //
  FTextValMapComparePred<FText, FText, 0>        //
};

ValueMapping FText_VMMap_LessEq[] =
{
  FTextValMapComparePred<CcString, FText, 1>,    //
  FTextValMapComparePred<FText, CcString, 1>,    //
  FTextValMapComparePred<FText, FText, 1>        //
};

ValueMapping FText_VMMap_Eq[] =
{
  FTextValMapComparePred<CcString, FText, 2>,    //
  FTextValMapComparePred<FText, CcString, 2>,    //
  FTextValMapComparePred<FText, FText, 2>        //
};

ValueMapping FText_VMMap_BiggerEq[] =
{
  FTextValMapComparePred<CcString, FText, 3>,    //
  FTextValMapComparePred<FText, CcString, 3>,    //
  FTextValMapComparePred<FText, FText, 3>        //
};

ValueMapping FText_VMMap_Bigger[] =
{
  FTextValMapComparePred<CcString, FText, 4>,    //
  FTextValMapComparePred<FText, CcString, 4>,    //
  FTextValMapComparePred<FText, FText, 4>        //
};

ValueMapping FText_VMMap_Neq[] =
{
  FTextValMapComparePred<CcString, FText, 5>,    //
  FTextValMapComparePred<FText, CcString, 5>,    //
  FTextValMapComparePred<FText, FText, 5>        //
};

int FTextSelectFunComparePred( ListExpr args )
{
  NList type(args);
  if( (type.first() == NList(CcString::BasicType())) &&
      (type.second() == NList(FText::BasicType())) )
    return 0;
  else if( (type.first() == NList(FText::BasicType())) &&
           (type.second() == NList(CcString::BasicType())) )
    return 1;
  else if( (type.first() == NList(FText::BasicType())) &&
       (type.second() == NList(FText::BasicType())) )
    return 2;
  return -1; // error
}

int FTextValueMapEvaluate( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  FText* CCommand     = (FText*)(args[0].addr);
  CcBool* CcIsNL      = (CcBool*)(args[1].addr);
  bool *finished;
  string querystring  = "";
  string querystringParsed = "";
  string typestring   = "";
  string resultstring = "";
  string errorstring  = "";
  Word queryresultword;
  bool success        = false;
  bool correct        = false;
  bool evaluable      = false;
  bool defined        = false;
  bool isFunction     = false;
  StopWatch myTimer;
  double myTimeReal   = 0;
  double myTimeCPU    = 0;

  SecParser mySecParser;
  ListExpr parsedCommand;
  TupleType *resultTupleType = 0;
  Tuple *newTuple            = 0;

  FText  *CcCmdStr           = 0;
  CcBool *CcSuccess          = 0;
  CcBool *CcCorrect          = 0;
  CcBool *CcEvaluable        = 0;
  CcBool *CcDefined          = 0;
  CcBool *CcIsFunction       = 0;
  FText  *CcResultType       = 0;
  FText  *CcResult           = 0;
  FText  *CcErrorMessage     = 0;
  CcReal *CcElapsedTimeReal  = 0;
  CcReal *CcElapsedTimeCPU   = 0;

  switch(message)
  {
    case OPEN:
      finished = new bool(false);
      local.addr = finished;
      *finished = (!CCommand->IsDefined() || !CcIsNL->IsDefined());
      return 0;
    case REQUEST:
      if(local.addr == 0)
      {
        result.setAddr(0);
        return CANCEL;
      }
      finished = (bool*)(local.addr);
      if(*finished)
      {
        result.setAddr(0);
        return CANCEL;
      }
      if(!CCommand->IsDefined())
      {
        *finished = true;
        result.setAddr(0);
        return CANCEL;
      }

      correct = true;
      querystring = CCommand->GetValue();

      if( !CcIsNL->GetBoolval() ) // Command in SecondoExecutableLanguage
      {
        // call Parser: add "query" and transform expression
        //              to nested-list-string
        if(mySecParser.Text2List( "query " + querystring,
                                       querystringParsed ) != 0)
        {
          errorstring = "ERROR: Text does not contain a "
              "parsable query expression.";
          correct = false;
        }
      }
      else // Command is already a nested-list-string: just copy
      {
        querystringParsed = querystring;
      }
      if( correct)
      { // read nested list: transform nested-list-string to nested list
        if (!nl->ReadFromString(querystringParsed, parsedCommand) )
        {
          errorstring = "ERROR: Text does not produce a "
              "valid nested list expression.";
          correct = false;
          cout << "NLimport: " << errorstring << endl;
        }
      }
      if ( correct && !CcIsNL->GetBoolval() )
      {  // remove the "query" from the list
        if ( (nl->ListLength(parsedCommand) == 2) )
        {
          parsedCommand = nl->Second(parsedCommand);
          //string parsedCommandstr;
          //nl->WriteToString(parsedCommandstr, parsedCommand);
          //cout << "NLimport: OK. parsedCommand=" << parsedCommandstr  << endl;
        }
        else
        {
          errorstring = "ERROR: Text does not produce a "
              "valid nested query list expression.";
          correct = false;
          //cout << "NLimport: " << errorstring  << endl;
        }
      }
      if (correct)
      { // evaluate command
        myTimer.start();

        try{
          success =
              QueryProcessor::ExecuteQuery(parsedCommand,
                                         queryresultword,
                                         typestring,
                                         errorstring,
                                         correct,
                                         evaluable,
                                         defined,
                                         isFunction);
        } catch(...){
           success = false;
        }

        myTimeReal = myTimer.diffSecondsReal();
        myTimeCPU  = myTimer.diffSecondsCPU();

        // handle result
        ListExpr queryResType;
        if ( !nl->ReadFromString( typestring, queryResType) )
        {
          errorstring = "ERROR: Invalid resulttype. " + errorstring;
        }
        else
        {
          if(   correct
                && evaluable
                && defined
                && ( typestring != Symbol::TYPEERROR()  )
            )
          { // yielded a result (no typerror)
            ListExpr valueList = SecondoSystem::GetCatalog()
                ->OutObject(queryResType,queryresultword);

            if(! SecondoSystem::GetCatalog()->
                   DeleteObj(queryResType,queryresultword)){
              cerr << "problem in deleting queryresultword" << endl;

            }
            nl->WriteToString(resultstring,valueList);
          }
          // else: typeerror or nonevaluable query
          //       - Just return retrieved values and let user decide -
        }
      }
      // handle times
      // create result tuple
      if(traces){
        cout << "\n---------------------------------------------------" << endl;
        cout << "\tsuccess     =" << (success ? "yes" : "no"    ) << endl;
        cout << "\tevaluable   =" << (evaluable ? "yes" : "no"  ) << endl;
        cout << "\tdefined     =" << (defined ? "yes" : "no"    ) << endl;
        cout << "\tisFunction  =" << (isFunction ? "yes" : "no" ) << endl;
        cout << "\tquerystring =" << querystring  << endl;
        cout << "\ttypestring  =" << typestring   << endl;
        cout << "\tresultstring=" << resultstring << endl;
        cout << "\terrorstring =" << errorstring  << endl;
        cout << "---------------------------------------------------" << endl;
      }

      resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      newTuple = new Tuple( resultTupleType );

      CcCmdStr       = new FText(true, querystring);
      CcSuccess      = new CcBool(true, success);
      CcCorrect      = new CcBool(true, correct);
      CcEvaluable    = new CcBool(true, evaluable);
      CcDefined      = new CcBool(true, defined);
      CcIsFunction   = new CcBool(true, isFunction);
      CcResultType   = new FText(true, typestring);
      CcResult       = new FText(true, resultstring);
      CcErrorMessage = new FText(true, errorstring);
      CcElapsedTimeReal = new CcReal(true, myTimeReal);
      CcElapsedTimeCPU  = new CcReal(true, myTimeCPU);

      newTuple->PutAttribute(  0,(Attribute*)CcCmdStr);
      newTuple->PutAttribute(  1,(Attribute*)CcSuccess);
      newTuple->PutAttribute(  2,(Attribute*)CcCorrect);
      newTuple->PutAttribute(  3,(Attribute*)CcEvaluable);
      newTuple->PutAttribute(  4,(Attribute*)CcDefined);
      newTuple->PutAttribute(  5,(Attribute*)CcIsFunction);
      newTuple->PutAttribute(  6,(Attribute*)CcResultType);
      newTuple->PutAttribute(  7,(Attribute*)CcResult);
      newTuple->PutAttribute(  8,(Attribute*)CcErrorMessage);
      newTuple->PutAttribute(  9,(Attribute*)CcElapsedTimeReal);
      newTuple->PutAttribute( 10,(Attribute*)CcElapsedTimeCPU);

      result.setAddr(newTuple);
      resultTupleType->DeleteIfAllowed();
      *finished = true;
      return YIELD;

    case CLOSE:
      if(local.addr != 0)
      {
        finished = (bool*)(local.addr);
        delete finished;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
Value Mapping Function for Operator ~replace~

*/

// {text|string} x {text|string} x {text|string} --> text
template<class T1, class T2, class T3>
int FTextValMapReplace( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText* Res = static_cast<FText*>(result.addr);

  T1* text       = static_cast<T1*>(args[0].addr);
  T2* patternOld = static_cast<T2*>(args[1].addr);
  T3* patternNew = static_cast<T3*>(args[2].addr);

  if(    !text->IsDefined()
      || !patternOld->IsDefined()
      || !patternNew->IsDefined()
    )
  {
    Res->Set(false, "");
    return 0;
  }
  string textStr       = text->GetValue();
  string patternOldStr = patternOld->GetValue();
  string patternNewStr = patternNew->GetValue();
  string textReplaced = "";
  textReplaced = stringutils::replaceAll(textStr, patternOldStr,
                                         patternNewStr);
  Res->Set(true, textReplaced);
  return 0;
}

// {text|string} x int    x int  x {text|string} --> text
template<class T1, class T2>
int FTextValMapReplace2( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText* Res = static_cast<FText*>(result.addr);

  T1*    text    = static_cast<T1*>   (args[0].addr);
  CcInt* start   = static_cast<CcInt*>(args[1].addr);
  CcInt* end     = static_cast<CcInt*>(args[2].addr);
  T2* patternNew = static_cast<T2*>   (args[3].addr);

  if(    !text->IsDefined()
          || !start->IsDefined()
          || !end->IsDefined()
          || !patternNew->IsDefined()
    )
  {
    Res->Set(false, "");
    return 0;
  }
  string textStr        = text->GetValue();
  string patternNewStr  = patternNew->GetValue();
  int starti            = start->GetIntval();
  int endi              = end->GetIntval();
  unsigned int len      = (unsigned int)(endi-starti);
  if(    (starti > endi)                             // illegal end pos
      || (starti < 1)                                // illegal startpos
      || (unsigned int)starti > textStr.length()    // illegal startpos
    )
  { // nothing to do
    Res->Set(true, textStr);
    return 0;
  }
  string textReplaced = "";
  textReplaced = textStr.replace(starti-1, len+1, patternNewStr);
  Res->Set(true, textReplaced);
  return 0;
}


/*
ValueMappingArray for ~replace~

*/

ValueMapping FText_VMMap_Replace[] =
{
  // {text|string} x {text|string} x {text|string} --> text
  FTextValMapReplace<FText,    FText,    FText>,    //  0
  FTextValMapReplace<FText,    FText,    CcString>, //  1
  FTextValMapReplace<FText,    CcString, FText>,    //  2
  FTextValMapReplace<FText,    CcString, CcString>, //  3
  FTextValMapReplace<CcString, FText,    FText>,    //  4
  FTextValMapReplace<CcString, FText,    CcString>, //  5
  FTextValMapReplace<CcString, CcString, FText>,    //  6
  FTextValMapReplace<CcString, CcString, CcString>, //  7
  // {text|string} x int    x int  x {text|string} --> text
  FTextValMapReplace2<FText,    FText>,             //  8
  FTextValMapReplace2<FText,    CcString>,          //  9
  FTextValMapReplace2<CcString, FText>,             // 10
  FTextValMapReplace2<CcString, CcString>           // 11
};


/*
Selection function for ~replace~

*/

int FTextSelectFunReplace( ListExpr args )
{
  NList type(args);
  int result = 0;
  if(type.hasLength(3))
  { // {text|string} x {text|string} x {text|string} --> text
    result = 0;
    if( type.third() == NList(CcString::BasicType()) )
      result += 1;
    if( type.second() == NList(CcString::BasicType()) )
      result += 2;
    if( type.first() == NList(CcString::BasicType()) )
      result += 4;
  }
  else
  { // {text|string} x int x int x {text|string} --> text
    result = 8;
    if( type.fourth() == NList(CcString::BasicType()) )
      result += 1;
    if( type.first() == NList(CcString::BasicType()) )
      result += 2;
  }
  return result;
}

/*
Value Mapping Function for Operator ~isDBObject~

*/
int FTextValueMapIsDBObject( Word* args, Word& result, int message,
                            Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool* Res = static_cast<CcBool*>(result.addr);
  CcString* objName=static_cast<CcString*>(args[0].addr);
  if(!objName->IsDefined()){
     Res->Set(false,false);
  } else {
     string oname = objName->GetValue();
     SecondoCatalog* ctl = SecondoSystem::GetCatalog();
     Res->Set(true,ctl->IsObjectName(oname));
  }


  return 0;
}


/*
Value Mapping Function for Operator ~getTypeNL~

*/
int FTextValueMapGetTypeNL( Word* args, Word& result, int message,
                            Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText* Res = static_cast<FText*>(result.addr);
  Word argResWord;
  qp->Request(args[1].addr,argResWord);
  FText* argType = (FText*) argResWord.addr;
  Res->CopyFrom(argType);
  return 0;
}

/*
Value Mapping Function for Operator ~getValueNL~

*/
// for single value
int FTextValueMapGetValueNL_single( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText*   Res       = static_cast<FText*>(result.addr);
  FText*   myTypeFT  = static_cast<FText*>(args[1].addr);
  string   myTypeStr = "";
  string   valueStr  = "";
  ListExpr myTypeNL;

  if( !myTypeFT->IsDefined() )
  { // Error: undefined type description
    cerr << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
        << "): ERROR: Undefined resulttype." << endl;
    Res->Set( false, "" );
    return 0;
  }
  if( args[0].addr == 0 )
  { // Error: NULL-pointer value
    cerr << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
        << "): ERROR: NULL-pointer argument." << endl;
    Res->Set( false, "" );
    return 0;
  }
  myTypeStr = myTypeFT->GetValue();
  if ( !nl->ReadFromString( myTypeStr, myTypeNL) )
  { // Error: could not parse type description
    cerr << __PRETTY_FUNCTION__<< "(" << __FILE__ << __LINE__
        << "): ERROR: Invalid resulttype." << endl;
    Res->Set( false, "" );
    return 0;
  }
  else if( myTypeStr != Symbol::TYPEERROR()  )
  {
    ListExpr valueNL =
        SecondoSystem::GetCatalog()->OutObject(myTypeNL,args[0]);
    nl->WriteToString(valueStr,valueNL);
    Res->Set( true, valueStr);
    return 0;
  }
  Res->Set( false, "" );
  return 0;
}

struct FTextValueMapGetValueNL_streamLocalInfo
{
  ListExpr myTypeNL;
  bool finished;
};

// for data streams
int FTextValueMapGetValueNL_stream( Word* args, Word& result, int message,
                             Word& local, Supplier s )
{
  result          = qp->ResultStorage( s );
  FText* Res      = 0;
  FText* myTypeFT = static_cast<FText*>(args[1].addr);
  FTextValueMapGetValueNL_streamLocalInfo *li;
  string   valueStr  = "";
  Word elem;

  switch( message )
  {
    case OPEN:
      li = new FTextValueMapGetValueNL_streamLocalInfo;
      li->finished = true;
      if( myTypeFT->IsDefined() )
      {
        string myTypeStr = myTypeFT->GetValue();
        if (    (myTypeStr != Symbol::TYPEERROR())
             && nl->ReadFromString( myTypeStr, li->myTypeNL)
           )
        {
          li->finished = false;
        }
      }
      qp->Open(args[0].addr);
      local.setAddr( li );
      return 0;

    case REQUEST:

      if(local.addr)
        li = (FTextValueMapGetValueNL_streamLocalInfo*) local.addr;
      else
      {
        return CANCEL;
      }
      if(li->finished)
      {
        return CANCEL;
      }
      qp->Request(args[0].addr, elem);
      if ( qp->Received(args[0].addr) )
      {
        ListExpr valueNL =
              SecondoSystem::GetCatalog()->OutObject(li->myTypeNL,elem);
        nl->WriteToString(valueStr,valueNL);
        Res = new FText(true, valueStr);
        result.setAddr( Res );
        ((Attribute*) elem.addr)->DeleteIfAllowed();
        return YIELD;
      }
      // stream exhausted - we are finished
      result.addr = 0;
      li->finished = true;
      return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      if(local.addr)
      {
        li = (FTextValueMapGetValueNL_streamLocalInfo*) local.addr;
        delete li;
        local.setAddr(0);
      }
      return 0;
  }
  /* should not happen */
  return -1;
}

// for tuple-streams
int FTextValueMapGetValueNL_tuplestream( Word* args, Word& result, int message,
                             Word& local, Supplier s )
{
  result          = qp->ResultStorage( s );
  FText* Res      = 0;
  FText* myTypeFT = static_cast<FText*>(args[1].addr);
  FTextValueMapGetValueNL_streamLocalInfo *li;
  li = static_cast<FTextValueMapGetValueNL_streamLocalInfo*>(local.addr);
  Tuple*          myTuple = 0;
  string valueStr = "";
  Word elem;

  switch( message )
  {
    case OPEN:
      if(li){
        delete li;
      }
      li = new FTextValueMapGetValueNL_streamLocalInfo;
      li->finished = true;
      if( myTypeFT->IsDefined() )
      {
        string myTypeStr = myTypeFT->GetValue();
        if (    (myTypeStr != Symbol::TYPEERROR())
             && nl->ReadFromString( myTypeStr, li->myTypeNL)
           )
        {
          li->myTypeNL = nl->OneElemList(li->myTypeNL);
          li->finished = false;
        }
      }
      qp->Open(args[0].addr);
      local.setAddr( li );
      return 0;

    case REQUEST:

      if(local.addr)
        li = (FTextValueMapGetValueNL_streamLocalInfo*) local.addr;
      else
      {
        return CANCEL;
      }
      if(li->finished)
      {
        return CANCEL;
      }
      qp->Request(args[0].addr, elem);
      if ( qp->Received(args[0].addr) )
      {
        myTuple = static_cast<Tuple*>(elem.addr);
        ListExpr valueNL = myTuple->Out( li->myTypeNL );
        nl->WriteToString(valueStr,valueNL);
        Res = new FText(true, valueStr);
        result.setAddr( Res );
        myTuple->DeleteIfAllowed();
        return YIELD;
      }
      // stream exhausted - we are finished
      result.addr = 0;
      li->finished = true;
      return CANCEL;

    case CLOSE:
      if(li){
         delete li;
         local.addr=0;
      }
      qp->Close(args[0].addr);
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
ValueMappingArray for ~replace~

*/

ValueMapping FText_VMMap_GetValueNL[] =
{
  FTextValueMapGetValueNL_tuplestream, // 0
  FTextValueMapGetValueNL_stream,      // 1
  FTextValueMapGetValueNL_single       // 2
};


/*
Selection function for ~replace~

*/

int FTextSelectFunGetValueNL( ListExpr args )
{
  NList type(args);
  if(    type.first().hasLength(2)
      && (type.first().first() == Symbol::STREAM())
      && (type.first().second().hasLength(2))
      && (type.first().second().first() == Tuple::BasicType())
    )
  {
    return 0; // tuplestream
  }
  if(     type.first().hasLength(2)
       && (type.first().first() == Symbol::STREAM())
    )
  {
    return 1; // datastream
  }
  return 2;     // normal object
}


/*

Type Mapping fucntion for toObject.



*/
ListExpr toObjectTM(ListExpr args){

  string err = "{text,string} x ANY [x bool] expected";
  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  ListExpr first = nl->First(args); 
  if(!CcString::checkType(first) && !FText::checkType(first)){
     return listutils::typeError(err);
  }
  // the second attribute can be any valid type
  if(!qp->IsCorrectTypeExpr(nl->Second(args))){
    return listutils::typeError("second arg is not a valid secondo type");
  }
  if(nl->HasLength(args,3)){
    if(!CcBool::checkType(nl->Third(args))){
       return listutils::typeError(err);
    }
  }
  return nl->Second(args);
}




/*
Value Mapping Function for Operator ~toObject~

*/

// Auxiliary function to get AlgebraId and Type ID from a Type-ListExpr
void FTextGetIds(int& algebraId, int& typeId, const ListExpr typeInfo)
{
  if(nl->IsAtom(typeInfo)) {
    return;
  }

  ListExpr b1 = nl->First(typeInfo);
  if(nl->IsAtom(b1)) {
    //typeInfo = type = (algId ...)
    if (nl->ListLength(typeInfo)!=2) {
      return;
    }
      //list = (algid typeid)
    algebraId = nl->IntValue(nl->First(typeInfo));
    typeId = nl->IntValue(nl->Second(typeInfo));
  } else {
    //typeInfo = (type1 type2).
    //We only need type1 since a collection can only be of one type.
    //type1 is b1 (nl->First(typeInfo)), so b1 is (algId typeId).
    if (nl->ListLength(b1)!=2) {
      return;
    }
      //b1 = (algId typeId)
    algebraId = nl->IntValue(nl->First(b1));
    typeId = nl->IntValue(nl->Second(b1));
  }
}

template<class T>
int FTextValueMapToObject( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  T* InText= static_cast<T*>(args[0].addr);
  result = qp->ResultStorage( s );
  bool isData = listutils::isDATA(qp->GetType(s));

  if(!InText->IsDefined())
  { // undefined text -> return undefined object
//     cout << __PRETTY_FUNCTION__ << " (" << __FILE__ << " line "
//          << __LINE__ << "): Text is undefined -> Undefined object."  << endl;
    if(isData){
       ((Attribute*)result.addr)->SetDefined(false);
    }
    return 0;
  }
  // extract the text
  string myText = InText->GetValue();
  // read nested list: transform nested-list-string to nested list
  ListExpr myTextNL;
  if (!nl->ReadFromString(myText, myTextNL) ) // HIER SPEICHERLOCH!
  {
//     cout << __PRETTY_FUNCTION__ << " (" << __FILE__ << " line "
//         << __LINE__ << "): Text does not produce a "
//         "valid nested list expression -> Undefined object."  << endl;
    if(isData){
       ((Attribute*)result.addr)->SetDefined(false);
    }
    return 0;
  }
  bool typeIncluded = false;   
  if(qp->GetNoSons(s)==3){
    CcBool* ti = (CcBool*) args[2].addr;
    if(ti->IsDefined() && ti->GetValue()){
        typeIncluded = true;
    }
  }

  if(typeIncluded){
     if(!nl->HasLength(myTextNL,2)){
        if(isData){
           ((Attribute*)result.addr)->SetDefined(false);
         }
         return 0;
     }
     ListExpr myTextType = nl->First(myTextNL);
     myTextNL = nl->Second(myTextNL);
     if(!nl->Equal(myTextType,qp->GetType(s))){
        if(isData){
           ((Attribute*)result.addr)->SetDefined(false);
         }
         return 0;
      }
  }



  // get information on resulttype
  ListExpr myTypeNL = qp->GetType( s );
  // call InFunction
  Word myRes( Address(0) );
  int errorPos = 0;
  ListExpr& errorInfo = nl->GetErrorList();
  bool correct = true;
  myRes = SecondoSystem::GetCatalog()->InObject( myTypeNL,
                                                 myTextNL,
                                                 errorPos,
                                                 errorInfo,
                                                 correct
                                               );
  if(!correct)
  {
//     cout << __PRETTY_FUNCTION__ << " (" << __FILE__ << " line "
//          << __LINE__ << "): InFunction failed -> Undefined object."  << endl;
//     cout << "\tmyTypeNL  = " << nl->ToString(myTypeNL) << endl;
//     cout << "\tmyTextNL  = " << nl->ToString(myTextNL) << endl;
//     cout << "\terrorInfo = " << nl->ToString(errorInfo) << endl;
//     cout << "\tErrorPos  = " << errorPos << endl;
   if(isData){
      ((Attribute*)result.addr)->SetDefined(false);
    }
    return 0;
  }
//   else
//   {
//     cout << __PRETTY_FUNCTION__ << " (" << __FILE__ << " line "
//         << __LINE__ << "): InFunction succeeded:"  << endl;
//     cout << "\tmyTypeNL  = " << nl->ToString(myTypeNL) << endl;
//     cout << "\tmyTextNL  = " << nl->ToString(myTextNL) << endl;
//     cout << "\tObject    = ";
//     ((Attribute*)(myRes.addr))->Print(cout); cout << endl;
//   }
  // Pass the Result
  qp->DeleteResultStorage(s);
  qp->ChangeResultStorage(s,myRes);
  result = qp->ResultStorage(s);
  return 0;
}

/*
ValueMappingArray for ~toObject~

*/

ValueMapping FText_VMMap_ToObject[] =
{
  FTextValueMapToObject<CcString>,   // 0
  FTextValueMapToObject<FText>,      // 1
};

/*
Selection function for ~toObject~

*/

int FTextSelectFunToObject( ListExpr args )
{
  NList type(args);
  if( type.first() == CcString::BasicType() )
    return 0;
  if ( type.first() == FText::BasicType() )
    return 1;
  return -1; // should not happen!
}

/*
Value Mapping Function for ~charT~

*/

int FTextValueMapChartext( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  CcInt* Cccode = static_cast<CcInt*>(args[0].addr);
  result = qp->ResultStorage( s );
  FText* res = static_cast<FText*>(result.addr);

  int code = 0;
  if ( !Cccode->IsDefined() )
    res->SetDefined( false );
  else{
    code = Cccode->GetIntval();
    if( (code >= 0) && (code <= 255) )
    {
      char ch = (char) code;
      ostringstream os;
      os << ch;
      string s = os.str();
      res->Set( true, s );
    }
    else
    { // illegal code --> return undef
      res->Set( false, "" );
    }
  }
  return 0;
}

/*

Operators ~toupper~, ~tolower~

*/
ListExpr changeCaseTM(ListExpr args){

  string err = "text or string expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
     return listutils::typeError(err);
  }
  return arg;
}



template<bool isToLower, class T>
int FTextValueMapChangeCaseT( Word* args, Word& result, int message,
                             Word& local, Supplier s )
{
  T* text = static_cast<T*>(args[0].addr);
  result = qp->ResultStorage( s );
  T* res = static_cast<T*>(result.addr);

  if ( !text->IsDefined() ){
    res->Set( false, "" );
  }else{
    string str = text->GetValue();
    if(isToLower){
      std::transform(str.begin(),str.end(), str.begin(), (int(*)(int)) tolower);
    }else{
      std::transform(str.begin(),str.end(), str.begin(), (int(*)(int)) toupper);
    }
    res->Set( true, str );
  }
  return 0;
}

/*
Value mappiung Array and Selection Function for ~toupper~, ~tolower~


*/

ValueMapping toLowerVM[] = {
    FTextValueMapChangeCaseT<true, CcString>,
    FTextValueMapChangeCaseT<true, FText>
};

ValueMapping toUpperVM[] = {
    FTextValueMapChangeCaseT<false, CcString>,
    FTextValueMapChangeCaseT<false, FText>
};

int changeCaseSelect(ListExpr args){
 return CcString::checkType(nl->First(args))?0:1;
}


/*
Value Mapping Function for ~tostring~, ~totext~

*/

template<bool isTextToString, class T1, class T2>
int FTextValueMapConvert( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  T1* inSec = static_cast<T1*>(args[0].addr);
  result = qp->ResultStorage( s );
  T2* res = static_cast<T2*>(result.addr);

  if( !inSec->IsDefined() )
  {
    res->Set( false, "" );
    return 0;
  }
  string inStr = inSec->GetValue();
  if(isTextToString)
  {
    string outStr = inStr.substr(0,MAX_STRINGSIZE);
    outStr = stringutils::replaceAll(outStr, "\"", "'");
    res->Set( true, outStr );
  }
  else
  {
    res->Set( true, inStr );
  }
  return 0;
}

/*
Operator ~sendtextUDP~

Send the text to a given host:port

Any status and error messages from the session are appended to the result text

*/

template<class T1, class T2, class T3, class T4, class T5>
int FTextValueMapSendTextUDP( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  result      = qp->ResultStorage( s );
  FText* Res  = static_cast<FText*>(result.addr);

  int no_args = qp->GetNoSons(s);
  ostringstream status;
  bool correct = true;

  // get message text (requiered)
  T1* CcMessage   = static_cast<T1*>(args[0].addr);
  string myMessage("");
  if(!CcMessage->IsDefined()){
    status << "ERROR: Message undefined.";
    correct = false;
  }else{
    myMessage = CcMessage->GetValue();
  }
  // get remote IP (requiered)
  T2* CcOtherIP   = static_cast<T2*>(args[1].addr);
  string OtherIP("");
  if (!CcOtherIP->IsDefined()){
    status << "ERROR: remoteIP undefined.";
    correct = false;
  }else{
    OtherIP = CcOtherIP->GetValue();
  }
  if(OtherIP == ""){
    status << "ERROR: remoteIP unspecified.";
    correct = false;
  }
  // get remote port number (requiered)
  T3* CcOtherPort = static_cast<T3*>(args[2].addr);
  string OtherPort("");
  if(!CcOtherPort->IsDefined()){
    status << "ERROR: remotePort undefined.";
    correct = false;
  }else{
    OtherPort = CcOtherPort->GetValue();
  }
  if(OtherPort == ""){
    status << "ERROR: remotePort unspecified.";
    correct = false;
  }
  // get sender IP (optional)
  string myIP("");
  if(no_args>=4){
    T4* CcMyIP      = static_cast<T4*>(args[3].addr);
    if (!CcMyIP->IsDefined()){
      status << "ERROR: localIP undefined.";
      correct = false;
    }else{
      myIP = CcMyIP->GetValue();
    }
  }
  // get sender port (requiered)
  string myPort("");
  if(no_args>=5){
    T5* CcMyPort    = static_cast<T5*>(args[4].addr);
    if(!CcMyPort->IsDefined()){
      status << "ERROR: localPort undefined.";
      correct = false;
    }else{
      myPort = CcMyPort->GetValue();
    }
  }
  if(myPort == ""){
    status << "ERROR: localPort unspecified.";
    correct = false;
  }
  // ensure diffrent sender and receiver address
  if( (myIP == OtherIP) && (myPort == OtherPort) ){
    status << "ERROR: sender and receiver address identical.";
    correct = false;
  }
  // return message on error due to any ERROR in parameters
  if(!correct){
    Res->Set( true, status.str() );
    return 0;
  }
  // define address struct for local socket:
  UDPaddress localAddress(myIP,myPort /*, AF_INET */);
  //cerr << "localAddress = " << localAddress << endl;
  if ( !localAddress.isOk() ) {
    status << "ERROR: Failed creating local address ("
           << localAddress.getErrorText() << ").";
    Res->Set( true, status.str() );
    return 0;
  }
  // define address struct for remote socket (destination)
  UDPaddress remoteAddress(OtherIP,OtherPort /*, AF_INET */);
  //cerr << "remoteAddress = " << remoteAddress << endl;
  if ( !remoteAddress.isOk() ) {
    status << "ERROR: Failed creating remote address ("
        << remoteAddress.getErrorText() << ").";
    Res->Set( true, status.str() );
    return 0;
  }
  // ensure diffrent sender and receiver address
  if(     ( localAddress.getIP() == remoteAddress.getIP() )
       && ( localAddress.getPort() == remoteAddress.getPort() ) ){
    status << "ERROR: sender and receiver address identical.";
    Res->Set( true, status.str() );
    return 0;
  }
  // create the local socket
  UDPsocket localSock(localAddress);
  if( !localSock.isOk() ){
    status << "ERROR: socket() failed ("
           <<  localSock.getErrorText() << ").";
    Res->Set( true, status.str() );
    return 0;
  }
  int msg_len = myMessage.length();
  cerr << "Trying to send " << msg_len << " bytes from IP "
       << localAddress.getIP() << " Port "
       << localAddress.getPort() << " to IP "
       << remoteAddress.getIP() << " Port "
       << remoteAddress.getPort() << endl;
  int sent_len = localSock.writeTo(remoteAddress,myMessage);
  if(sent_len < 0){
    status << "ERROR: sendto() failed (" << localSock.getErrorText() << ").";
  } else if (sent_len != msg_len){
    status << "WARNING: Message sent partially (" << sent_len << "/"
           <<msg_len<< " bytes).";
    Res->Set( true, status.str() );
    return 0;
  } else{
    status << "OK. " << sent_len << " bytes sent.";
  }
  // close socket and return status
  localSock.close();
  Res->Set( true, status.str() );
  return 0;
}

/*
ValueMappingArray for ~sendtext~

*/

ValueMapping FText_VMMap_MapSendTextUDP[] =
{
  FTextValueMapSendTextUDP<CcString,CcString,CcString,CcString,CcString>,  // 0
  FTextValueMapSendTextUDP<CcString,CcString,CcString,CcString,FText   >,  //
  FTextValueMapSendTextUDP<CcString,CcString,CcString,FText,   CcString>,  // 2
  FTextValueMapSendTextUDP<CcString,CcString,CcString,FText,   FText   >,  //
  FTextValueMapSendTextUDP<CcString,CcString,FText   ,CcString,CcString>,  // 4
  FTextValueMapSendTextUDP<CcString,CcString,FText   ,CcString,FText   >,  //
  FTextValueMapSendTextUDP<CcString,CcString,FText   ,FText,   CcString>,  // 6
  FTextValueMapSendTextUDP<CcString,CcString,FText   ,FText,   FText   >,  //
  FTextValueMapSendTextUDP<CcString,FText   ,CcString,CcString,CcString>,  // 8
  FTextValueMapSendTextUDP<CcString,FText   ,CcString,CcString,FText   >,  //
  FTextValueMapSendTextUDP<CcString,FText   ,CcString,FText,   CcString>,  // 10
  FTextValueMapSendTextUDP<CcString,FText   ,CcString,FText,   FText   >,  //
  FTextValueMapSendTextUDP<CcString,FText   ,FText   ,CcString,CcString>,  // 12
  FTextValueMapSendTextUDP<CcString,FText   ,FText   ,CcString,FText   >,  //
  FTextValueMapSendTextUDP<CcString,FText   ,FText   ,FText,   CcString>,  // 14
  FTextValueMapSendTextUDP<CcString,FText   ,FText   ,FText,   FText   >,  //
  FTextValueMapSendTextUDP<FText   ,CcString,CcString,CcString,CcString>,  // 16
  FTextValueMapSendTextUDP<FText   ,CcString,CcString,CcString,FText   >,  //
  FTextValueMapSendTextUDP<FText   ,CcString,CcString,FText,   CcString>,  // 18
  FTextValueMapSendTextUDP<FText   ,CcString,CcString,FText,   FText   >,  //
  FTextValueMapSendTextUDP<FText   ,CcString,FText   ,CcString,CcString>,  // 20
  FTextValueMapSendTextUDP<FText   ,CcString,FText   ,CcString,FText   >,  //
  FTextValueMapSendTextUDP<FText   ,CcString,FText   ,FText,   CcString>,  // 22
  FTextValueMapSendTextUDP<FText   ,CcString,FText   ,FText,   FText   >,  //
  FTextValueMapSendTextUDP<FText   ,FText   ,CcString,CcString,CcString>,  // 24
  FTextValueMapSendTextUDP<FText   ,FText   ,CcString,CcString,FText   >,  //
  FTextValueMapSendTextUDP<FText   ,FText   ,CcString,FText,   CcString>,  // 26
  FTextValueMapSendTextUDP<FText   ,FText   ,CcString,FText,   FText   >,  //
  FTextValueMapSendTextUDP<FText   ,FText   ,FText   ,CcString,CcString>,  // 28
  FTextValueMapSendTextUDP<FText   ,FText   ,FText   ,CcString,FText   >,  //
  FTextValueMapSendTextUDP<FText   ,FText   ,FText   ,FText,   CcString>,  // 30
  FTextValueMapSendTextUDP<FText   ,FText   ,FText   ,FText,   FText   >   // 31
};

/*
Selection function for ~sendtext~

*/

int FTextSelectSendTextUDP( ListExpr args )
{
  NList type(args);
  int noargs = nl->ListLength(args);
  int index = 0;
  if( noargs>=1 && type.first()==FText::BasicType() )
    index += 16;
  if( noargs>=2 && type.second()==FText::BasicType() )
    index += 8;
  if( noargs>=3 && type.third()==FText::BasicType() )
    index += 4;
  if( noargs>=4 && type.fourth()==FText::BasicType() )
    index += 2;
  if( noargs>=5 && type.fifth()==FText::BasicType() )
    index += 1;
  return index;
}

/*
Operator ~receivetextUDP~

Receive text from a remote host

*/

// template to convert from string
template<typename T>
bool FromString( const std::string& str, T& result )
{
  std::istringstream is(str);
  is >> result;
  if( !is ) {
    return false;
  }
  return true;
}

template<class T1, class T2>
int FTextValueMapReceiveTextUDP( Word* args, Word& result, int message,
                                 Word& local, Supplier s )
{
  bool *finished             = 0;

  CcBool *ccOk               = 0;
  FText *ccMsg               = 0;
  CcString *ErrMsg           = 0;
  CcString *SenderIP         = 0;
  CcString *SenderPort       = 0;
  CcString *SenderIPversion  = 0;
  TupleType *resultTupleType = 0;
  Tuple *newTuple            = 0;

  bool   m_Ok              = true;
  string m_Msg             = "";
  string m_ErrMsg          = "";

  T1* CcMyIP         = static_cast<T1*>(args[0].addr);
  T2* CcMyPort       = static_cast<T2*>(args[1].addr);
  CcReal* CcRtimeout = static_cast<CcReal*>(args[2].addr);

  string myIP("");
  string myPort("");
  double timeoutSecs = 0.0;
  int iMyPort = 0;
  ostringstream status;

  switch( message )
  {
    case OPEN:{
      finished = new bool(false);
      local.setAddr( finished );
      return 0;
    }
    case REQUEST:{
      // check whether already finished
      if (local.addr == 0){
        return CANCEL;
      }
      finished = (bool*) local.addr;
      if( *finished ){
        return CANCEL;
      }
      // get arguments
      if (!CcMyIP->IsDefined()){ // get own IP
        status << "LocalIP undefined. ";
        m_Ok = false;
      }else{
        myIP = CcMyIP->GetValue();
      }
      if(!CcMyPort->IsDefined()){ // get own port
        status << "LocalPort undefined. ";
        m_Ok = false;
      }else{
        myPort = CcMyPort->GetValue();
      }
      if( (!FromString<int> (myPort,iMyPort))
                || (iMyPort < 1024)
                || (iMyPort > 65536)) {
        status << "LocalPort " << iMyPort
               << " is no valid port number. ";
        m_Ok = false;
      }
      if(CcRtimeout->IsDefined()){ // get timeout
        timeoutSecs = CcRtimeout->GetRealval();
      }
      if(timeoutSecs > 0.0){
        cout << "INFO: receivetextUDP: Timeout = " << timeoutSecs
             << " secs." << endl;
      } else {
        cout << "INFO: receivetextUDP: No timeout." << endl;
      }
      UDPaddress localAddress;
      UDPaddress senderAddress;
      // define address for local datagram socket:
      if(m_Ok){
        localAddress = UDPaddress(myIP,myPort);
        if ( !(localAddress.isOk()) ) {
          status << localAddress.getErrorText() << ".";
          m_Ok = false;
        }
      }
      if(m_Ok){
      // create the socket
        UDPsocket my_socket(localAddress);
        if(!my_socket.isOk()){
          status << my_socket.getErrorText() << ".";
          m_Ok = false;
        }
      // bind socket to local port
        if(m_Ok && !my_socket.bind()){
          status << my_socket.getErrorText() << ".";
          m_Ok = false;
        }
      // receive a message
        if(m_Ok){
          m_Msg = my_socket.readFrom(senderAddress,timeoutSecs);
          if( !(my_socket.isOk()) ){
            status << my_socket.getErrorText();
            m_Ok = false;
          }
        }
        if (!my_socket.close()){
          status << my_socket.getErrorText() << ".";
          m_Ok = false;
        }
      // create result tuple
      m_ErrMsg = status.str(); // get error messages

      ccOk            = new CcBool(true, m_Ok);
      ccMsg           = new FText((m_Msg.length() > 0), m_Msg);
      ErrMsg          = new CcString(true, m_ErrMsg);
      SenderIP        = new CcString(senderAddress.getIP() != "",
                                     senderAddress.getIP());
      SenderPort      = new CcString(senderAddress.getPort() != "",
                                     senderAddress.getPort());
      SenderIPversion = new CcString(senderAddress.getFamily() != "",
                                     senderAddress.getFamily());
      resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      newTuple        = new Tuple( resultTupleType );
      newTuple->PutAttribute(  0,(Attribute*)ccOk);
      newTuple->PutAttribute(  1,(Attribute*)ccMsg);
      newTuple->PutAttribute(  2,(Attribute*)ErrMsg);
      newTuple->PutAttribute(  3,(Attribute*)SenderIP);
      newTuple->PutAttribute(  4,(Attribute*)SenderPort);
      newTuple->PutAttribute(  5,(Attribute*)SenderIPversion);
      result.setAddr(newTuple);
      *finished = true;

      // free created objects
      resultTupleType->DeleteIfAllowed();
      }
      return YIELD;
    }
    case CLOSE:{
      if (local.addr != 0){
        finished = (bool*) local.addr;
        delete finished;
        local.addr = 0;
      }
      return 0;
    }
  }
  /* should not happen */
  return -1;
}

ValueMapping FText_VMMap_MapReceiveTextUDP[] =
{
  FTextValueMapReceiveTextUDP<CcString,CcString>,  // 0
  FTextValueMapReceiveTextUDP<CcString,FText   >,  // 1
  FTextValueMapReceiveTextUDP<FText,   CcString>,  // 2
  FTextValueMapReceiveTextUDP<FText,   FText   >   // 3
};

// {string|text}^2 x real
int FTextSelectReceiveTextUDP( ListExpr args )
{
  NList type(args);
  int index = 0;
  if( type.first()==FText::BasicType() )
    index += 2;
  if( type.second()==FText::BasicType() )
    index += 1;
  return index;
}


/*
Operator ~receivetextstreamUDP~

*/

struct FTextValueMapReceiveTextStreamUDPLocalInfo{
  bool       finished;
  double     localTimeout;
  double     globalTimeout;
  bool       hasGlobalTimeout;
  bool       hasLocalTimeout;
  double     initial;
  double     final;
  UDPaddress localAddress;
  UDPsocket  my_socket;
  TupleType *resultTupleType;
};

template<class T1, class T2>
int FTextValueMapReceiveTextStreamUDP( Word* args, Word& result, int message,
                                       Word& local, Supplier s )
{
  FTextValueMapReceiveTextStreamUDPLocalInfo *li;

  CcBool    *ccOk             = 0;
  FText     *ccMsg            = 0;
  CcString  *ErrMsg           = 0;
  CcString  *SenderIP         = 0;
  CcString  *SenderPort       = 0;
  CcString  *SenderIPversion  = 0;
  Tuple     *newTuple         = 0;

  bool   m_Ok              = true;
  string m_Msg             = "";
  string m_ErrMsg          = "";

  T1* CcMyIP          = static_cast<T1*>(args[0].addr);
  T2* CcMyPort        = static_cast<T2*>(args[1].addr);
  CcReal* CcRltimeout = static_cast<CcReal*>(args[2].addr);
  CcReal* CcRgtimeout = static_cast<CcReal*>(args[3].addr);

  string myIP("");
  string myPort("");
  int iMyPort = 0;
  ostringstream status;

  UDPaddress senderAddress;

  double timeoutSecs = 0.0;

  switch( message )
  {
    case OPEN:{
      li = new FTextValueMapReceiveTextStreamUDPLocalInfo;
      li->finished      = true;
      li->localTimeout  = 0.0;
      li->globalTimeout = 0.0;
      li->localTimeout  = 0.0;
      li->globalTimeout = 0.0;
      time_t now;
      now = time(NULL);
      li->initial = now;
      li->final   = now;
      li->hasGlobalTimeout = false;
      li->hasLocalTimeout  = false;

      local.setAddr( li );

      li->resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));

      // get arguments
      if (!CcMyIP->IsDefined()){ // get own IP
        status << "LocalIP undefined. ";
        m_Ok = false;
      }else{
        myIP = CcMyIP->GetValue();
      }
      if(!CcMyPort->IsDefined()){ // get own port
        status << "LocalPort undefined. ";
        m_Ok = false;
      }else{
        myPort = CcMyPort->GetValue();
      }
      if( (!FromString<int> (myPort,iMyPort))
            || (iMyPort < 1024)
            || (iMyPort > 65536)) {
        status << "LocalPort " << iMyPort
            << " is no valid port number. ";
        m_Ok = false;
      }
      // local timeout
      if(CcRltimeout->IsDefined()){ // get timeout
        li->localTimeout = CcRltimeout->GetRealval();
      }
      if(li->localTimeout > 0.0){
        li->hasLocalTimeout = true;
        cout << "INFO: receivetextstreamUDP: Local Timeout = "
             << li->localTimeout
             << " secs." << endl;
      } else {
        li->hasLocalTimeout = false;
        cout << "INFO: receivetextstreamUDP: No local timeout." << endl;
      }
      // global timeout
      if(CcRgtimeout->IsDefined()){ // get timeout
        li->globalTimeout = CcRgtimeout->GetRealval();
      }
      if(li->globalTimeout > 0.0){
        li->hasGlobalTimeout = true;
        cout << "INFO: receivetextstreamUDP: Global Timeout = "
             << li->globalTimeout
             << " secs." << endl;
      } else {
        li->hasGlobalTimeout = false;
        cout << "INFO: receivetextstreamUDP: "
            << "No global timeout (Endless stream!)." << endl;
      }
      // correct timeout arguments
      li->localTimeout = min(li->localTimeout,li->globalTimeout);
      li->hasLocalTimeout = (li->localTimeout <= li->globalTimeout);
      if(li->hasGlobalTimeout){
        li->final = li->initial + li->globalTimeout;
      }
      // define address for local datagram socket:
      if(m_Ok){
        li->localAddress = UDPaddress(myIP,myPort);
        if ( !(li->localAddress.isOk()) ) {
          status << li->localAddress.getErrorText() << ".";
          m_Ok = false;
        }
      }
      if(m_Ok){
      // create the socket
        li->my_socket = UDPsocket(li->localAddress);
        if(!li->my_socket.isOk()){
          status << li->my_socket.getErrorText() << ".";
          m_Ok = false;
        }
      // bind socket to local port
        if(m_Ok && !li->my_socket.bind()){
          status << li->my_socket.getErrorText() << ".";
          m_Ok = false;
        }
      }
      li->finished = !m_Ok;
      if(!m_Ok){
        cerr << "ERROR: " << status.str() << endl;
      }
      return 0;
    }

    case REQUEST:{
      // check whether already finished
      if (local.addr == 0){
        return CANCEL;
      }
      li = (FTextValueMapReceiveTextStreamUDPLocalInfo*) local.addr;
      if( li->finished ){
        return CANCEL;
      }
      // handle global and local timeouts
      time_t now;
      now = time(NULL);
      if(li->hasGlobalTimeout && (li->final <= now) ){
        status << "Global Timeout.";
        li->finished = true;
        m_Ok = false;
      } else if(!li->hasGlobalTimeout && !li->hasLocalTimeout){
        timeoutSecs = 0.0; // blocking - wait forever
      } else if(li->hasGlobalTimeout && !li->hasLocalTimeout){
        timeoutSecs = (li->final - now); // remainder of global timeout
      } else if(!li->hasGlobalTimeout && li->hasLocalTimeout){
          timeoutSecs = li->localTimeout; // set local timeout;
      } else if(li->hasGlobalTimeout && li->hasLocalTimeout){
        timeoutSecs = min( li->localTimeout, (li->final - now) );
      } else {
        status << "ERROR: Something's wrong.";
        m_Ok = false;
        li->finished = true;
      }
      // receive a message
      if(m_Ok){
        m_Msg = li->my_socket.readFrom(senderAddress,timeoutSecs);
        if( !(li->my_socket.isOk()) ){
          status << li->my_socket.getErrorText();
          m_Ok = false;
        }
      }
      // create result tuple
      m_ErrMsg = status.str(); // get error messages
      ccOk            = new CcBool(true, m_Ok);
      ccMsg           = new FText((m_Msg.length() > 0), m_Msg);
      ErrMsg          = new CcString(true, m_ErrMsg);
      SenderIP        = new CcString(senderAddress.getIP() != "",
                                     senderAddress.getIP());
      SenderPort      = new CcString(senderAddress.getPort() != "",
                                     senderAddress.getPort());
      SenderIPversion = new CcString(senderAddress.getFamily() != "",
                                     senderAddress.getFamily());
      newTuple        = new Tuple( li->resultTupleType );
      newTuple->PutAttribute(  0,(Attribute*)ccOk);
      newTuple->PutAttribute(  1,(Attribute*)ccMsg);
      newTuple->PutAttribute(  2,(Attribute*)ErrMsg);
      newTuple->PutAttribute(  3,(Attribute*)SenderIP);
      newTuple->PutAttribute(  4,(Attribute*)SenderPort);
      newTuple->PutAttribute(  5,(Attribute*)SenderIPversion);
      result.setAddr(newTuple);
      return YIELD;
    }

    case CLOSE:{
      if (local.addr != 0){
        li = (FTextValueMapReceiveTextStreamUDPLocalInfo*) local.addr;
        if (!li->my_socket.close()){
          cerr << "ERROR: " << li->my_socket.getErrorText() << "." << endl;
        }
        li->resultTupleType->DeleteIfAllowed();
        li->resultTupleType = 0;
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  /* should not happen */
  return -1;
}


ValueMapping FText_VMMap_MapReceiveTextStreamUDP[] =
{
  FTextValueMapReceiveTextStreamUDP<CcString,CcString>,  // 0
  FTextValueMapReceiveTextStreamUDP<CcString,FText   >,  // 1
  FTextValueMapReceiveTextStreamUDP<FText,   CcString>,  // 2
  FTextValueMapReceiveTextStreamUDP<FText,   FText   >   // 3
};

/*
Operator ~letObject~

*/
template <class T1, class T2>
int ftextletObjectVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result      = qp->ResultStorage( s );
  FText *Res  = reinterpret_cast<FText*>(result.addr);

  T1* CCObjName  = static_cast<T1*>(args[0].addr);
  T2* CCommand   = static_cast<T2*>(args[1].addr);
  CcBool* CcIsNL = static_cast<CcBool*>(args[2].addr);
  string querystring       = "";
  string querystringParsed = "";
  string typestring        = "";
  string errorstring       = "";
  string ObjNameString     = "";
  Word queryresultword;

  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();

  SecParser mySecParser;
  ListExpr parsedCommand;

  // check definedness of all parameters
  if(    !CCObjName->IsDefined()
      || !CCommand->IsDefined()
      || !CcIsNL->IsDefined() ){
    Res->Set(false, "");
    return 0;
  }

  // check for a valid object name
  ObjNameString = CCObjName->GetValue();
  if( (ObjNameString == "") ){
    Res->Set(true, "ERROR: Empty object name.");
    return 0;
  } else if ( ctlg->IsSystemObject(ObjNameString) ) {
    Res->Set(true, "ERROR: Object name identifier is a reseverved identifier.");
    return 0;
  } else if (ctlg->IsObjectName(ObjNameString) ) {
    Res->Set(true, "ERROR: Object name identifier "
                      + ObjNameString + " is already used.");
    return 0;
  } else if(!ctlg->IsValidIdentifier(ObjNameString)){
    Res->Set(true, "ERROR: Object name " + ObjNameString +
                   " is not a valid identifier");
    return 0;
  }

  // try to create the value from the valeNL/valueText
  querystring = CCommand->GetValue();

  if( !CcIsNL->GetBoolval() ){ // Command in SecondoExecutableLanguage
        // call Parser: add "query" and transform expression
        //              to nested-list-string
        if(mySecParser.Text2List( "query " + querystring,
                                             querystringParsed ) != 0)
        {
          Res->Set(true, "ERROR: Value text does not contain a "
              "parsable value expression.");
          return 0;
        }

  } else {// Command is already a nested-list-string: just copy
    querystringParsed = querystring;
  }


  // read nested list: transform nested-list-string to nested list
  if (!nl->ReadFromString(querystringParsed, parsedCommand) ) {
    Res->Set(true, "ERROR: Value text does not produce a "
              "valid nested list expression.");
    return 0;
  }
  if ( !CcIsNL->GetBoolval() ) {
    // remove the "query" from the list
    if ( (nl->ListLength(parsedCommand) == 2) ){
          parsedCommand = nl->Second(parsedCommand);
          //string parsedCommandstr;
          //nl->WriteToString(parsedCommandstr, parsedCommand);
          //cout << "NLimport: OK. parsedCommand=" << parsedCommandstr  << endl;
    } else {
      Res->Set(true, "ERROR: Value text does not produce a "
                     "valid nested list expression.");
      return 0;
    }
  }
  // evaluate command
  OpTree tree = 0;
  ListExpr resultType;
//   if( !SecondoSystem::BeginTransaction() ){
//     Res->Set(true, "ERROR: BeginTranscation failed!");
//     return 0;
//   };
  QueryProcessor *qpp = new QueryProcessor( nl, am );

  try{
    bool correct        = false;
    bool evaluable      = false;
    bool defined        = false;
    bool isFunction     = false;
    Word qresult;
    //cerr <<  "Trying to build the operator tree" << endl;
    //cerr << " from " << nl->ToString(parsedCommand) << endl << endl;
    qpp->Construct( parsedCommand,
                   correct,
                   evaluable,
                   defined,
                   isFunction,
                   tree,
                   resultType );
    if ( !correct ){
      Res->Set(true, "ERROR: Value text yields a TYPEERROR.");
      // Do not need to destroy tree here!
      return 0;
    }
    typestring = nl->ToString(resultType);

    //cerr << "typeString is " << typestring << endl;
    if(!evaluable && !isFunction){
      Res->Set(true, "ERROR: Expression not evaluable and not a function");
      if(tree){
        qpp->Destroy(tree,true);
        tree = 0;
      }
      return 0;
    }

    if ( evaluable && !isFunction){
      qpp->EvalS( tree, qresult, 1 );
      if( IsRootObject( tree ) && !IsConstantObject( tree ) ){
        ctlg->CloneObject( ObjNameString, qresult );
        qpp->Destroy( tree, true );
      } else {
        ctlg->InsertObject( ObjNameString, "",resultType,qresult,true);
        qpp->Destroy( tree, false );
      }
      tree = 0;
    } else if ( isFunction ) { // abstraction or function object
      ctlg->CreateObject(ObjNameString, "", resultType, 0);
      if ( nl->IsAtom( parsedCommand ) ) { // function object
        ListExpr functionList = ctlg->GetObjectValue(
                nl->SymbolValue( parsedCommand ) );
        ctlg->UpdateObject( ObjNameString, SetWord( functionList ) );
      } else {
        ctlg->UpdateObject( ObjNameString, SetWord( parsedCommand ) );
      }
      if( tree ) {
        qpp->Destroy( tree, true );
        tree = 0;
      }
    }
    ctlg->CleanUp(true);
  } catch(SI_Error err) {
    if(tree) {
      qpp->Destroy( tree, true );
      tree = 0;
    }
    if( qpp ) {
      delete qpp;
      qpp = 0;
    }
    errorstring = "ERROR: " + SecondoInterface::GetErrorMessage(err);
//     if ( !SecondoSystem::AbortTransaction() ){
//       errorstring += ". AbortTransaction failed."
//     };
    Res->Set(true, errorstring);
    return 0;
  }
//   if( !SecondoSystem::CommitTransaction() ){
//     Res->Set(true, "ERROR: CommitTranscation failed!");
//     return 0;
//   };
  if( qpp ) {
    delete qpp;
    qpp = 0;
  }
  // Create object descriptor for the result FText
  string restring = "(OBJECT " + ObjNameString + " () (" + typestring + "))";
  Res->Set(true, restring);

  return 0;
}

// value mapping array
ValueMapping ftextletobject_vm[] = {
         ftextletObjectVM<CcString, CcString>,
         ftextletObjectVM<CcString, FText>,
         ftextletObjectVM<FText,    CcString>,
         ftextletObjectVM<FText,    FText> };

/*
Operator ~deleteObject~

*/

template<class T1>
int ftextdeleteObjectVM( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result               = qp->ResultStorage( s );
  FText *Res           = reinterpret_cast<FText*>(result.addr);
  T1* CCObjName        = static_cast<T1*>(args[0].addr);
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  string objName       = CCObjName->GetValue();
  string typestring    = "";

  if ( !CCObjName->IsDefined() ){
    Res->Set(false, "");
    return 0;
  } else if ( (objName == "") ){
    Res->Set(true, "ERROR: Empty object name.");
    return 0;
  } else if ( ctlg->IsSystemObject(objName) ) {
      Res->Set(true, "ERROR: Cannot delete system object " + objName + ".");
      return 0;
  } else if ( !ctlg->IsObjectName(objName) ) {
    Res->Set(true, "ERROR: Object " + objName + " is unknown.");
    return 0;
  } else {
    ListExpr typeExpr = ctlg->GetObjectTypeExpr( objName );
    typestring = nl->ToString(typeExpr);
    if ( !ctlg->DeleteObject( objName ) ){
      Res->Set(true, "ERROR: Object " + objName + " is unknown.");
      return 0;
    } else {
      // also delete from derived objects table if necessary
      DerivedObj *derivedObjPtr = new DerivedObj();
      derivedObjPtr->deleteObj( objName );
      delete derivedObjPtr;
    }
  }
  Res->Set(true,"(OBJECT " + objName +" () (" + typestring + "))");
  return 0;
}

// value mapping array
ValueMapping ftextdeleteobject_vm[] = {
         ftextdeleteObjectVM<CcString>,
         ftextdeleteObjectVM<FText> };

// Selection function
int ftextdeleteobjectselect( ListExpr args )
{
  NList type(args);
  if( type.first()==CcString::BasicType() )
    return 0;
  if( type.first()==FText::BasicType() )
    return 1;
  return -1;
}


int CheckOperatorTypeMapSelect(ListExpr args){
  return listutils::isSymbol(nl->Second(args),FText::BasicType())?0:1;
}

int strequal_select(ListExpr args){
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(listutils::isSymbol(first,CcString::BasicType())){
     if(listutils::isSymbol(second,CcString::BasicType())){
       return 0;
     } else {
       return 1;
     }
  }
  if(listutils::isSymbol(first,FText::BasicType())){
     if(listutils::isSymbol(second,CcString::BasicType())){
       return 2;
     } else {
       return 3;
     }
  }
  return -1;
}

/*
Operator ~createObject~

*/
template <class T1, class T2>
int ftextcreateObjectVM( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result      = qp->ResultStorage( s );
  FText *Res  = reinterpret_cast<FText*>(result.addr);

  T1* CCObjName  = static_cast<T1*>(args[0].addr);
  T2* CCommand   = static_cast<T2*>(args[1].addr);
  CcBool* CcIsNL = static_cast<CcBool*>(args[2].addr);
  string querystring       = "";
  string querystringParsed = "";
  string typestring        = "";
  string errorstring       = "";
  string ObjNameString     = "";
  Word queryresultword;

  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();

  SecParser mySecParser;
  ListExpr parsedCommand;

  // check definedness of all parameters
  if(    !CCObjName->IsDefined()
      || !CCommand->IsDefined()
      || !CcIsNL->IsDefined() ){
    Res->Set(false, "");
    return 0;
  }

  // check for a valid object name
  ObjNameString = CCObjName->GetValue();
  if( (ObjNameString == "") ){
    Res->Set(true, "ERROR: Empty object name.");
    return 0;
  } else if ( ctlg->IsSystemObject(ObjNameString) ) {
    Res->Set(true, "ERROR: Object name identifier is a reseverved identifier.");
    return 0;
  } else if (ctlg->IsObjectName(ObjNameString) ) {
    Res->Set(true, "ERROR: Object name identifier "
                      + ObjNameString + " is already used.");
    return 0;
  }

  // try to create the value from the valueNL/valueText
  querystring = CCommand->GetValue();

  if( !CcIsNL->GetBoolval() ){ // Command in SecondoExecutableLanguage
        // call Parser: add "query" and transform expression
        //              to nested-list-string
        if(mySecParser.Text2List( "query " + querystring,
                                             querystringParsed ) != 0)
        {
          Res->Set(true, "ERROR: Type text does not contain a "
              "parsable value expression.");
          return 0;
        }

  } else {// Command is already a nested-list-string: just copy
    querystringParsed = querystring;
  }
  // read nested list: transform nested-list-string to nested list
  if (!nl->ReadFromString(querystringParsed, parsedCommand) ) {
    Res->Set(true, "ERROR: Type text does not produce a "
              "valid nested list expression.");
    return 0;
  }
  if ( !CcIsNL->GetBoolval() ) {
    // remove the "query" from the list
    if ( (nl->ListLength(parsedCommand) == 2) ){
          parsedCommand = nl->Second(parsedCommand);
          //string parsedCommandstr;
          //nl->WriteToString(parsedCommandstr, parsedCommand);
          //cout << "NLimport: OK. parsedCommand=" << parsedCommandstr  << endl;
    } else {
      Res->Set(true, "ERROR: Type text does not produce a "
                     "valid nested list expression.");
      return 0;
    }
  }
  ListExpr typeExpr2 = ctlg->ExpandedType( parsedCommand );
  ListExpr errorList;
  string userDefTypeName = "";
  string typeExprString = nl->ToString(typeExpr2);
  if ( ctlg->KindCorrect( typeExpr2, errorList ) ) {
    if (    nl->IsAtom( parsedCommand )
         && ((nl->AtomType( parsedCommand ) == SymbolType))
       ) {
      userDefTypeName = nl->SymbolValue( parsedCommand );
      if ( !ctlg->MemberType( userDefTypeName ) ) { // not a user-defined type
          userDefTypeName = "";
      }
    }
    if ( !ctlg->CreateObject( ObjNameString, userDefTypeName, typeExpr2, 0 ) ){
      Res->Set(true, "ERROR: Object name identifier "
                        + ObjNameString + " is already used.");
      return 0;
    }
  } else { // Wrong type expression
      Res->Set(true, "ERROR: Invalid type expression.");
      return 0;
  }
  Res->Set(true, "(OBJECT " + ObjNameString +" () (" + typeExprString + "))");
  return -1;
}

// value mapping array
ValueMapping ftextcreateobject_vm[] = {
         ftextcreateObjectVM<CcString, CcString>,
         ftextcreateObjectVM<CcString, FText>,
         ftextcreateObjectVM<FText,    CcString>,
         ftextcreateObjectVM<FText,    FText>
};

/*
Operator ~getObjectTypeNL~

*/
template <class T1>
int ftextgetObjectTypeNL_VM( Word* args, Word& result, int message,
                                     Word& local, Supplier s )
{
  result      = qp->ResultStorage( s );
  FText *Res  = reinterpret_cast<FText*>(result.addr);

  T1* CCObjName  = static_cast<T1*>(args[0].addr);
  string ObjNameString     = "";
  string typestring        = "";
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();

  // check definedness of the parameters
  if( !CCObjName->IsDefined() ){
    Res->Set(false, "");
    return 0;
  }
  // check for a valid object name
  ObjNameString = CCObjName->GetValue();
  if( (ObjNameString == "") ){
    Res->Set(false, "");
    return 0;
  } else if ( !ctlg->IsObjectName(ObjNameString) ) {
    Res->Set(false, "");
    return 0;
  }
  // get the type expression
  typestring =
            nl->ToString(ctlg->GetObjectTypeExpr( ObjNameString ));
  // set result
  Res->Set(true, typestring);
  return 0;
}

// value mapping array
ValueMapping ftextgetObjectTypeNL_vm[] = {
         ftextgetObjectTypeNL_VM<CcString>,
         ftextgetObjectTypeNL_VM<FText>
};

/*
Operator ~getObjectValueNL~

*/
template <class T1>
int ftextgetObjectValueNL_VM( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  result      = qp->ResultStorage( s );
  FText *Res  = reinterpret_cast<FText*>(result.addr);

  T1* CCObjName  = static_cast<T1*>(args[0].addr);
  string ObjNameString     = "";
  string valuestring       = "";
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();

  // check definedness of the parameters
  if( !CCObjName->IsDefined() ){
    Res->Set(false, "");
    return 0;
  }
  // check for a valid object name
  ObjNameString = CCObjName->GetValue();
  if( (ObjNameString == "") ){
    Res->Set(false, "");
    return 0;
  } else if ( !ctlg->IsObjectName(ObjNameString) ) {
    Res->Set(false, "");
    return 0;
  }
  // get the value expression
  valuestring = nl->ToString(ctlg->GetObjectValue( ObjNameString ));
  // set result
  Res->Set(true, valuestring);
  return 0;
}

// value mapping array
ValueMapping ftextgetObjectValueNL_vm[] = {
         ftextgetObjectValueNL_VM<CcString>,
         ftextgetObjectValueNL_VM<FText>
};

/*
Operator ~getDatabaseName~

*/

int getDatabaseName_VM( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result         = qp->ResultStorage( s );
  CcString *Res  = reinterpret_cast<CcString*>(result.addr);
  string dbname  = SecondoSystem::GetInstance()->GetDatabaseName();
  Res->Set(true, dbname);
  return 0;
}

class matchingOpsLocalInfo{
 public:
    matchingOpsLocalInfo(ListExpr argList,
                         ListExpr attrList):pos(0),ops() {
       tupleType = new TupleType(attrList);
       ops = am->matchingOperators(argList);
    }

    matchingOpsLocalInfo(const int algId,
                         ListExpr argList,
                         ListExpr attrList):pos(0),ops() {
       tupleType = new TupleType(attrList);
       am->matchingOperators(algId, argList, ops);
    }

    matchingOpsLocalInfo(ListExpr argList):pos(0),ops(),tupleType(0) {
       ops = am->matchingOperators(argList);
    }

    CcString* nextName() {
      if( pos < ops.size() ) {
        Operator* op = am->GetOP(ops[pos].first.first, /** AlgebraId **/
                                 ops[pos].first.second /** OpId **/ );
        CcString* r = new CcString(op->GetName());
        pos++;
        return r;
      }
      return 0;
    }

    Tuple* nextTuple(){

        if(pos>=ops.size()){
          return 0;
        }
        pair< pair<int,int>, ListExpr> op = ops[pos];
        pos++;

        Tuple* res = new Tuple(tupleType);

        Operator* currop = am->GetOP(op.first.first, op.first.second);
        // opName
        res->PutAttribute(0, new CcString(currop->GetName()));
        // opId
        res->PutAttribute(1, new CcInt(true, op.first.second));
        // algName
        res->PutAttribute(2, new CcString(am->GetAlgebraName(op.first.first)));
        // algId
        res->PutAttribute(3, new CcInt(true, op.first.first));
        // resType
        ListExpr resultList = op.second;
        if(nl->HasLength(resultList,3) &&
           listutils::isSymbol(nl->First(resultList),Symbol::APPEND())){
           resultList = nl->Third(resultList);
        }
        res->PutAttribute(4, new FText(true,nl->ToString(resultList)));


        OperatorInfo oi = currop->GetOpInfo();
        // signature
        res->PutAttribute(5, new FText(true,oi.signature));
        // syntax
        res->PutAttribute(6, new FText(true,oi.syntax));

        // meaning
        res->PutAttribute(7, new FText(true,oi.meaning));

        // Example
        res->PutAttribute(8, new FText(true,oi.example));

        // Remark
        res->PutAttribute(9, new FText(true,oi.remark));
        return  res;
    }

    ~matchingOpsLocalInfo(){
      if(tupleType){
         tupleType->DeleteIfAllowed();
         tupleType = 0;
      }

    }



 private:
   size_t pos;                                   // used as iterator
   vector< pair< pair<int,int>, ListExpr> > ops; // ((AlgId,OpId) ResType)
   TupleType* tupleType;
};


template<bool onlyNames>
int matchingOperatorsVM( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
   switch(message){
     case OPEN: {
       if(local.addr){
         delete (matchingOpsLocalInfo*) local.addr;
         local.addr = 0;
       }
       int noSons = qp->GetNoSons(s);
       FText* t = (FText*) args[noSons-1].addr;
       ListExpr argList;
       nl->ReadFromString(t->GetValue(),argList);
       if(onlyNames){
          local.addr= new matchingOpsLocalInfo(argList);
       } else {
          local.addr= new matchingOpsLocalInfo(argList,
                               nl->Second(GetTupleResultType(s)));
       }
       return 0;
    }
    case REQUEST:{
       if(!local.addr){
          return CANCEL;
       }
       if(onlyNames){
          result.addr = ((matchingOpsLocalInfo*) local.addr)->nextName();
       } else {
          result.addr = ((matchingOpsLocalInfo*) local.addr)->nextTuple();
       }
       return result.addr?YIELD:CANCEL;
    }
    case CLOSE: {
       if(local.addr){
         delete (matchingOpsLocalInfo*) local.addr;
         local.addr = 0;
       }
       return 0;
    }

  }
  return -1;

}

template<bool getsNL, class T>
int sysgetMatchingOperatorsVM( Word* args, Word& result, int message,
                               Word& local, Supplier s )
{
   switch(message){
     case OPEN: {
       if(local.addr){
         delete (matchingOpsLocalInfo*) local.addr;
         local.addr = 0;
       }
       int noSons = qp->GetNoSons(s);
       CcInt* AlgId = 0;
       string val = "";
       if(getsNL){ // type list from normal argument
         AlgId = static_cast<CcInt*>(args[1].addr);
         T* v = static_cast<T*>(args[0].addr);
         if(!v->IsDefined()){
           return 0;
         }
         val = v->GetValue();
       } else { // typelist was appended in TypeMapping function
         AlgId = static_cast<CcInt*>(args[0].addr);
         FText* t = (FText*) args[noSons-1].addr;
         val = t->GetValue();
       }
       if(   AlgId->IsDefined()
          && (AlgId->GetValue()>0)
          && (AlgId->GetValue()<=am->getMaxAlgebraId()) ){
        ListExpr argList;
        nl->ReadFromString(val,argList);
        argList = nl->Rest(argList);
        local.addr= new matchingOpsLocalInfo(AlgId->GetValue(),
                                             argList,
                                             nl->Second(GetTupleResultType(s))
                                            );
       }
       return 0;
    }
    case REQUEST:{
       if(!local.addr){
          return CANCEL;
       } else {
          result.addr = ((matchingOpsLocalInfo*) local.addr)->nextTuple();
       }
       return result.addr?YIELD:CANCEL;
    }
    case CLOSE: {
       if(local.addr){
         delete (matchingOpsLocalInfo*) local.addr;
         local.addr = 0;
       }
       return 0;
    }

  }
  return -1;

}

// valuemapping array
ValueMapping sysgetMatchingOperators_vm[] = {
  sysgetMatchingOperatorsVM<false, FText>,
  sysgetMatchingOperatorsVM<true, CcString>,
  sysgetMatchingOperatorsVM<true, FText>
};

// selection function
int sysgetMatchingOperators_select(ListExpr args){
    NList type(args);
    if( type.first()==CcInt::BasicType() )
      return 0;
    if( type.first()==CcString::BasicType() )
      return 1;
    if( type.first()==FText::BasicType() )
      return 2;
    return -1;
}


/*
VM for operator ~sys\_getAlgebraName~

*/
int sys_getAlgebraNameVM( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);
  CcInt* arg = static_cast<CcInt*>(args[0].addr);
  if(!arg->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  int algId = arg->GetValue();
  if( (algId < 0) || (algId > am->getMaxAlgebraId()) )
  {
    res->Set(false,"UnknownAlgebra");
    return 0;
  }
  string algName = am->GetAlgebraName(algId);
  res->Set(algName!="UnknownAlgebra",algName);
  return 0;
}

/*
VM for operator ~sys\_getAlgebraId~

*/
template<class T>
int sys_getAlgebraIdVM( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  T* arg = static_cast<T*>(args[0].addr);
  if(!arg->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  int algId = am->GetAlgebraId(arg->GetValue()); // =0 indicates invalid algebra
  res->Set( algId>0 , algId );
  return 0;
}

// valuemapping array
ValueMapping sys_getAlgebraId_vm[] = {
  sys_getAlgebraIdVM<CcString>,
  sys_getAlgebraIdVM<FText>
};

// selection function
int sys_getAlgebraId_select(ListExpr args){
    NList type(args);
    if( type.first()==CcString::BasicType() )
      return 0;
    if( type.first()==FText::BasicType() )
      return 1;
    return -1;
}

/*
VM for operator ~checkTypeMap~

*/
template<class T>
int CheckOperatorTypeMapVM( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{

  CcString* opName = static_cast<CcString*>(args[0].addr);
  T* t = static_cast<T*>(args[qp->GetNoSons(s)-1].addr);
  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  if(!opName->IsDefined() || !t->IsDefined()){
    res->SetDefined(false);
  } else {
     ListExpr list;
     if(!nl->ReadFromString(t->GetValue(),list)){
       res->SetDefined(false);
     } else {

       ListExpr restype;
       int algid;
       int opid;
       bool found = am->findOperator(opName->GetValue(), list,
                                     restype, algid,opid);
       if(found){
           // remove append if found
           if(nl->HasLength(restype,3) &&
              listutils::isSymbol(nl->First(restype),Symbol::APPEND())){
              restype = nl->Third(restype);
           }
           res->Set(true,nl->ToString(restype));
       } else {
           res->SetDefined(false);
       }
     }
 }
 return 0;

}

ValueMapping CheckOperatorTypeMap_vm[] = {
         CheckOperatorTypeMapVM<FText>,
         CheckOperatorTypeMapVM<CcString>
};

template<class T1, class T2>
int strequalVM( Word* args, Word& result, int message,
                        Word& local, Supplier s ){

 result = qp->ResultStorage(s);
 CcBool* res = static_cast<CcBool*>(result.addr);
 T1* s1 = static_cast<T1*>(args[0].addr);
 T2* s2 = static_cast<T2*>(args[1].addr);
 CcBool* c = static_cast<CcBool*>(args[2].addr);
 if(!c->IsDefined()){
    res->SetDefined(false);
    return 0;
 }
 if(!s1->IsDefined() && !s2->IsDefined()){
    res->Set(true,true);
    return 0;
 }
 if(!s1->IsDefined() || !s2->IsDefined()){
    res->Set(true,false);
    return 0;
 }
 string str1 = s1->GetValue();
 string str2 = s2->GetValue();

 if(str1.length() != str2.length()){
    res->Set(true,false);
    return 0;
 }

 if(c->GetValue()){
    res->Set(true, str1==str2);
    return 0;
 }

 // compare two string case insensitive
 bool eq = true;
 for (string::const_iterator c1 = str1.begin(), c2 = str2.begin();
     (c1 != str1.end()) && eq;
     ++c1, ++c2) {
        if (tolower(*c1) != tolower(*c2)) {
            eq = false;
        }
 }
 res->Set(true,eq);
 return 0;
}



ValueMapping strequal_vm[] = {
    strequalVM<CcString,CcString>,
    strequalVM<CcString,FText>,
    strequalVM<FText,CcString>,
    strequalVM<FText,FText>,
};


/*
  tokenizeVM

*/
class tokenizeLI{
public:
  tokenizeLI(FText* text, CcString* delims){
     if(!text->IsDefined() || !delims->IsDefined()){
        st =0;
        return;
     }
     string d = delims->GetValue();

     d = stringutils::replaceAll(d,"\\n","\n");
     d = stringutils::replaceAll(d,"\\r","\r");
     // insert more replacements here if required
     d = stringutils::replaceAll(d,"\\\\","\\");


     st = new  stringutils::StringTokenizer(text->GetValue(), d);
  }

  ~tokenizeLI(){
      if(st){
         delete st;
         st = 0;
      }
  }

  FText* getNext(){
     if(!st){
       return 0;
     }
     if(!st->hasNextToken()){
        delete st;
        st = 0;
        return 0;
     }
     return new FText(true,st->nextToken());
  }

private:
   stringutils::StringTokenizer* st;
};

int tokenizeVM( Word* args, Word& result, int message,
                        Word& local, Supplier s ){

   switch(message){
     case OPEN:{
        if(local.addr){
           delete (tokenizeLI*) local.addr;
        }
        local.addr = new tokenizeLI(static_cast<FText*>(args[0].addr),
                                     static_cast<CcString*>(args[1].addr));
        return 0;
     }
     case REQUEST:{
        if(!local.addr){
          return CANCEL;
        }
        result.addr = ((tokenizeLI*)local.addr)->getNext();
        return result.addr?YIELD:CANCEL;
     }
     case CLOSE:{
        if(local.addr){
           delete (tokenizeLI*) local.addr;
           local.addr = 0;
        }
        return 0;
     }
   }
   return -1;
}

/*
Value Mapping for operator ~sendtextstreamTCP~

*/
template<class Elem_T>
class SendtextstreamTCP_LI {
  public:

    SendtextstreamTCP_LI(const string& _ip,
                         const string& _port,
                         const double& _timeout,
                         const double& _retries,
                         Word& _streamarg,
                         ListExpr _resulttype)
      : client(0),
        ok(true),
        inputstream(0),
        timeout((time_t)_timeout),
        retries((int)_retries),
        finished(false),
        restupletype(0){

      static MessageCenter* msgcenter = MessageCenter::GetInstance();

      // create socket/ connect to remote server
      retries = (_retries>=1)?_retries:1;
      timeout = (timeout>0)?_timeout:WAIT_FOREVER;
      client = Socket::Connect( _ip,
                                _port,
                                Socket::SockGlobalDomain,
                                retries,
                                timeout);
      if( !client || !client->IsOk() ){
        stringstream warning;
        warning << "Warning: Operator sendtextstreamTCP failed to "
           <<   "connect to server " << _ip << ":" << _port << " (timeout="
           << ( (int)timeout ) << "s, retries=" << retries << ").";
        msgcenter->Send(nl, nl->TwoElemList(nl->SymbolAtom("simple"),
                                        nl->TextAtom(warning.str())));
        ok = false;
        finished = true;
      } else { // stream is ok
//         client->GetSocketStream() << "(" << endl
//               << "(stream " << FText::BasicType() + ")" << endl;
        // open inputstream
        inputstream = _streamarg.addr;
        qp->Open(inputstream);
        // create result tuple type
        restupletype = new TupleType(nl->Second(_resulttype));
      }
    }

    ~SendtextstreamTCP_LI(){
      if(client){
//         if(ok) {
//           client->GetSocketStream() << ")"<< endl; // send end symbol
//         }
        bool shutdownok = client->ShutDown();  // shut down  socket
        bool closeok = client->Close(); // close socket
        if(shutdownok && closeok){
          delete client;
        } else {
          cerr << __PRETTY_FUNCTION__
               <<": CLOSE. Could not delete socket 'client'! Address = "
               << client << "." << endl;
        }
        client = 0;
      }
      if(inputstream){
        qp->Close(inputstream);            // close inputstream
        inputstream = 0;
      }
      // delete restupletype
      if(restupletype){
        restupletype->DeleteIfAllowed();        // drop result tupletype
        restupletype = 0;
      }
    }

    Tuple* next(){
      Word res;
      if(!ok || finished || !inputstream || !client || !client->IsOk() ){
        return 0;
      }
      Word elem;
      qp->Request(inputstream, elem);
      finished = !qp->Received(inputstream);
      if(finished){
        return 0;
      }
      CcBool* ok_cc = new CcBool(true, true);
      FText* msg_cc = new FText(true, "");
      CcString* errmsg_cc = new CcString(true, "");
      Elem_T* cc_elem = static_cast<Elem_T*>(elem.addr);
      if(!cc_elem->IsDefined()){
        ok_cc->Set(true, true);
        msg_cc->SetDefined(false);
      } else {
        string line = cc_elem->GetValue();
        client->GetSocketStream() << line << endl; // send it
        bool socketok = client->IsOk();
        ok_cc->Set(true, socketok);
        msg_cc->Set(true, line);
        errmsg_cc->Set(true, (socketok ? "Ok." : client->GetErrorText()) );
        if(!socketok){
          ok = false;
          finished = true;
        }
      }
      Tuple* restuple = new Tuple(restupletype);
      restuple->PutAttribute(  0,(Attribute*)ok_cc);
      restuple->PutAttribute(  1,(Attribute*)msg_cc);
      restuple->PutAttribute(  2,(Attribute*)errmsg_cc);
      cc_elem->DeleteIfAllowed(); // delete input stream elem
      return restuple;
    }

    bool isOk() {
      return ok;
    }

    bool isFinished() {
      return finished;
    }

  private:
    Socket* client;
    bool ok;           // =false means, that an error occured on the socket.
    void* inputstream;
    time_t timeout;
    int retries;
    bool finished;     // =true means, that no more elements will be delivered
    TupleType* restupletype;
};

template<class Elem_T, class RemoteIp_T, class RemotePort_T,
         class TimeOut_T, class Retries_T>
int sendtextstreamTCP_VM( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  SendtextstreamTCP_LI<Elem_T>* li = 0;

  switch(message){
    case OPEN:{
      if(local.addr){
        delete (SendtextstreamTCP_LI<Elem_T>*) local.addr;
      }
      RemoteIp_T*   cc_ip      = static_cast<RemoteIp_T*>(args[1].addr);
      RemotePort_T* cc_port    = static_cast<RemotePort_T*>(args[2].addr);
      TimeOut_T*    cc_timeout = static_cast<TimeOut_T*>(args[3].addr);
      Retries_T*    cc_retries = static_cast<Retries_T*>(args[4].addr);
      if( !cc_ip->IsDefined() ||
          !cc_port->IsDefined() ||
          !cc_timeout->IsDefined() ||
          !cc_retries->IsDefined()){
        local.setAddr(0);
      } else {
        li = new SendtextstreamTCP_LI<Elem_T>(cc_ip->GetValue(),
                                              cc_port->GetValue(),
                                              cc_timeout->GetValue(),
                                              cc_retries->GetValue(),
                                              args[0],
                                              GetTupleResultType(s));
        if(li->isOk()){
          local.addr = li;
        } else {
          delete li;
          local.setAddr(0);
        }
        return 0;
      }
    }
    case REQUEST:{
      if(!local.addr){
        result.setAddr(0);
        return CANCEL;
      }
      li = ((SendtextstreamTCP_LI<Elem_T>*)local.addr);
      if(li->isFinished()) {
        result.setAddr(0);
        return CANCEL;
      }
      result.setAddr(li->next());
      if(result.addr != 0){
        return YIELD;
      }
      return CANCEL;
    }
    case CLOSE:{
      if(local.addr){
        delete (SendtextstreamTCP_LI<Elem_T>*) local.addr;
        local.setAddr(0);
      }
      result.setAddr(0);
      return 0;
    }
  }
  return -1;
}

ValueMapping sendtextstream_vm[] = {
  sendtextstreamTCP_VM<CcString,CcString,CcString,CcInt,CcInt>,
  sendtextstreamTCP_VM<CcString,CcString,CcString,CcInt,CcReal>,
  sendtextstreamTCP_VM<CcString,CcString,CcString,CcReal,CcInt>,
  sendtextstreamTCP_VM<CcString,CcString,CcString,CcReal,CcReal>,
  sendtextstreamTCP_VM<CcString,CcString,FText,CcInt,CcInt>,
  sendtextstreamTCP_VM<CcString,CcString,FText,CcInt,CcReal>,
  sendtextstreamTCP_VM<CcString,CcString,FText,CcReal,CcInt>,
  sendtextstreamTCP_VM<CcString,CcString,FText,CcReal,CcReal>,
  sendtextstreamTCP_VM<CcString,FText,CcString,CcInt,CcInt>,
  sendtextstreamTCP_VM<CcString,FText,CcString,CcInt,CcReal>,
  sendtextstreamTCP_VM<CcString,FText,CcString,CcReal,CcInt>,
  sendtextstreamTCP_VM<CcString,FText,CcString,CcReal,CcReal>,
  sendtextstreamTCP_VM<CcString,FText,FText,CcInt,CcInt>,
  sendtextstreamTCP_VM<CcString,FText,FText,CcInt,CcReal>,
  sendtextstreamTCP_VM<CcString,FText,FText,CcReal,CcInt>,
  sendtextstreamTCP_VM<CcString,FText,FText,CcReal,CcReal>,
  sendtextstreamTCP_VM<FText,CcString,CcString,CcInt,CcInt>,
  sendtextstreamTCP_VM<FText,CcString,CcString,CcInt,CcReal>,
  sendtextstreamTCP_VM<FText,CcString,CcString,CcReal,CcInt>,
  sendtextstreamTCP_VM<FText,CcString,CcString,CcReal,CcReal>,
  sendtextstreamTCP_VM<FText,CcString,FText,CcInt,CcInt>,
  sendtextstreamTCP_VM<FText,CcString,FText,CcInt,CcReal>,
  sendtextstreamTCP_VM<FText,CcString,FText,CcReal,CcInt>,
  sendtextstreamTCP_VM<FText,CcString,FText,CcReal,CcReal>,
  sendtextstreamTCP_VM<FText,FText,CcString,CcInt,CcInt>,
  sendtextstreamTCP_VM<FText,FText,CcString,CcInt,CcReal>,
  sendtextstreamTCP_VM<FText,FText,CcString,CcReal,CcInt>,
  sendtextstreamTCP_VM<FText,FText,CcString,CcReal,CcReal>,
  sendtextstreamTCP_VM<FText,FText,FText,CcInt,CcInt>,
  sendtextstreamTCP_VM<FText,FText,FText,CcInt,CcReal>,
  sendtextstreamTCP_VM<FText,FText,FText,CcReal,CcInt>,
  sendtextstreamTCP_VM<FText,FText,FText,CcReal,CcReal>
};

int sendtextstream_Select(ListExpr args) {
  int res = 0;
  if(nl->ListLength(args)!=5){ return -1; }
  if(listutils::isSymbol(nl->Fifth(args),CcReal::BasicType())){ res +=1; }
  if(listutils::isSymbol(nl->Fourth(args),CcReal::BasicType())){ res +=2; }
  if(listutils::isSymbol(nl->Third(args),FText::BasicType())){ res +=4; }
  if(listutils::isSymbol(nl->Second(args),FText::BasicType())){ res +=8; }
  if(listutils::isSymbol(nl->Second(args),FText::BasicType())){ res +=16; }
  return res;
}


/*
4.25 Operator ~charToText~

*/
int charToTextFun( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText* res = static_cast<FText*>(result.addr);
  const CcInt* Cccode = static_cast<const CcInt*>(args[0].addr);

  int code = 0;
  if ( !Cccode->IsDefined() ){
    res->SetDefined( false );
    return 0;
  }
  code = Cccode->GetIntval();
  if ( (code<0) || (code>255) ) {
    // illegal code --> return undef
    res->Set( false , "");
    return 0;
  }
  char ch = (char) code;
  ostringstream os;
  os << ch;
  res->Set( true, os.str() );
  return 0;
}

/*
4.26 Operator ~attr2text~

*/
int attr2textVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  FText* res = static_cast<FText*>(result.addr);

  Attribute* attr = static_cast<Attribute*>(args[0].addr);

  stringstream ss;
  attr->Print(ss);
  res->Set(true,ss.str());
  return 0;
}


int isValidIDVM( Word* args, Word& result, int message,
                      Word& local, Supplier s ){

   const CcString* arg1 = static_cast<CcString*>(args[0].addr);
   const CcBool* arg2 = static_cast<CcBool*>(args[1].addr);
   result = qp->ResultStorage( s );
   CcBool* res = static_cast<CcBool*>(result.addr);
    
   if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
   } else {
     string errMsg;
     SecondoCatalog* ct = SecondoSystem::GetCatalog();
     string str = arg1->GetValue();
     bool checkObject = arg2->GetValue();
     res->Set(true, ct->IsValidIdentifier(str, errMsg, checkObject));
   }
   return 0;
}


/*
4.27 Value Mappings for trimAll

4.27.1 Simple cases stream<text> and stream<string>

*/
template<class T, bool undef>
int trimAllVM1(Word* args, Word& result, int message,
              Word& local,Supplier s){

  switch(message){
    case OPEN: {
                qp->Open(args[0].addr);
                return 0;
               }
    case REQUEST: {
                 Word elem;
                 qp->Request(args[0].addr,elem);
                 if(qp->Received(args[0].addr)){
                   T* r = ((T*) elem.addr)->Clone();
                   r->trim();
                   if(undef){
                     if(r->IsDefined() && r->Length()==0){
                        r->SetDefined(false);
                     }
                   }
                   result.addr = r;
                   ((T*)elem.addr)->DeleteIfAllowed();
                   return YIELD;
                 } else {
                   return CANCEL;
                 }
               }
     case CLOSE : {
                 qp->Close(args[0].addr);
                 return 0;
               }

  }
  return -1;
} 

/*

4.27.2 trimAll for a tupleStream

*/
template<bool undef>
class TrimAllInfo{
  public:

  TrimAllInfo(Word& _stream, Supplier s, Word* args): 
              stream(_stream), strings(){
    
     int count = qp->GetNoSons(s);

     for(int i=1; i< count; i += 2){
        pair<int, bool> p ( ((CcInt*)args[i].addr)->GetIntval(), 
                           ((CcBool*)args[i+1].addr)->GetBoolval());
        strings.push_back(p);
     }
     stream.open();
  }

  ~TrimAllInfo(){
      stream.close();
  }

  Tuple* nextTuple(){
    Tuple* src = stream.request();
    if(!src){
       return 0;
    }
    if(strings.size()==0u){ // nothing to do
      return src;
    }
    Tuple* res = new Tuple(src->GetTupleType());

    unsigned int vpos = 0;
    int npos = strings[vpos].first;

    for(int pos=0;pos<src->GetNoAttributes();pos++){
      if(pos==npos){
         bool isText = strings[vpos].second;
         if(isText){
            FText* t = ((FText*)src->GetAttribute(pos))->Clone();
            t->trim();
            if(undef){
               if(t->IsDefined() && (t->Length()==0)){
                   t->SetDefined(false);
               }
            }
            res->PutAttribute(pos,t);
         } else {
            CcString* s = ((CcString*)src->GetAttribute(pos))->Clone();
            s->trim();
            if(undef){
               if(s->IsDefined() && s->Length()==0){
                   s->SetDefined(false);
               }
            }
            res->PutAttribute(pos,s);
         }
         vpos++;
         if(vpos<strings.size()){
            npos = strings[vpos].first;
         }  else {
           npos = -1;
         }
      } else {
        res->CopyAttribute(pos, src, pos);
      }
    }
    src->DeleteIfAllowed();
    return res;
  }


  private:
    Stream<Tuple> stream;
    vector< pair <int , bool> > strings;
};

template<bool undef>
int trimAllVM2(Word* args, Word& result, int message,
              Word& local,Supplier s){


  TrimAllInfo<undef>* li = (TrimAllInfo<undef>*) local.addr;
  switch(message){
    case OPEN: {
                 if(li){
                   delete li;
                 }
                 local.addr = new TrimAllInfo<undef>(args[0], s, args);
                 return 0;
               }
    case REQUEST: {
                 if(!li){
                    return CANCEL;
                 }
                 result.addr = li->nextTuple();
                 return result.addr?YIELD:CANCEL;
               }
     case CLOSE : {
                  if(li){
                    delete li;
                    local.addr=0;
                  }
               }
  }
  return -1;
} 


/*
Vampue MApping array and Selection function.

*/

ValueMapping trimAllVM[] = { trimAllVM1<CcString, false>, 
                             trimAllVM1<FText, false>,
                             trimAllVM2<false> };

ValueMapping trimAllUndefVM[] = { trimAllVM1<CcString, true>, 
                             trimAllVM1<FText, true>,
                             trimAllVM2<true> };

int trimAllSelect(ListExpr args){
  ListExpr arg = nl->First(args);
  if(Stream<CcString>::checkType(arg)){
      return 0;
  }
  if(Stream<FText>::checkType(arg)){
     return 1;
  }
  return 2;
}



/*
3.4 Definition of Operators

Used to explain signature, syntax and meaning of the operators 
of the type ~text~.

*/

const string containsSpec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" )"
    "("
    "<text>({text | string)} x {text |string) -> bool</text--->"
    "<text>_a_ contains _b_</text--->"
    "<text>Returns whether _a_ contains pattern _g_.</text--->"
    ")"
  ")";


const string lengthSpec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" )"
    "("
    "<text>("+typeName+" ) -> int</text--->"
    "<text>length ( _ )</text--->"
    "<text>length returns the length of "+typeName+".</text--->"
    ")"
  ")";

const string keywordsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(text) -> (stream string)</text--->"
             "<text>_ keywords</text--->"
             "<text>Creates a stream of strings containing the single words"
             " of the origin text, on the assumption, that words in the text"
             " are separated by a space character.</text--->"
             "<text>let Keyword = documents feed "
             "extendstream[kword: .title keywords] consume</text--->"
             ") )";

const string sentencesSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(text) -> (stream text)</text--->"
             "<text>_ sentences</text--->"
             "<text>Creates a stream of standardized texts containing "
             "complete sentences"
             " of the origin text, on the assumption, that sentences "
             "in the text"
             " are terminated by a ., ! or ? character.</text--->"
             "<text>let MySentences = documents feed "
             "projectextendstream[title; newattr: .content sentences] "
             "consume</text--->"
             ") )";

const string diceSpec  =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                 "\"Example\" )"
            "( <text> int x text x text -> real </text--->"
             "<text> dice( _ _ _)</text--->"
             "<text>Computes the dice coefficient between the text using"
             " n-grams where n is speciefied by the first argument."
             " Can be used to compare texts .</text--->"
             "<text> dice(3  text1  text2) "
             "</text--->"
             ") )";

const string getCatalogSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> -> stream(tuple((ObjectName string)(Type string)"
    "(TypeExpr ftext)))</text--->"
    "<text>getCatalog( )</text--->"
    "<text>Returns all object descriptions from the catalog of the currently "
    "opened database as a tuple stream.</text--->"
    "<text>query getCatalog( ) consume</text--->"
    ") )";

const string substrSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> text x int x int -> string</text--->"
    "<text>substr( s, b, e )</text--->"
    "<text>Returns a substring of a text value, beginning at position 'b' "
    "and ending at position 'e', where the first character's position is 1. "
    "if (e - b)>48, the result will be truncated to its starting 48 "
    "characters.</text--->"
    "<text>query substr('Hello world!', 1, 3 ) consume</text--->"
    ") )";

const string subtextSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> text x int x int -> text</text--->"
    "<text>subtext( s, b, e )</text--->"
    "<text>Returns a subtext of a text value, beginning at position 'b' "
    "and ending at position 'e', where the first character's position is 1. "
    "</text--->"
    "<text>query subtext('Hello world!', 1, 3 )</text--->"
    ") )";

const string FTextfindSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> {text | string} x {text | string}  -> stream(int)</text--->"
    "<text>find( s, p )</text--->"
    "<text>Returns a stream of integers giving the starting positions of all "
    "occurances of a pattern 'p' within a string or text 's'. The position of "
    "the first character in 's' is 1. For any malformed parameter combination, "
    "the result is an empty stream.</text--->"
    "<text>query find('Hello world!', 'l') count</text--->"
    ") )";

const string FTextisemptySpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> {text | string} x {text | string}  -> stream(int)</text--->"
    "<text>isempty( t )</text--->"
    "<text>Returns TRUE, if text 't' is either undefined or empty.</text--->"
    "<text>query isempty('')</text--->"
    ") )";

const string FTexttrimSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> text -> text | string -> string</text--->"
    "<text>trim( t )</text--->"
    "<text>Removes whitespaces at the start and and end of"
    " the argument</text--->"
    "<text>query trim('  hello  you    ')</text--->"
    ") )";

const string FTextplusSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> {text | string} x text -> text \n"
    " text x {text | string -> text}</text--->"
    "<text>t1 + t2</text--->"
    "<text>Returns the concatenation of a combination of text with another "
    "text or string value.</text--->"
    "<text>query ('Hello' + \" world\" + '!')</text--->"
    ") )";

/*
Comparison predicates

*/
const string FTextLessSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text          x {text|string} -> bool\n"
    "{text|string} x text          -> bool</text--->"
    "<text>_ < _</text--->"
    "<text>Lexicographical ordering predicate 'less than'.</text--->"
    "<text>query 'TestA' < 'TestB'</text--->"
    ") )";

const string FTextLessEqSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text          x {text|string} -> bool\n"
    "{text|string} x text          -> bool</text--->"
    "<text>_ <= _</text--->"
    "<text>Lexicographical ordering predicate 'less than or equal'.</text--->"
    "<text>query 'TestA' <= 'TestB'</text--->"
    ") )";

const string FTextEqSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text          x {text|string} -> bool\n"
    "{text|string} x text          -> bool</text--->"
    "<text>_ = _</text--->"
    "<text>Lexicographical ordering predicate 'equals'.</text--->"
    "<text>query 'TestA' = 'TestB'</text--->"
    ") )";

const string FTextBiggerEqSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text          x {text|string} -> bool\n"
    "{text|string} x text          -> bool</text--->"
    "<text>_ >= _</text--->"
    "<text>Lexicographical ordering predicate 'greater than or equal'."
    "</text--->"
    "<text>query 'TestA' >= 'TestB'</text--->"
    ") )";

const string FTextBiggerSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text          x {text|string} -> bool\n"
    "{text|string} x text          -> bool</text--->"
    "<text>_ > _</text--->"
    "<text>Lexicographical ordering predicate 'greater than'.</text--->"
    "<text>query 'TestA' > 'TestB'</text--->"
    ") )";

const string FTextNeqSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text          x {text|string} -> bool\n"
    "{text|string} x text          -> bool</text--->"
    "<text>_ # _</text--->"
    "<text>Lexicographical ordering predicate 'nonequal to'.</text--->"
    "<text>query 'TestA' # 'TestB'</text--->"
    ") )";

/*
Special operators for subquery support

*/
const string FTextEvaluateSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text [ x bool ] -> stream(tuple((CmdStr text) (Success bool) "
    "(Evaluable bool) (Defined bool) (IsFunction bool)"
    "(ResultType text) (Result text) (ErrorMessage text) "
    "(ElapsedTimeReal real) (ElapsedTimeCPU real)))</text--->"
    "<text>evaluate( query , isNL )</text--->"
    "<text>Interprets the text argument 'query' as a Secondo Executable "
    "Language query expression and evaluates it. The calculated result "
    "returned as a nested list expression. Operator's result is a stream "
    "containing at most 1 tuple with a copy of the command, the result, "
    "errormessage, runtimes, and some more status information. If the optional "
    "second argument 'isNL' (default = FALSE) is set to TRUE, 'query' is "
    "expected to be in nested list format. </text--->"
    "<text>query evaluate('ten feed filter[.no > 5] count') "
    "filter[.Success] count > 1</text--->"
    ") )";

const string FTextReplaceSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>{text|string} x {text|string} x {text|string} -> text\n"
    "{text|string} x int    x int  x {text|string} -> text</text--->"
    "<text>replace( text , Pold, Pnew )\n replace( text, start, end, Pnew)"
    "</text--->"
    "<text>Within 'text', replace all occurencies of pattern 'Pold' by "
    "pattern 'Pnew'. Within 'text' replace characters starting at "
    "position 'start' and ending at position 'end' by pattern 'Pnew')</text--->"
    "<text>query replace('Fish! Fresh fish! Fresh fished fish!',"
    "'Fresh','Rotten')</text--->"
    ") )";

const string FTextIsDBObjectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>string -> bool</text--->"
    "<text>isDBObject( Name )</text--->"
    "<text>Returns true iff Name is the name of a database object.</text--->"
    "<text>query isDBObject(\"blubb\")</text--->"
    ") )";

const string FTextGetTypeNLSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>Expression -> text</text--->"
    "<text>_ getTypeNL</text--->"
    "<text>Retrieves the argument's type as a nested list expression.</text--->"
    "<text>query int getTypeNL</text--->"
    ") )";

const string FTextGetValueNLSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(DATA)   -> stream(text)\n"
    "stream(tuple(X)) -> stream(text)\n"
    "Expression       -> text</text--->"
    "<text>_ getValueNL</text--->"
    "<text>Returns the argument's nested list value expression as a text "
    "value. If the argument is a stream, a stream of text values with the "
    "textual representations for each stream element is produced.</text--->"
    "<text>query ten feed getValueNL</text--->"
    ") )";

const string FTextToObjectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>{text|string} x T [ x bool ]-> T, T in ANY</text--->"
    "<text>toObject( ValueList, TypePattern, typeIncluded )</text--->"
    "<text>Creates an object of type T from a nested list expression "
    "'ValueList'. Argument 'TypePattern' is only needed to determine the type "
    "T of the result mapping, its value will otherwise be be ignored. 'list' "
    "is a text whose value is a valid nested list value expression for type T."
    "If the value expression does not match the type of 'TypePattern', the "
    "result is a default object or undefined in case of an DATA type. "
    "If the optional boolean argument is given and true, the expected list"
    " format is (<type> <value>)."
    "</text--->"
    "<text>query toObject('3.141592653589793116', 1.0)</text--->"
    ") )";

/*
Type conversion operations

*/
const string FTextChartextSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>int -> text</text--->"
    "<text>chartext( code )</text--->"
    "<text>Creates a text of length 1 containing the character specified by "
    "ASCII symbol code 'code'.</text--->"
    "<text>query chartext(13)"
    "</text--->"
    ") )";

const string FTextToLowerSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text -> text</text--->"
    "<text>tolower( text )</text--->"
    "<text>Creates a copy of 'text', where upper case characters are replaced "
    "by lower case characters.</text--->"
    "<text>query tolower('Hello World!')"
    "</text--->"
    ") )";

const string FTextToUpperSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text -> text</text--->"
    "<text>toupper( text )</text--->"
    "<text>Creates a copy of 'text', where lower case characters are replaced "
    "by upper case characters.</text--->"
    "<text>query toupper('Hello World!')"
    "</text--->"
    ") )";

const string FTextTextToStringSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text -> string</text--->"
    "<text>tostring( text )</text--->"
    "<text>Converts 'text' to a string value. One the first 48 characters are "
    "copied. Any contained doublequote (\") is replaced by a single quote ('). "
    "</text--->"
    "<text>query tostring('Hello World!')"
    "</text--->"
    ") )";

const string FTextStringToTextSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>string -> text</text--->"
    "<text>totext( string )</text--->"
    "<text>Converts 'string' to a text value.</text--->"
    "<text>query totext(\"Hello World!\")"
    "</text--->"
    ") )";

/*
Operations for sending and receiving text via UDP/IP

*/
const string FTextSendTextUDPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>{string|text}^n -> text, 3<=n<=5</text--->"
    "<text>sendtextUDP( message, remoteIP, remotePort [, myIP [, myPort] ] )"
    "</text--->"
    "<text>Sends 'message' to 'remotePort' to host 'remoteIP' using 'myPort' "
    "on 'myIP' as the sender address/port and returns a text with a send "
    "status report. If optional parameters are omitted or are empty, standard "
    "parameters will be used automatically. DNS is used to lookup for host"
    "names. Uses the UDP connection-less protocol.</text--->"
    "<text>query sendtextUDP(\"Hello World!\", '127.0.0.0', '2626')</text--->"
    ") )";

const string FTextReceiveTextUDPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>{string|text} x {string|text} x real -> stream(tuple("
    "(Ok bool)(Msg text)(ErrMsg string)(SenderIP string)(SenderPort string)"
    "(SenderIPversion string)))</text--->"
    "<text>receivetextUDP( myIP, myPort, timeout )"
    "</text--->"
    "<text>Tries to receive a UDP-message to 'myPort' under local address "
    "'myIP' for a duration up to 'timeout' seconds. Parameter 'myIP' "
    "is looked up automatically, being an empty string/text. DNS is used to "
    "lookup for host names. Negative 'timeout' values will deactive the "
    "timeout, possibly waiting forever. The result is a stream with a single "
    "tuple containing an OK-flag, the message, an error message, the "
    "sender's IP and port, and its IP-version.</text--->"
    "<text>query receivetextUDP(\"\",'2626',0.01) tconsume</text--->"
    ") )";

const string FTextReceiveTextStreamUDPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>{string|text} x {string|text} x real x real -> stream(tuple("
    "(Ok bool)(Msg text)(ErrMsg string)(SenderIP string)(SenderPort string)"
    "(SenderIPversion string)))</text--->"
    "<text>receivetextstreamUDP( myIP, myPort, localTimeout, globalTimeout )"
    "</text--->"
    "<text>Tries to receive a series of UDP-messages to 'myPort' under local "
    "address 'myIP' for a duration up to 'localTimeout' seconds each. "
    "Terminates after a total time of 'globalTimeout'. Parameter 'myIP' "
    "is looked up automatically, being an empty string/text. DNS is used to "
    "lookup for host names. Negative 'timeout' values will deactive the "
    "timeout, possibly waiting forever. The result is a stream with a single "
    "tuple containing an OK-flag, the message, an error message, the "
    "sender's IP and port, and its IP-version. Can also be used to create a "
    "sequence of 'events' with a selected frequecy. You can use the 'cancel' "
    "operator to wait for 'magic messages' terminating the stream.</text--->"
    "<text>query receivetextstreamUDP(\"\",'2626',1.0, 10.0) tconsume</text--->"
    ") )";


/*
Operations for conversion of text to/from svg

*/
const string svg2textSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>svg -> text</text--->"
    "<text>svg2text( svg )</text--->"
    "<text>Converts 'svg' to a text value.</text--->"
    "<text>query svg2text(...)"
    "</text--->"
    ") )";

const string text2svgSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text -> svg</text--->"
    "<text>text2svg( text )</text--->"
    "<text>Converts 'text' to an svg  value.</text--->"
    "<text>query text2svg(...)"
    "</text--->"
    ") )";

/*
Operations for en-/decryption of texts

*/
const string cryptSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> t1 [x t2] -> string, t1,t2 in {string, text} </text--->"
    "<text>crypt( word [, salt] )</text--->"
    "<text>encrypt word using the DES crypt</text--->"
    "<text>query crypt(\"TopSecret\",\"TS\")"
    "</text--->"
    ") )";

const string checkpwSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> t1 x t2 -> bool, t1,t2 in {string, text} </text--->"
    "<text>checkpw( plain, encrypted )</text--->"
    "<text>checks whether encrypted is an encrypted version of "
    " plain using the crypt function </text--->"
    "<text>query checkpw(\"Secondo\",crypt(\"Secondo\"))"
    "</text--->"
    ") )";

const string md5Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string, text} [x {string, text}]  -> string </text--->"
    "<text>md5( word [, salt]  )</text--->"
    "<text>encrypt word using the md5 encryption</text--->"
    "<text>query md5(\"TopSecret\")"
    "</text--->"
    ") )";

const string blowfish_encodeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string, text} x {string, text}  -> text </text--->"
    "<text>blowfish_encode( password, text )</text--->"
    "<text>encrypt text using the blowfish encryption</text--->"
    "<text>query blowfish_encode(\"TopSecret\",\"Secondo\")"
    "</text--->"
    ") )";

const string blowfish_decodeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string, text} x {string, text}  -> text </text--->"
    "<text>blowfish_decode( password, hex )</text--->"
    "<text>decrypt hex using the blowfish decryption</text--->"
    "<text>query blowfish_decode(\"TopSecret\",\"f27d7581d1aaaff\")"
    "</text--->"
    ") )";

/*
More operations for subquery support

*/
const string ftextletObjectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string|text} x {string|text} x bool  -> text </text--->"
    "<text>letObject( objectName, valueNL, isNL )</text--->"
    "<text>Create a database object as a side effect. The object's name is "
    "objectName, and it is set to value valueNL. Parameter isNL indicates "
    "whether valueNL is given in NL syntax (TRUE) or text syntax (FALSE). "
    "The operator acts like the 'let object' command. The return value is a "
    "object descriptor (OBJECT () (<typeNL>) (<valueNL>)) is assignment was "
    "successful, an error message otherwise.</text--->"
    "<text>query letObject(\"MyThreeInt\",'3', FALSE)"
    "</text--->"
    ") )";

const string ftextdeleteObjectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string|text} -> text </text--->"
    "<text>deleteObject( objectName )</text--->"
    "<text>Deletes a database object as a side effect. The object is named "
    "objectName. You should NEVER try to delete an object, that is used by the "
    "query trying to delete it. If deletion fails, an error message is "
    "returned, otherwise a descriptor (OBJECT () (<typeNL>) (<valueNL>)) for "
    "the deleted object is returned.</text--->"
    "<text>query deleteObject(\"MyThreeInt\")"
    "</text--->"
    ") )";

const string ftextcreateObjectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string|text} x {string|text} x bool -> text </text--->"
    "<text>createObject( objectName, typeexpr, isNL )</text--->"
    "<text>Creates a database object as a side effect. The object is named "
    "objectName. It's type is given by typeexpr. The boolean parameter must be "
    "set to TRU, iff the typeexpr is given in NL-syntax, and FALSE when in "
    "text sxntax. If creation fails, an error message is returned, otherwise a "
    "descriptor (OBJECT () (<typeNL>) (<valueNL>)) for the created "
    "object.</text--->"
    "<text>query createObject(\"MyThreeInt\", 'int', TRUE)"
    "</text--->"
    ") )";

const string ftextgetObjectTypeNLSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string|text} -> text </text--->"
    "<text>getObjectTypeNL( objectName )</text--->"
    "<text>Returns a text with the NL type expression associated with the "
    "database object with the name given as a text or string argument. "
    "If the according object does not exist, the result is undefined.</text--->"
    "<text>query 'getObjectTypeNL(\"MyThreeInt\")"
    "</text--->"
    ") )";

const string ftextgetObjectValueNLSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string|text} -> text </text--->"
    "<text>getObjectValueNL( objectName )</text--->"
    "<text>Returns a text with the NL value expression associated with the "
    "database object with the name given as a text or string argument. "
    "If the according object does not exist, the result is undefined.</text--->"
    "<text>query 'getObjectValueNL(\"MyThreeInt\")"
    "</text--->"
    ") )";

const string getDatabaseNameSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>-> string </text--->"
    "<text>getDatabaseName()</text--->"
    "<text>Returns a string with the name of the current database.</text--->"
    "<text>query getDatabaseName()</text--->"
    ") )";

/*
Operations for checking type mapping and inquiries on available
type constructors and operators.

*/
const string matchingOperatorsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>  ANY -> stream(tuple(( \n"
    "          OperatorName: string,\n"
    "           OperatorId : int,\n"
    "          AlgebraName : string,\n"
    "            AlgebraId : int,\n"
    "           ResultType : text,\n"
    "           Signature  : text,\n"
    "               Syntax : text,\n"
    "              Meaning : text,\n"
    "              Example : text,\n"
    "               Remark : text)))\n"
    " </text--->"
    "<text> query matchingOperators(arg1, arg2, ...) </text--->"
    "<text>Returns the operators which could be applied to the"
    " types coming from "
    "  evaluation of the arguments  </text--->"
    "<text>query matchingOperators(Trains) tconsume</text--->"
    ") )";


const string sysgetMatchingOperatorsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>  int x ANY -> R, \n"
    "{string|text} x int -> R, where R = stream(tuple(( \n"
    "          OperatorName: string,\n"
    "           OperatorId : int,\n"
    "          AlgebraName : string,\n"
    "            AlgebraId : int,\n"
    "           ResultType : text,\n"
    "           Signature  : text,\n"
    "               Syntax : text,\n"
    "              Meaning : text,\n"
    "              Example : text,\n"
    "               Remark : text)))\n"
    " </text--->"
    "<text> sys_getMatchingOperators( AlgId, arg1, arg2, ... ) \n"
    "sys_getMatchingOperators( TypeNL, AlgId ) </text--->"
    "<text>Returns the operators from Algebra AlgId, which could be applied to "
    "the types coming from evaluation of the remaining arguments. Alternatively"
    ", the text/string argument TypeNL contains the nested list representation "
    "of the argument list as a string/text.</text--->"
    "<text>query sys_getMatchingOperators(0, \"Hallo\") tconsume;\n"
    "query sys_getMatchingOperators('int', 0) tconsume</text--->"
    ") )";

const string sys_getAlgebraNameSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>int -> string</text--->"
    "<text>sys_getAlgebraName( AlgId )</text--->"
    "<text>Returns the name of the algebra module indicated by the number "
    "AlgId. If AlgId is not positive or not associated with an algebra module, "
    "the result is UNDEF.</text--->"
    "<text>query sys_getAlgebraName(1)</text--->"
    ") )";

const string sys_getAlgebraIdSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>{string|text} -> int</text--->"
    "<text>sys_getAlgebraId( AlgebraName )</text--->"
    "<text>Returns the numeric AlgebraId associated with the algebra module "
    "name indicated by AlgebraName. If no algebra is associated with that "
    "name, the result is UNDEF.</text--->"
    "<text>query sys_getAlgebraId('StandardAlgebra')</text--->"
    ") )";

const string matchingOperatorNamesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>  ANY -> stream(string) </text--->"
    "<text> matchingOperatorNames( Arg1, Arg2, ... ) </text--->"
    "<text>Returns the names of all operators which could be "
    "applied to a parameter list having the types resulting from the "
    "evaluation of the arguments.</text--->"
    "<text>query matchingOperatorNames(Trains) count</text--->"
    ") )";

const string CheckOperatorTypeMapSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>  string x ANY -> text </text--->"
    "<text> checkOperatorTypeMap( Opname, Arg1, ... ) </text--->"
    "<text>Checks whether an operator Opname exists that can "
    "process the arguments Arg_1, ..., Arg_n."
    " The resulting text contains the operator's result type as a nested list. "
    "It is undefined if no matching signature exists for the Operator."
    " </text--->"
    "<text>query checkOperatorTypeMap(\"+\",1,1) count</text--->"
    ") )";


const string CheckOperatorTypeMap2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>  string x {text | string}  -> text </text--->"
    "<text> checkOperatorTypeMap2(Opname, Arguments) </text--->"
    "<text>Checks whether an operator Opname exists that can be process the "
    "given Arguments. Arguments is a text value containing a nested list "
    "describing the argument types (a left handside of a valid signature for "
    "the Operator). "
    "The resulting text contains the type of the result as a nested list. "
    "The result is undefined if no such an operator exists.</text--->"
    "<text>query checkOperatorTypeMap2(\"+\",\"int int\") count</text--->"
    ") )";


const string strequalSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {string, text} x {string, text} x bool -> bool </text--->"
    "<text>strequals( S1, S2, IsCaseSensitive)</text--->"
    "<text>Checks whether two strings (string or text) S1, S2 are equal. "
    "The third argument controls whether the comparison is case sensitivity "
    "(TRUE), or case insensitive (FALSE)."
    "Two undefined strings are always equal. If the boolean value "
    "is undefined, also the result is undefined.</text--->"
    "<text>query strequal(\"hello\",'HELLO',FALSE) </text--->"
    ") )";

const string tokenizeSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text x string -> stream(text) </text--->"
    "<text> tokenize(Text, Delimiters) </text--->"
    "<text>Splits the Text at the given Delimiters and returns "
    "each found token as an stream element. 'Delimiters' is a concatenation of "
    "all character symbols to be interpreted as delimiters."
    " </text--->"
    "<text>query tokenize('Hello World', \" \") count </text--->"
    ") )";

const string sendtextstreamTCP_Spec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> stream({string|text}) x {string|text} x  {string|text} x "
    "{int|real} x {int|real} "
    "-> stream(tuple((Ok bool)(Msg text)(ErrMsg string))), 2<=n<=4 </text--->"
    "<text> _ sendtextstreamTCP[RemoteIP,RemotePort,TimeOut,Retries] "
    "</text--->"
    "<text>Tries to establish a TCP/IP connection to the specified server "
    "when the result stream is OPENED. If no connection is etsablished"
    "within 'Retries' attempts with a timeout of 'TimeOut' seconds, "
    "the result stream is empty. Negative TimeOut deactivates the timeout, "
    "waiting for the server's rely indefinitively. Less then 1 Retries are "
    "mapped to 1 attempt.\n"
    "When requesting a result tuple, one input stream element is sent over the "
    "link to the server and a tuple is put on the result stream, with 'Ok' = "
    "TRUE, 'Msg' containing the input stream element, and 'ErrMsg' = \"Ok\". "
    "UNDEFINED stream elements are not sent, but a result tuple with "
    "'Ok'=TRUE, 'Msg'=UNDEFINED, 'ErrMsg'=\"Ok. Did not transmit UNDEFINED "
    "element.\" is created.\n"
    "When the input stream exhausts, no more results are created. The "
    "connection is closed when an error occurs or the result stream is closed. "
    "Messages from the remote server are ignored. If the link gets broken or "
    "some other error occurs during transfer, for the according result stream "
    "element 'Ok' is set to FALSE, and 'ErrMsg' contains an according error "
    "message. No further tuples are written to the result stream. Invalid "
    "arguments result in an empty stream.</text--->"
    "<text>query 'Hello World!' feed sendtextstreamTCP['localhost', '4387', 2.0"
    ", 3] count = 0</text--->"
    ") )";

const string charToText_Spec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> int -> text </text--->"
    "<text> charToText( Code ) </text--->"
    "<text>Creates a 1-character text with the symbol according to "
    "ASCII-code Code."
    " </text--->"
    "<text>query 'Hello' + charToText( 10 ) + 'world!'</text--->"
    ") )";

const string attr2textSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> DATA -> text </text--->"
    "<text> attr2text(_) </text--->"
    "<text>Creates a string representation of an attribute "
    " type using its Print function."
    " </text--->"
    "<text>query attr2text(32) </text--->"
    ") )";

const string isValidIDSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> string [ x bool] -> bool </text--->"
    "<text> isValidID(ID, checkForObject) </text--->"
    "<text>Checks whether the given string can be used as a name, "
    " i.e. ID is not a keyword, an operator, or a type. If the boolean"
    " parameter is present and TRUE, the ID must not be object to evaluate"
    " to TRUE "
    " </text--->"
    "<text>query isValidID(\"query\") </text--->"
    ") )";


const string trimAllSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> stream(string> -> stream(string) , "
    "stream(text) -> stream(text), stream(tuple(X)) -> stream(tuple(X))"
    " </text--->"
    "<text>  _ trimAll </text--->"
    "<text>Trims all occurences of text and string within the stream,"
    " </text--->"
    "<text>query trains trimAll count </text--->"
    ") )";

const string trimAllUndefSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> stream(string> -> stream(string) , "
    "stream(text) -> stream(text), stream(tuple(X)) -> stream(tuple(X))"
    " </text--->"
    "<text>  _ trimAll </text--->"
    "<text>Trims all occurences of text and string within the stream,"
    "empty strings are maked undefined"
    " </text--->"
    "<text>query trains trimAll count </text--->"
    ") )";
/*
Operator Definitions

*/
Operator contains
(
  "contains",           //name
  containsSpec,         //specification
  4,                    //no. of value mappings
  FText_VMMap_Contains, //value mapping
  SelectFun_TextString_TextString,   //selection function
  TypeMap_TextString_TextString__Bool   //type mapping
);

Operator length
(
  "length",             //name
  lengthSpec,           //specification
  ValMapTextInt,        //value mapping
  Operator::SimpleSelect,         //trivial selection function
  TypeMap_Text__Int        //type mapping
);

Operator getkeywords
(
  "keywords",            //name
  keywordsSpec,          //specification
  ValMapkeywords,        //value mapping
  Operator::SimpleSelect,          //trivial selection function
  TypeMap_text__stringstream        //type mapping
);

Operator getsentences
(
  "sentences",            //name
  sentencesSpec,          //specification
  ValMapsentences,        //value mapping
  Operator::SimpleSelect,           //trivial selection function
  TypeMap_text__textstream        //type mapping
);

Operator diceCoeff(
   "dice",
   diceSpec,
   ValMapDice_t_t,
   Operator::SimpleSelect,
   TypeMap_int_text_text__real
);

Operator ftextgetcatalog
(
    "getcatalog",
    getCatalogSpec,
    ValMapGetCatalog,
    Operator::SimpleSelect,
    TypeGetCatalog
);

Operator ftextsubstr
(
    "substr",
    substrSpec,
    ValMapSubstr,
    Operator::SimpleSelect,
    TypeFTextSubstr
);

Operator ftextsubtext
(
    "subtext",
    subtextSpec,
    ValMapSubtext,
    Operator::SimpleSelect,
    TypeFTextSubtext
);

Operator ftextfind
    (
    "find",
    FTextfindSpec,
    4,
    FText_VMMap_Find,
    SelectFun_TextString_TextString,
    TypeMap_textstring_textstring__intstream
    );

Operator ftextisempty
    (
    "isempty",
    FTextisemptySpec,
    FTextValMapIsEmpty,
    Operator::SimpleSelect,
    TypeMap_text__bool
    );

Operator ftexttrim
    (
    "trim",
    FTexttrimSpec,
    2,
    trimVM,
    trimSelect,
    trimTM
    );

Operator ftextplus
    (
    "+",
    FTextplusSpec,
    3,
    FText_VMMap_Plus,
    FTextSelectFunPlus,
    FTextTypeMapPlus
    );

/*
Comparison predicates

*/
Operator ftextless
    (
    "<",
    FTextLessSpec,
    3,
    FText_VMMap_Less,
    FTextSelectFunComparePred,
    FTextTypeMapComparePred
    );

Operator ftextlesseq
    (
    "<=",
    FTextLessEqSpec,
    3,
    FText_VMMap_LessEq,
    FTextSelectFunComparePred,
    FTextTypeMapComparePred
    );

Operator ftexteq
    (
    "=",
    FTextEqSpec,
    3,
    FText_VMMap_Eq,
    FTextSelectFunComparePred,
    FTextTypeMapComparePred
    );

Operator ftextbiggereq
    (
    ">=",
    FTextBiggerEqSpec,
    3,
    FText_VMMap_BiggerEq,
    FTextSelectFunComparePred,
    FTextTypeMapComparePred
    );

Operator ftextbigger
    (
    ">",
    FTextBiggerSpec,
    3,
    FText_VMMap_Bigger,
    FTextSelectFunComparePred,
    FTextTypeMapComparePred
    );

Operator ftextneq
    (
    "#",
    FTextNeqSpec,
    3,
    FText_VMMap_Neq,
    FTextSelectFunComparePred,
    FTextTypeMapComparePred
    );

/*
Operators for subqueries

*/
Operator ftextevaluate
    (
    "evaluate",
    FTextEvaluateSpec,
    FTextValueMapEvaluate,
    Operator::SimpleSelect,
    FTextTypeMapEvaluate
    );

Operator ftextreplace
    (
    "replace",
    FTextReplaceSpec,
    12,
    FText_VMMap_Replace,
    FTextSelectFunReplace,
    FTextTypeReplace
    );

Operator isDBObject
    (
    "isDBObject",
    FTextIsDBObjectSpec,
    FTextValueMapIsDBObject,
    Operator::SimpleSelect,
    TypeMap_string__bool
    );

Operator getTypeNL
    (
    "getTypeNL",
    FTextGetTypeNLSpec,
    FTextValueMapGetTypeNL,
    Operator::SimpleSelect,
    FTextTypeMapGetTypeNL
    );

Operator getValueNL
    (
    "getValueNL",
    FTextGetValueNLSpec,
    3,
    FText_VMMap_GetValueNL,
    FTextSelectFunGetValueNL,
    FTextTypeMapGetValueNL
    );

Operator ftexttoobject
    (
    "toObject",
    FTextToObjectSpec,
    2,
    FText_VMMap_ToObject,
    FTextSelectFunToObject,
    toObjectTM 
    );

/*
Type conversion operations

*/
Operator chartext
    (
    "chartext",
    FTextChartextSpec,
    FTextValueMapChartext,
    Operator::SimpleSelect,
    TypeMap_int__text
    );

Operator ftexttoupper
    (
    "toupper",
    FTextToUpperSpec,
    2,
    toUpperVM,
    changeCaseSelect, 
    changeCaseTM
 );


Operator ftexttolower
    (
    "tolower",
    FTextToLowerSpec,
    2,
    toLowerVM,
    changeCaseSelect, 
    changeCaseTM
 );


Operator ftexttostring
    (
    "tostring",
    FTextTextToStringSpec,
    FTextValueMapConvert<true, FText, CcString>,
    Operator::SimpleSelect,
    TypeMap_text__string
    );

Operator ftexttotext
    (
    "totext",
    FTextStringToTextSpec,
    FTextValueMapConvert<false, CcString, FText>,
    Operator::SimpleSelect,
    TypeMap_string__text
    );

/*
Operations for sending/receiving texts vie UPD/IP

*/
Operator ftsendtextUDP
    (
    "sendtextUDP",
    FTextSendTextUDPSpec,
    32,
    FText_VMMap_MapSendTextUDP,
    FTextSelectSendTextUDP,
    FTextTypeSendTextUDP
    );

Operator ftreceivetextUDP
    (
    "receivetextUDP",
    FTextReceiveTextUDPSpec,
    4,
    FText_VMMap_MapReceiveTextUDP,
    FTextSelectReceiveTextUDP,
    FTextTypeReceiveTextUDP
    );

Operator ftreceivetextstreamUDP
    (
    "receivetextstreamUDP",
    FTextReceiveTextStreamUDPSpec,
    4,
    FText_VMMap_MapReceiveTextStreamUDP,
    FTextSelectReceiveTextUDP,
    FTextTypeReceiveTextStreamUDP
    );

/*
Coverting between text and svg

*/
Operator svg2text
(
  "svg2text",           //name
  svg2textSpec,   //specification
  SVG2TEXTVM, //value mapping
  Operator::SimpleSelect,         //trivial selection function
  TypeMap_svg__text //type mapping
);

Operator text2svg
(
  "text2svg",           //name
  text2svgSpec,   //specification
  SVG2TEXTVM, //value mapping
  Operator::SimpleSelect,         //trivial selection function
  TypeMap_text__svg //type mapping
);

/*
En-/decrypting texts

*/
Operator crypt
    (
    "crypt",
    cryptSpec,
    6,
    cryptvm,
    cryptSelect,
    cryptTM
    );

Operator checkpw
    (
    "checkpw",
    checkpwSpec,
    4,
    checkpwvm,
    checkpwSelect,
    checkpwTM
    );

Operator md5
    (
    "md5",
    md5Spec,
    6,
    md5vm,
    md5Select,
    md5TM
    );

Operator blowfish_encode ( "blowfish_encode", blowfish_encodeSpec,
                           4, blowfish_encodevm, blowfish_encodeSelect,
                           blowfish_encodeTM
    );
Operator blowfish_decode ( "blowfish_decode", blowfish_decodeSpec,
                           4, blowfish_decodevm, blowfish_decodeSelect,
                           blowfish_decodeTM
    );

/*
More subquery support

*/
Operator ftextletObject ( "letObject", ftextletObjectSpec,
                           4, ftextletobject_vm, blowfish_encodeSelect,
                           StringtypeStringtypeBool2TextTM
    );

Operator ftextdeleteObject ( "deleteObject", ftextdeleteObjectSpec,
                           2, ftextdeleteobject_vm, ftextdeleteobjectselect,
                           TypeMap_textstring__text
    );

Operator ftextcreateObject ( "createObject", ftextcreateObjectSpec,
                           4, ftextcreateobject_vm, blowfish_encodeSelect,
                           StringtypeStringtypeBool2TextTM
    );

// Operator ftextupdateObject ( "updateObject", ftextupdateObjectSpec,
//                            4, ftextupdateobject_vm, blowfish_encodeSelect,
//                            StringtypeStringtypeBool2TextTM
//     );
//
// Operator ftextderiveObject ( "deriveObject", ftextderiveObjectSpec,
//                            4, ftextderiveobject_vm, blowfish_encodeSelect,
//                            StringtypeStringtypeBool2TextTM
//     );

Operator ftextgetObjectTypeNL ( "getObjectTypeNL", ftextgetObjectTypeNLSpec,
                           2, ftextgetObjectTypeNL_vm, ftextdeleteobjectselect,
                           TypeMap_textstring__text
    );

Operator ftextgetObjectValueNL ("getObjectValueNL",ftextgetObjectValueNLSpec,
                           2,ftextgetObjectValueNL_vm, ftextdeleteobjectselect,
                           TypeMap_textstring__text
    );

Operator getDatabaseName
(
   "getDatabaseName",          //name
   getDatabaseNameSpec,        //specification
   getDatabaseName_VM,         //value mapping
   Operator::SimpleSelect,     //trivial selection function
   TypeMap_empty__string              //type mapping
);

/*
Checking type mappings, inquiries on type constructors and operators

*/
Operator matchingOperatorNames
(
   "matchingOperatorNames",          //name
   matchingOperatorNamesSpec,        //specification
   matchingOperatorsVM<true>,         //value mapping
   Operator::SimpleSelect,     //trivial selection function
   matchingOperatorNamesTM              //type mapping
);


Operator matchingOperators
(
   "matchingOperators",          //name
   matchingOperatorsSpec,        //specification
   matchingOperatorsVM<false>,         //value mapping
   Operator::SimpleSelect,     //trivial selection function
   matchingOperatorsTM              //type mapping
);

Operator sysgetMatchingOperators
(
  "sys_getMatchingOperators",              //name
  sysgetMatchingOperatorsSpec,             //specification
  3,                                       // no of VM-functions
  sysgetMatchingOperators_vm,              //value mapping
  sysgetMatchingOperators_select,          //trivial selection function
  sysgetMatchingOperatorsTM                //type mapping
);


Operator sysgetAlgebraName
(
"sys_getAlgebraName",           //name
 sys_getAlgebraNameSpec,       //specification
 sys_getAlgebraNameVM,        //value mapping
 Operator::SimpleSelect,     //trivial selection function
 int2stringTM               //type mapping
);

Operator sysgetAlgebraId
(
"sys_getAlgebraId",             //name
 sys_getAlgebraIdSpec,         //specification
 2,                           // no of VM functions
 sys_getAlgebraId_vm,        //value mapping
 sys_getAlgebraId_select,   //trivial selection function
 stringORtext2intTM        //type mapping
);

// Operator sysgetOperatorInfo
// (
//  "sys_getOperatorInfo",          //name
//   sys_getOperatorInfoSpec,      //specification
//   sys_getOperatorInfoVM,       //value mapping
//   Operator::SimpleSelect,     //trivial selection function
//   sys_getOperatorInfoTM      //type mapping
// );
//

Operator checkOperatorTypeMap
(
   "checkOperatorTypeMap",          //name
   CheckOperatorTypeMapSpec,        //specification
   CheckOperatorTypeMapVM<FText>,         //value mapping
   Operator::SimpleSelect,     //trivial selection function
   CheckOperatorTypeMapTM              //type mapping
);

Operator checkOperatorTypeMap2 (
   "checkOperatorTypeMap2",          //name
   CheckOperatorTypeMap2Spec,        //specification
   2,
   CheckOperatorTypeMap_vm,
   CheckOperatorTypeMapSelect,             //type mapping
   CheckOperatorTypeMap2TM              //type mapping
    );


Operator strequal (
   "strequal",          //name
   strequalSpec,        //specification
   4,
   strequal_vm,
   strequal_select,             //type mapping
   strequalTM              //type mapping
    );

Operator tokenize
(
  "tokenize",             //name
  tokenizeSpec,           //specification
  tokenizeVM,        //value mapping
  Operator::SimpleSelect,         //trivial selection function
  tokenizeTM        //type mapping
);

// Operator sysgetTypeConstructorInfo
// (
//   "sys_getTypeConstructorInfo",          //name
//   sys_getTypeConstructorInfoSpec,        //specification
//   sys_getTypeConstructorInfoVM,         //value mapping
//   Operator::SimpleSelect,     //trivial selection function
//   sys_getTypeConstructorInfoTM              //type mapping
// );
//
//
// Operator  sysgetNoAlgebras (
//   "sys_getNoAlgebras",          //name
//   SysGetNoAlgebrasSpec,        //specification
//   SysGetNoAlgebrasVM,         //value mapping
//   Operator::SimpleSelect,    //trivial selection function
//   empty2intTM               //type mapping
// )
//

Operator sendtextstreamTCP
(
 "sendtextstreamTCP",
 sendtextstreamTCP_Spec,
 32,
 sendtextstream_vm,
 sendtextstream_Select,
 sendtextstreamTCP_TM
);


Operator charToText
(
"charToText",
 charToText_Spec,
 charToTextFun,
 Operator::SimpleSelect,
 Int2Text_TM
 );


Operator attr2text(
 "attr2text",
 attr2textSpec,
 attr2textVM,
 Operator::SimpleSelect,
 attr2textTM
 );

Operator isValidID(
   "isValidID",
   isValidIDSpec,
   isValidIDVM,
   Operator::SimpleSelect,
   isValidIDTM
  );

Operator trimAll
(
 "trimAll",
 trimAllSpec,
 3,
 trimAllVM,
 trimAllSelect,
 trimAllTM
);

Operator trimAllUndef
(
 "trimAllUndef",
 trimAllUndefSpec,
 3,
 trimAllUndefVM,
 trimAllSelect,
 trimAllTM
);

/*
~Operator~ cn2en  by Cheng Wang 18.09.2014
Convert Chinese characters into English letters

*/

/*
pinyin\_utf8: convert utf8 encoding chinese charaters into english letters
@inbuf The input chinese charaters

*/
string pinyin_utf8(string inbuf) {
  int inbuf_len = inbuf.length();
  int uni=0;
  string resStr;
  for (int i = 0; i < inbuf_len; i++) {
    if ((unsigned char) inbuf[i] < 0x80) { //English letters or numbers
      resStr += inbuf[i];
      continue;
    } else if ((inbuf[i] & 0xE0) == 0xC0) { //others
      resStr += inbuf[i];
      continue;
    } else if ((inbuf[i] & 0xF0) == 0xE0) { //Chinese
      if (i + 2 >= inbuf_len) {
        break;
      }
      uni = (((int) (inbuf[i] & 0x0F)) << 12)
        | (((int) (inbuf[i + 1] & 0x3F)) << 6)
        | (inbuf[i + 2] & 0x3F);
      if (uni > 19967 && uni < 40870) {
        string ch(pytable[uni - 19968]);
        int n = ch.find('|');
        if (n != -1)
          ch = ch.substr(0, n);
        for(size_t i =0; i < ch.length(); i++)
          if ( ch[i] >= 'a' && ch[i] <= 'z')
            ch[i] = ch[i] - 32 ;
        resStr += ch;
        resStr += " ";
      }
    }
  }
  return resStr;
}

/*
Type Mapping
  
*/
ListExpr cn2enTM(ListExpr args){
   string err =" string or text expected";
   if(!nl->HasLength(args,1)){
      return listutils::typeError(err);
   }
   ListExpr arg = nl->First(args);
   if(!FText::checkType(arg) &&
      !CcString::checkType(arg)){
      return listutils::typeError(err);
   }
   return nl->SymbolAtom(FText::BasicType());
}

/*
Value Mapping

*/
template<class T>
int cn2enVM1( Word* args, Word& result, int message,
                      Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  T* arg = (T*) args[0].addr;
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string str = arg->GetValue();
  stringutils::trim(str);
  if(str.length()==0){
     res->SetDefined(false);
     return 0;
  }
  string resStr = pinyin_utf8(str);
  
  if(resStr.length() == 0 ){
    res->SetDefined(false);
  } else {
    res->Set(true,resStr);
  }
  return 0;
}


/*
Selection function and Value Mapping array

*/

int cn2enSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
     return 0;
  }
  return 1; // text
}

ValueMapping cn2enVM[] = {
                cn2enVM1<CcString>,
                cn2enVM1<FText>
            };


/*
Specification

*/
const string cn2enSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text,string} -> text </text--->"
    "<text> cn2en(_) </text--->"
    "<text>Converts Chinese characters into English letters. "
    " </text--->"
    "<text>query cn2en('中国')  </text--->"
    ") )";

/*
Operator instance

*/


Operator cn2en
(
"cn2en",             //name
 cn2enSpec,         //specification
 2,                           // no of VM functions
 cn2enVM,        //value mapping
 cn2enSelect,   //trivial selection function
 cn2enTM        //type mapping
);





/*
~Operator~ str2real

Type Mapping

*/
ListExpr str2realTM(ListExpr args){
   string err =" string or text expected";
   if(!nl->HasLength(args,1)){
      return listutils::typeError(err);
   }
   ListExpr arg = nl->First(args);
   if(!FText::checkType(arg) &&
      !CcString::checkType(arg)){
      return listutils::typeError(err);
   }
   return nl->SymbolAtom(CcReal::BasicType());
}

/*
Value Mapping

*/
template<class T>
int str2realVM1( Word* args, Word& result, int message,
                      Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcReal* res = (CcReal*) result.addr;
  T* arg = (T*) args[0].addr;
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string str = arg->GetValue();
  stringutils::trim(str);
  if(str.length()==0){
     res->SetDefined(false);
     return 0;
  }
  stringstream ss;
  ss.str(str);
  double r;
  ss >> r;
  if(r!=r || !ss.eof()){
    res->SetDefined(false);
  } else {
    res->Set(true,r);
  }
  return 0;
}


/*
Selection function and Value Mapping array

*/

int str2realSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
     return 0;
  }
  return 1; // text
}

ValueMapping str2realVM[] = {
                str2realVM1<CcString>,
                str2realVM1<FText>
            };


/*
Specification

*/
const string str2realSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text | string -> real </text--->"
    "<text> str2real(_) </text--->"
    "<text>Converts a string or text to a real value. "
    " </text--->"
    "<text>query str2real('3.14')  </text--->"
    ") )";

/*
Operator instance

*/


Operator str2real
(
"str2real",             //name
 str2realSpec,         //specification
 2,                           // no of VM functions
 str2realVM,        //value mapping
 str2realSelect,   //trivial selection function
 str2realTM        //type mapping
);



/*
~Operator~ str2int

Type Mapping

*/
ListExpr str2intTM(ListExpr args){
   string err =" string or text expected";
   if(!nl->HasLength(args,1)){
      return listutils::typeError(err);
   }
   ListExpr arg = nl->First(args);
   if(!FText::checkType(arg) &&
      !CcString::checkType(arg)){
      return listutils::typeError(err);
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

/*
Value Mapping

*/
template<class T>
int str2intVM1( Word* args, Word& result, int message,
                      Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  T* arg = (T*) args[0].addr;
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string str = arg->GetValue();
  stringutils::trim(str);
  if(str.length()==0){
     res->SetDefined(false);
     return 0;
  }
  stringstream ss;
  ss.str(str);
  int r;
  ss >> r;
  if(!ss.eof()){
    res->SetDefined(false);
  } else {
    res->Set(true,r);
  }
  return 0;
}


/*
Selection function and Value Mapping array

*/

int str2intSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
     return 0;
  }
  return 1; // text
}

ValueMapping str2intVM[] = {
                str2intVM1<CcString>,
                str2intVM1<FText>
            };


/*
Specification

*/
const string str2intSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text | string -> int </text--->"
    "<text> str2int(_) </text--->"
    "<text>Converts a string or text to a int value. "
    " </text--->"
    "<text>query str2int('666')  </text--->"
    ") )";

/*
Operator instance

*/


Operator str2int
(
"str2int",             //name
 str2intSpec,         //specification
 2,                           // no of VM functions
 str2intVM,        //value mapping
 str2intSelect,   //trivial selection function
 str2intTM        //type mapping
);

#ifdef RECODE

/*
4.12 Operator ~recode~


Type Mapping

Signature: text x string x string -> text
         : string x string x string -> string
Meaning: recodes a text from one charset into another one

*/
ListExpr recodeTM(ListExpr args){
   string err = " t x string x string ->  t, t in {string, text} expected";
   if(!nl->HasLength(args,3)){
     return listutils::typeError(err);
   }
   ListExpr f = nl->First(args);
   if(!CcString::checkType(f) && !FText::checkType(f)){
     return listutils::typeError(err);
   }
   if(!CcString::checkType(nl->Second(args)) ||
      !CcString::checkType(nl->Third(args))) {
     return listutils::typeError(err);
   }
   return f;
}

/*
 Value Mapping

*/
  template<class T>
  int recodeVM1( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
     result = qp->ResultStorage(s);
     T* res = (T*) result.addr;
     T* text1 = (T*) args[0].addr;
     CcString* from1 =  (CcString*) args[1].addr;
     CcString* to1 = (CcString*) args[2].addr;
     if(!text1->IsDefined() || !from1->IsDefined() || !to1->IsDefined()){
       res->SetDefined(false);
       return 0;
     }
     string text = text1->GetValue();
     string from = from1->GetValue();
     string to = to1->GetValue();

     string rs = trim(from)+".."+trim(to);
     
     // use recode lib

     RECODE_OUTER outer = recode_new_outer(true);
     RECODE_REQUEST request = recode_new_request(outer);

     bool success = recode_scan_request (request, rs.c_str());
     if(!success){
         recode_delete_request(request);
         recode_delete_outer(outer);
         res->SetDefined(false);
         return 0;
     }

     char* recoded = recode_string(request, text.c_str());

      // make clean
     recode_delete_request(request);
     recode_delete_outer(outer);
     if(recoded==0){
        res->SetDefined(false);
        return 0;
     }
     res->Set(true, recoded);
     free(recoded);
     return 0;
  }

  
/*
Value Mapping Array and Selection function

*/

ValueMapping recodeVM [] = {recodeVM1<CcString>,
                            recodeVM1<FText>};

int recodeSelect(ListExpr args){
   ListExpr f = nl->First(args);
   if(CcString::checkType(f)) return 0;
   if(FText::checkType(f)) return 1;
   return -1;
}     

/*
Specification


*/

  const string recodeSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
      "( <text>  t x string x string  -> t, t uin {text,string} </text--->"
      "<text> _ recode [_ , _]  </text--->"
      "<text>Recodes a string/text from one charset to another one "
      " </text--->"
      "<text>query 'Teststring' recode [\"utf8\", \"latin1\"] </text--->"
      ") )";

  /*
  Operator instance

  */


  Operator recode
  (
  "recode",             //name
   recodeSpec,         //specification
   2,                           // no of VM functions
   recodeVM,        //value mapping
   recodeSelect,   //trivial selection function
   recodeTM        //type mapping
  );

#endif

/*
4.13 Operator ~endsWith~

4.13.1 Type Mapping 

Signature is {text, string} x {text, string} -> bool

*/

ListExpr starts_ends_WithTM(ListExpr args){
  string err = " {text, string} x {text, string} expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(!CcString::checkType(first) &&
     !FText::checkType(first)){
     return listutils::typeError(err);
  }
  if(!CcString::checkType(second) &&
     !FText::checkType(second)){
     return listutils::typeError(err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}


/*
4.13.2 Value Mapping of ~endsWith~ and ~startsWith~

*/

  template<class A1, class A2, bool end>
  int starts_ends_WithVM1( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

     A1* a1 = (A1*) args[0].addr;
     A2* a2 = (A2*) args[1].addr;
     result = qp->ResultStorage(s);
     CcBool* res = (CcBool*) result.addr;
     if(!a1->IsDefined() || !a2->IsDefined()){
        res->SetDefined(false);
     } else {
        if(end){
          res->Set(true, stringutils::endsWith(a1->GetValue(), 
                                               a2->GetValue()));
        } else {
          res->Set(true, stringutils::startsWith(a1->GetValue(), 
                                               a2->GetValue()));
        }
     }
     return 0; 
  }





/*
4.13.3 Value Mapping Array and Selection function

*/   
   ValueMapping endsWithVM[] = {
                starts_ends_WithVM1<CcString, CcString,true>,
                starts_ends_WithVM1<CcString, FText,true>,
                starts_ends_WithVM1<FText, CcString,true>,
                starts_ends_WithVM1<FText, FText,true>
            };
   
   ValueMapping startsWithVM[] = {
                starts_ends_WithVM1<CcString, CcString,false>,
                starts_ends_WithVM1<CcString, FText,false>,
                starts_ends_WithVM1<FText, CcString,false>,
                starts_ends_WithVM1<FText, FText,false>
            };

   int starts_ends_WithSelect(ListExpr args){
      ListExpr first = nl->First(args);
      ListExpr second = nl->Second(args);
      if(CcString::checkType(first)){
         if(CcString::checkType(second)){
           return 0;
         } else {
           return 1;
         }
      } else {
         if(CcString::checkType(second)){
           return 2;
         } else {
           return 3;
         }
      }
   }

/*
4.13.4 Specification

*/
  const string endsWithSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
      "( <text>  {text,string} x {text,string} -> bool </text--->"
      "<text> _ endsWith _  </text--->"
      "<text>Checks whether the first argument's value ends with the  "
      " second's argument value</text--->"
      "<text>query \"Hello\" endsWith 'llo'  </text--->"
      ") )";

  const string startsWithSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
      "( <text>  {text,string} x {text,string} -> bool </text--->"
      "<text> _ startsWith _  </text--->"
      "<text>Checks whether the first argument's value starts with the  "
      " second's argument value</text--->"
      "<text>query \"Hello\" startsWith 'Hell'  </text--->"
      ") )";
  
  const string startsSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
      "( <text>  {text,string} x {text,string} -> bool </text--->"
      "<text> _ starts _  </text--->"
      "<text>Checks whether the first argument's value starts with the  "
      " second's argument value</text--->"
      "<text>query \"Hello\" startsWith 'Hell'  </text--->"
      ") )";

  Operator endsWith
  (
  "endsWith",             //name
   endsWithSpec,         //specification
   4,                           // no of VM functions
   endsWithVM,        //value mapping
   starts_ends_WithSelect,   
   starts_ends_WithTM        //type mapping
  );

  Operator startsWith
  (
  "startsWith",             //name
   startsWithSpec,         //specification
   4,                           // no of VM functions
   startsWithVM,        //value mapping
   starts_ends_WithSelect,   
   starts_ends_WithTM        //type mapping
  );

  Operator startsOp
  (
  "starts",             //name
   startsSpec,         //specification
   4,                           // no of VM functions
   startsWithVM,        //value mapping
   starts_ends_WithSelect,   
   starts_ends_WithTM        //type mapping
  );

/*
4.14 Operator markText


This operator sets markers at specified positions within a text.

4.14.1 Type mapping

Signature stream(tuple()) x text x ai x aj x string x string -> text
with ai,aj : int

*/

ListExpr markTextTM(ListExpr args) {
    string err = "stream(tuple) x aname x aname x string x string expected";
    if(!nl->HasLength(args,6)){
      return listutils::typeError(err);
    }
    ListExpr stream = nl->First(args);
    ListExpr aName1 = nl->Second(args);
    ListExpr aName2 = nl->Third(args);
    ListExpr text = nl->Fourth(args);
    ListExpr smark = nl->Fifth(args);
    ListExpr emark = nl->Sixth(args);
    if(!Stream<Tuple>::checkType(stream) ||
       !listutils::isSymbol(aName1) ||
       !listutils::isSymbol(aName2) ||
       !FText::checkType(text) ||
       !CcString::checkType(smark) ||
       !CcString::checkType(emark)){
     return listutils::typeError(err);
   }
   ListExpr attrList = nl->Second(nl->Second(stream));
   ListExpr type;
   string name1 = nl->SymbolValue(aName1);
   int index1 = listutils::findAttribute( attrList, name1,type);
   if(index1==0){
     return listutils::typeError("Attribute " + name1 + " unknown");
   }
   if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + name1 + " not od type int");
   }
   string name2 = nl->SymbolValue(aName2);
   int index2 = listutils::findAttribute( attrList, name2,type);
   if(index2==0){
     return listutils::typeError("Attribute " + name2 + " unknown");
   }
   if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + name2 + " not od type int");
   }
   return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                             nl->TwoElemList(
                                  nl->IntAtom(index1-1),
                                  nl->IntAtom(index2-1)),
                            listutils::basicSymbol<FText>());
}


/*
4.14.2 Value Mapping

*/

int markTextVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

     result = qp->ResultStorage(s);
     FText* res = (FText*) result.addr;
     int attrPos1 = ((CcInt*) args[6].addr)->GetValue();
     int attrPos2 = ((CcInt*) args[7].addr)->GetValue();
     FText* theText = (FText*) args[3].addr;
     CcString* sMark = (CcString*) args[4].addr;
     CcString* eMark = (CcString*) args[5].addr;

     if(!theText->IsDefined() || ! sMark->IsDefined() || !eMark->IsDefined()){
         res->SetDefined(false);
         return 0;
     }
     Stream<Tuple> stream(args[0]);
     stream.open();
     Tuple* tuple;
     string marks = sMark->GetValue();
     string marke = eMark->GetValue();
     priority_queue<int> smarks;
     priority_queue<int> emarks;

     string text = theText->GetValue();
     int length = text.length();

     

     while( (tuple=stream.request())!=0){
        CcInt* s = (CcInt*) tuple->GetAttribute(attrPos1);
        CcInt* e = (CcInt*) tuple->GetAttribute(attrPos2);
        if(s->IsDefined() && e->IsDefined()){
           int si = s->GetValue();
           int ei = e->GetValue();
           if((si>=0) && (ei>=0) && (si<=ei) && (ei <= length)){
              smarks.push(si);
              emarks.push(ei);
           }
        }
        tuple->DeleteIfAllowed();
     }
     stream.close(); 


     while(!smarks.empty() && !emarks.empty()){
         int stop = smarks.top();
         int etop = emarks.top();
         int top;
         string in;

         if(stop>=etop){
           top = stop;
           in = marks;
           smarks.pop();
         } else {
           top = etop;
           in = marke;
           emarks.pop(); 
         }
         text = text.insert(top,in);
     }
     while(!smarks.empty()){
         int top = smarks.top();
         smarks.pop();
         text = text.insert(top,marks); 
     }
     while(!emarks.empty()){
         int top = emarks.top();
         emarks.pop();
         text = text.insert(top,marke); 
     }



     res->Set(true,text);
     return 0;
 }

  const string markTextSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
      "( <text>  stream(tuple) x ai x aj x text x string x string"
      " with ai, aj : int "
      "</text--->"
      "<text> _  marktext[_,_,_,_,_]  </text--->"
      "<text>Inserts markers (string arguments) at a text at all"
      " positions decsribed in the stream."
      "<text>query 5 feed namedtransformstream[S] extend[E : s + 4] "
      "markText['Hello World',A,E,\"<b>\",\"<\\b>\" </text--->"
      ") )";

  Operator markText 
  (
  "markText",             //name
   markTextSpec,         //specification
   markTextVM,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
   markTextTM        //type mapping
  );

/*
4.15 Operator bashModifier

This Operator takes a description of an bash modifier and returns a string
representing this modifier. For example 
the query 
   query "Normal" +  bashModifier("RED") Red + bashModifier("Normal") + Normal.

will write the word "red" with foreground color red and the remainder in the
standard colors used by the shell. Note that this operator only works 
in bash compatible shells.

*/
ListExpr bashModifierTM(ListExpr args){
  string err = "string expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcString>();
}


enum bashModifier  { NORMAL, BOLD, UNDERLINE, BLACK, RED, GREEN, 
                     YELLOW, BLUE, VIOLET, CYAN, BG_BLACK, BG_RED, 
                     BG_GREEN, BG_YELLOW, BG_VIOLET,BG_CYAN, UNKNOWN};

string getModifierName(bashModifier m){
   switch(m){
      case  NORMAL : return "NORMAL"  ;
      case  BOLD : return  "BOLD" ;
      case  UNDERLINE : return  "UNDERLINE" ;
      case  BLACK : return "BLACK"  ;
      case  RED : return  "RED" ;
      case  GREEN : return  "GREEN" ;
      case  YELLOW : return  "YELLOW"  ;
      case  BLUE : return   "BLUE" ;
      case  VIOLET : return  "VIOLET" ;
      case  CYAN : return  "CYAN" ;
      case  BG_BLACK : return  "BG_BLACK" ;
      case  BG_RED : return   "BG_RED" ;
      case  BG_GREEN : return  "BG_GREEN" ;
      case  BG_YELLOW : return  "BG_YELLOW" ;
      case  BG_VIOLET : return  "BG_VIOLET" ;
      case  BG_CYAN : return   "BG_CYAN";
      default:   return  "UNKNOWN" ;
   }
};

string getModifier(bashModifier m){
  switch(m){
    case NORMAL     : return "\033[0m";
    case BOLD       : return "\033[1m";
    case UNDERLINE  : return "\033[4m";
    case BLACK      : return "\033[30m";
    case RED        : return "\033[31m";
    case GREEN      : return "\033[32m";
    case YELLOW     : return "\033[33m";
    case BLUE       : return "\033[34m";
    case VIOLET     : return "\033[35m";
    case CYAN       : return "\033[36m";
    case BG_BLACK   : return "\033[40m";
    case BG_RED     : return "\033[41m";
    case BG_GREEN   : return "\033[42m";
    case BG_YELLOW  : return "\033[43m";
    case BG_VIOLET  : return "\033[44m";
    case BG_CYAN    : return "\033[45m";
    default: return "";
  }
};

bashModifier str2mod(string s){
  stringutils::toLower(s);
  if(s=="normal") return NORMAL;
  if(s=="bold") return  BOLD ;
  if(s=="underline") return UNDERLINE ;
  if(s=="black") return BLACK ;
  if(s=="red") return RED ;
  if(s=="green") return GREEN ;
  if(s=="yellow") return YELLOW ;
  if(s=="blue") return BLUE ;
  if(s=="violet") return VIOLET ;
  if(s=="cyan") return CYAN ;
  if(s=="bg_black") return BG_BLACK ;
  if(s=="bg_red") return BG_RED ;
  if(s=="bg_green") return BG_GREEN ;
  if(s=="bg_yellow") return BG_YELLOW ;
  if(s=="bg_violet") return BG_VIOLET ;
  if(s=="bg_cyan") return BG_CYAN ;
  return UNKNOWN;
}

string allBashModifiers(){
  stringstream ss;
  for(int i=NORMAL; i< UNKNOWN; i++){
    ss << getModifierName(static_cast<bashModifier>(i)) << " ";
  }
  ss << getModifierName(UNKNOWN);
  return ss.str();
}



int bashModifierVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){

  CcString* a = (CcString*) args[0].addr;
  result = qp->ResultStorage(s);
  CcString* res = (CcString*) result.addr;
  if(!a->IsDefined()){
    res->SetDefined(false);
  } else {
    string s = getModifier(str2mod(a->GetValue()));
    res->Set(true,s);
  }
  return 0;
}

OperatorSpec bashModifierSpec(
           "string -> string",
           "bashModifier(_)",
           "Returns a string modifying bash settings "
           "(see also getBashModifiers).",
           " query 'normal' + bashModifier(\"RED\") + ' red ' + "
           "bashModifier(\"NORMAL\") + 'normal'" );

Operator bashModifierOp
  (
  "bashModifier",             //name
   bashModifierSpec.getStr(),         //specification
   bashModifierVM,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
   bashModifierTM        //type mapping
  );

/*
4.17 getBashModifiers

4.17.1 Type Mapping

Signature is -> string

*/
ListExpr getBashModifiersTM(ListExpr args){
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no argument expected");
  } 
  return listutils::basicSymbol<FText>();
}


int getBashModifiersVM( Word* args, Word& result, int message,
                    Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   res->Set(true,allBashModifiers());
   return 0;
}


OperatorSpec getBashModifiersSpec(
           "-> string",
           "igetBashModifiers()",
           "Returns all meaningful arguments to the bashModifier Operator ",
           " query getBashModifiers()a ");


Operator getBashModifiersOp
  (
  "getBashModifiers",             //name
   getBashModifiersSpec.getStr(),         //specification
   getBashModifiersVM,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
   getBashModifiersTM        //type mapping
  );



/*
5.1 Operator pointertest

  adds two integer values using a pointer within a subquery

5.1.1 Type Mapping


*/
ListExpr pointerTestTM(ListExpr args){
   string err = "int x int expected";
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }
   if(!CcInt::checkType(nl->First(args)) ||
      !CcInt::checkType(nl->Second(args))){
      return listutils::typeError(err);
   }
   return listutils::basicSymbol<CcInt>();
}

/*
5.1.2 Value Mapping

*/

int pointerTestVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   CcInt* i1 = (CcInt*)args[0].addr;
   CcInt* i2 = (CcInt*) args[1].addr;
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;

   string query = "[const int  pointer " + 
                  nl->ToString(listutils::getPtrList(i1) ) + 
                  " ] + [const int pointer "
                   + nl->ToString(listutils::getPtrList(i2)) + " ] ";

   // cout << "Query = " << query << endl;

   SecParser mySecParser;

   string queryAsListStr; 
   if (mySecParser.Text2List("query " +  query,
                              queryAsListStr)!= 0) {
    //  cerr << "Cannot parse query" << endl;
      res->SetDefined(false);
      return 0;
   }


   // step 1 tnaslate to nested list
   ListExpr list;
   bool success = nl->ReadFromString(queryAsListStr,list);
   if(!success){
     // cout << "problem in converting to a list" << endl;
      res->SetDefined(false);
   } else {
      list = nl->Second(list);
    //  cout << "List successful created : " << nl->ToString(list) << endl;
      // step 2: create a new QueryProcessor
      QueryProcessor* qpp =  new QueryProcessor( nl,
                           SecondoSystem::GetAlgebraManager() );
      //step 3 : Built operator tree
      bool correct, evaluable, defined, isFunction;
      OpTree tree;
      ListExpr resultType;
    //  cout << "call Construt" << endl;
      try{
        qpp->Construct( list, correct,
                      evaluable, defined, isFunction, tree, resultType ); 
       
      } catch(int i){
         cout << "int error" <<  i << endl;
      }
       
     // cout << "Construct finished" << endl; 
      if( ! correct || !evaluable || !defined || isFunction){
       //  cout << "correct = " << correct << endl;
       //  cout << "evaluable = " << evaluable << endl;
       //  cout << "defined = " << defined << endl;
       //  cout << "isFunction = " << isFunction << endl;
         res->SetDefined(false);
      } else {
         if(!CcInt::checkType(resultType)){
          //  cerr << "Reuslt not of type int" << endl;
            res->SetDefined(false);
         } else {
          //  cerr << "ok, evaluate " << endl;
            // step 4: evaluate
            Word qResult;
            qpp->EvalS( tree, qResult, OPEN );
            // step 5: destroy operator tree
            qpp->Destroy( tree, false );
            // step 5: process result;
            CcInt* iqResult = (CcInt*)  qResult.addr;
            (*res) =   (*iqResult);
             // step 6: delete result
             iqResult->DeleteIfAllowed();
       
             
         }
      }
   delete qpp;
   }
   return 0; 
}


OperatorSpec pointerTestSpec(
           "int x int -> int",
           " pointerTest(_,_)",
           "Adds two integers using pointers and subqueries ",
           " query pointerTest(8,9) ");



Operator pointerTest
  (
  "pointerTest",             //name
   pointerTestSpec.getStr(),         //specification
   pointerTestVM,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
   pointerTestTM        //type mapping
  );


/*
5.38 Operator getQueryNL

*/
ListExpr getQueryNLTM(ListExpr args){
  if(!nl->HasLength(args,1)){
     return listutils::typeError("1 argument expected");
  }
  if(!FText::checkType(nl->First(args))){
     return listutils::typeError("text expected");
  }
  return listutils::basicSymbol<FText>();
}

int getQueryNLVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
   FText* arg = (FText*) args[0].addr;
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   if(!arg->IsDefined()){
      res->SetDefined(false);
   } else {
      SecParser sp;
      string le;
      if(sp.Text2List(arg->GetValue(),le)!=0){
         res->SetDefined(false);
      } else {
        res->Set(true,le);
     }
   }
   return 0;
}



OperatorSpec getQueryNLSpec(
           "text -> text",
           " getQueryNL(_)",
           " Returns the nested list representation of a query ",
           " query getQueryNL('query ten feed consume') ");



Operator getQueryNL
  (
  "getQueryNL",             //name
   getQueryNLSpec.getStr(),         //specification
   getQueryNLVM,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
   getQueryNLTM        //type mapping
  );


/*
5.39 Operator getOpTreeNL

*/
ListExpr getOpTreeNLTM(ListExpr args){
  if(!nl->HasLength(args,1)){
     return listutils::typeError("1 argument expected");
  }
  if(!FText::checkType(nl->First(args))){
     return listutils::typeError("text expected");
  }
  return listutils::basicSymbol<FText>();
}

int getOpTreeNLVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   FText* arg = (FText*) args[0].addr;
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   res->SetDefined(false);
   if(!arg->IsDefined()){
      return 0;
   } 
   SecParser sp;
   string les;
   if(sp.Text2List(arg->GetValue(),les)!=0){
     return 0;
   }
   ListExpr qle;
   if(!nl->ReadFromString(les,qle)){
     return 0;
   }
   bool correct;
   bool evaluable;
   bool defined;
   bool isFunction;
   OpTree tree=0;
   ListExpr resType;
   if(nl->HasLength(qle,2) && listutils::isSymbol(nl->First(qle),"query")){
      qle = nl->Second(qle);
   }
   qp->Construct(qle, correct, evaluable,
                 defined, isFunction, tree, resType);
   if(!correct || !evaluable || tree==0 ){
      if(tree){
        qp->Destroy(tree,true);
      }
      return 0;
   }
   stringstream ss;
   ListExpr r = qp->GetSimpleList(tree);
   qp->Destroy(tree,true);
   res->Set(true,nl->ToString(r));
   return 0;
}



OperatorSpec getOpTreeNLSpec(
           "text -> text",
           " getOpTreeNL(_)",
           " Returns the nested list representation of a query tree",
           " query getOpTreeNL('query ten feed consume') ");



Operator getOpTreeNL
  (
  "getOpTreeNL",             //name
   getOpTreeNLSpec.getStr(),         //specification
   getOpTreeNLVM,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
   getOpTreeNLTM        //type mapping
  );


/*
5.41 GetOpName

Returns an operator name if algebra id and operator id are given.

*/
ListExpr getOpNameTM(ListExpr args){
   string err = "int x int expected";
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }
   if(!CcInt::checkType(nl->First(args)) ||
      !CcInt::checkType(nl->Second(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<CcString>();
}


int getOpNameVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
   CcInt* algId = (CcInt*) args[0].addr;
   CcInt* opId = (CcInt*) args[1].addr;
   result = qp->ResultStorage(s);
   CcString* res = (CcString*) result.addr;
   if(!algId->IsDefined() || !opId->IsDefined()){
      res->SetDefined(false);
      return 0; 
   }
   string name; 
   Operator* op= am->GetOP(algId->GetIntval(),opId->GetIntval());
   if(!op){
      res->SetDefined(false);
      return 0; 
   }
   res->Set(true,op->GetName());
   return 0;
}

OperatorSpec getOpNameSpec(
           "int x int -> string",
           " getOpName(algId,opId)",
           " Returns the name of an operator for given algebra id "
           "and operator id",
           " query getOpName('1,10') ");


Operator getOpName
  (
  "getOpName",             //name
   getOpNameSpec.getStr(),         //specification
   getOpNameVM,        //value mapping
   Operator::SimpleSelect,   //trivial selection function
   getOpNameTM        //type mapping
  );



/*
4.29 Operator regexmatch

4.29.1 TypeMapping

Signature is:
   text x regex -> bool
   string x regex -> bool

*/
ListExpr regexmatchTM(ListExpr args){
  string err = "{string,text} x {regex, regex2} expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(!RegExPattern::checkType(nl->Second(args)) &&
     !RegExPattern2::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->First(args)) &&
     !FText::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}

/*
4.29.2 Value Mapping 

*/
template<class T, class P>
int regexmatchVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   T* arg = (T*) args[0].addr;
   P* p = (P*) args[1].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!arg->IsDefined() || !p->IsDefined()){
      res->SetDefined(false);
   } else {
      bool m = p->matches(arg->GetValue());
      res->Set(true,m);
   }
   return  0;
}

/*
4.29.3 Value Mapping Array and Selection function

*/

ValueMapping regexmatchVMs[] = {
     regexmatchVM<CcString, RegExPattern>,
     regexmatchVM<FText, RegExPattern>,
     regexmatchVM<CcString, RegExPattern2>,
     regexmatchVM<FText, RegExPattern2>
   };

int regexmatchSelect(ListExpr args){
   int res = 0;
   if(CcString::checkType(nl->First(args))){
      res = 0;
   } else {
      res = 1;
   }
   if(RegExPattern2::checkType(nl->Second(args))){
       res += 2;
   }
   return res;
}


OperatorSpec regexmatchSpec(
           "{string,text} x {regex, regex2} -> bool",
           " regexmatch(_,_)",
           " Checks whether the text matches the pattern ",
           " query regexmatch('secondo' [const regex value '.*cond.*'])");

/*
4.29.4 Operator instance

*/
Operator regexmatches(
    "regexmatches",
    regexmatchSpec.getStr(),
    4,
    regexmatchVMs,
    regexmatchSelect,
    regexmatchTM 
  );


/*
4.30 Operator startsReg

Signature: {text,string} x regex -> bool

*/
ListExpr startsRegTM(ListExpr args){
  string err = "{string,text} x {regex, regex2} expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(!RegExPattern::checkType(nl->Second(args)) &&
     !RegExPattern2::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->First(args)) &&
     !FText::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();

}

/*
4.30.2 Value Mapping 

*/
template<class T, class P>
int startsRegVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   T* arg = (T*) args[0].addr;
   P* p = (P*) args[1].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!arg->IsDefined() || !p->IsDefined()){
      res->SetDefined(false);
   } else {
      bool m = p->starts(arg->GetValue());
      res->Set(true,m);
   }
   return  0;
}


/*
4.30.3 Value Mapping Array and Selection function

*/

ValueMapping startsRegVMs[] = {
     startsRegVM<CcString, RegExPattern>,
     startsRegVM<FText, RegExPattern>,
     startsRegVM<CcString, RegExPattern2>,
     startsRegVM<FText, RegExPattern2>
 };

int startsRegSelect(ListExpr args){
   int res = 0;
   if(CcString::checkType(nl->First(args))){
      res = 0;
   } else {
      res = 1;
   }
   if(RegExPattern2::checkType(nl->Second(args))){
     res += 2;
   }
   return res;
}


OperatorSpec startsRegSpec(
           "{string,text} x {regex,regex2} -> bool",
           " startsReg(_,_)",
           " Checks whether the text starts with the pattern ",
           " query startsWith('secondo' [const regex value '.*cond'])");

/*
4.29.4 Operator instance

*/
Operator startsReg(
    "startsreg",
    startsRegSpec.getStr(),
    4,
    startsRegVMs,
    startsRegSelect,
    startsRegTM 
  );


/*
4.30 Operator ~searchPattern~

4.30.1 TypeMapping

Signature: {text,string} x {regex,regex2} [ x bool] -> 
stream(tuple( (P1 : int), (P2 : int)))

*/

ListExpr findPatternTM(ListExpr args){
  string err = "{string,text} x {regex, regex2} [x bool [x bool]] expected";
  int len = nl->ListLength(args);
  if((len!=2) && (len!=3) && (len!=4)){
     return listutils::typeError(err);
  }
  if(!RegExPattern::checkType(nl->Second(args)) &&
     !RegExPattern2::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  if(!CcString::checkType(nl->First(args)) &&
     !FText::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  ListExpr attrList = nl->TwoElemList(
              nl->TwoElemList( nl->SymbolAtom("P1"),
                               listutils::basicSymbol<CcInt>()),
              nl->TwoElemList( nl->SymbolAtom("P2"),
                               listutils::basicSymbol<CcInt>()));

  ListExpr reslist = nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
             nl->TwoElemList(
                  listutils::basicSymbol<Tuple>(),
                   attrList));

  if(len == 2){
    // use FALSE, FALSE  as default values for boolean Params
    return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                            nl->TwoElemList(nl->BoolAtom(false),
                                            nl->BoolAtom(false)),
                            reslist);

  }

  if(!CcBool::checkType(nl->Third(args))){ // check 1 optional arg
     return listutils::typeError("third argument must be of type bool");
  }
  if(len==3){
    return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                            nl->OneElemList(nl->BoolAtom(false)),
                            reslist);
  }
  // len == 4
  if(!CcBool::checkType(nl->Fourth(args))){ // check 1 optional arg
     return listutils::typeError("fourth argument must be of type bool");
  }
  return reslist;  

 
}

/*
4.30.2 Value Mapping

*/
template<class T, class P>
class FindPatternInfo{

 public:
    FindPatternInfo( T* t, P* p,      // text, pattern
                     CcBool* findMaxLength, // search for maximum length?
                     CcBool* allowEmpty, // allow empty matches
                     ListExpr tupleType){
      if(!t->IsDefined() || !p->IsDefined()){
         text = "";
         pattern = 0;
         pos = 0;
         tt = 0;
      } else {
         text = t->GetValue();
         pattern = p;
         pos = 0;
         tt = new TupleType(tupleType);
      }
      findMax = (findMaxLength->IsDefined() && findMaxLength->GetBoolval());
      this->allowEmpty = (allowEmpty->IsDefined() && allowEmpty->GetBoolval());
    }

   ~FindPatternInfo(){
      if(tt){
         tt->DeleteIfAllowed();
      }
      pattern = 0;
   }

   Tuple* next(){
      if(!pattern){
        return 0;
      }
      int length;
      while(pos < text.length()){
         if(pattern->starts2(text,pos,length, findMax,allowEmpty)){
            Tuple* res = new Tuple(tt);
            res->PutAttribute( 0 , new CcInt(true,pos));
            res->PutAttribute( 1 , new CcInt(true,pos+length));
            if(length>0){
               pos += length;
            } else {
               pos++;
            }  
            return res;
        
         }
         pos++;
      }
      return 0;
   }

 private:
   string text;
   P* pattern;
   unsigned int pos;
   TupleType* tt;
   bool findMax;
   bool allowEmpty;
};

template<class T, class P>
int findPatternVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
   FindPatternInfo<T,P>* li = (FindPatternInfo<T,P>*) local.addr;
   switch(message){
      case OPEN: {
          if(li){
             delete li;
          }
          local.addr = new FindPatternInfo<T,P> ( (T*) args[0].addr,
                                          (P*) args[1].addr,
                                          (CcBool*) args[2].addr,
                                          (CcBool*) args[3].addr,
                                          nl->Second(GetTupleResultType(s)));
          return 0;  
      }
      case REQUEST: {
          result.addr = li?li->next():0;
          return result.addr?YIELD:CANCEL;
      }
      case CLOSE : {
         if(li){
            delete li;
            local.addr = 0;
         }
         return 0;
      }
   }
   return -1;
}

/*
4.30.3 Value Mapping Array and Selection function

*/

ValueMapping findPatternVMs[] = {
     findPatternVM<CcString, RegExPattern>,
     findPatternVM<FText, RegExPattern>,
     findPatternVM<CcString, RegExPattern2>,
     findPatternVM<FText, RegExPattern2>
 };

int findPatternSelect(ListExpr args){
   int res = 0;
   if(CcString::checkType(nl->First(args))){
      res = 0;
   } else {
      res = 1;
   }
   if(RegExPattern2::checkType(nl->Second(args))){
     res += 2;
   }
   return res;
}


OperatorSpec findPatternSpec(
           "{string,text} x {regex,regex2} [ x bool [x bool]] -> "
           "stream(tuple((P1 : int, P2: int)))",
           " findPattern(text,pattern, findMax, allowEmpty)",
           " Returns all positions of regex in text. P1 ist the index where"
           " matched part of the string starts, P2 is the position after the"
           "matched pattern ends."
           "If the optional  parameter findMax is set to TRUE (default false)",
           ", patterns of maximum lengths are searched, pattern of minimum "
           " length otherwise. The boolean parameter allowEmpty controls "
           "whether finding the empty word holds a match.",
           " query findPattern('Secondo', [const regex2 value 'o']) count");

/*
4.29.4 Operator instance

*/
Operator findPattern(
    "findPattern",
    findPatternSpec.getStr(),
    4,
    findPatternVMs,
    findPatternSelect,
    findPatternTM 
  );

/*
4.31 Operators createRegEx / createRegEx2

4.31.1 Type Mapping 

*/
template<bool two>
ListExpr createRegExTM(ListExpr args){
  string err = "string or text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }

  if(!CcString::checkType(nl->First(args)) &&
     !FText::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return two?listutils::basicSymbol<RegExPattern2>()
            :listutils::basicSymbol<RegExPattern>();
}

/*
4.31.2 Value Mapping

*/
template<class T, class P>
int createRegExVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  T* arg = (T*) args[0].addr;
  P* res = (P*)result.addr;
  if(!arg->IsDefined()){
     res->SetDefined(false);
  } else {
     res->constructFrom(arg->GetValue());
  }
  return 0;
}

/*
4.31.3 Value Mapping Arrays and Selection function

*/

ValueMapping createRegExVMs[] = {
      createRegExVM<CcString,RegExPattern>,  
      createRegExVM<FText,RegExPattern>  
 };

ValueMapping createRegEx2VMs[] = {
      createRegExVM<CcString,RegExPattern2>,  
      createRegExVM<FText,RegExPattern2>  
 };

int createRegExSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

/*
4.31.4 Specifications

*/
OperatorSpec createRegExSpec(
           "{string,text}  -> regex",
           " createRegEx(_)",
           " Creates a regex pattern from a string/text ",
           " query createRegEx(\".*ob?a\")");

OperatorSpec createRegEx2Spec(
           "{string,text}  -> regex2",
           " createRegEx2(_)",
           " Creates a regex2 pattern from a string/text ",
           " query createRegEx2(\".*ob?a\")");

/*
4.31.5 Operator instances

*/
Operator createRegEx(
    "createRegEx",
    createRegExSpec.getStr(),
    2,
    createRegExVMs,
    createRegExSelect,
    createRegExTM<false> 
  );

Operator createRegEx2(
    "createRegEx2",
    createRegEx2Spec.getStr(),
    2,
    createRegEx2VMs,
    createRegExSelect,
    createRegExTM<true> 
  );

/*
4.32 Operator numOfFlobs

4.32.1 Type Mapping

*/
ListExpr numOfFlobsTM(ListExpr args){
 string err = "DATA expected";
 if(!nl->HasLength(args,1)){
   return listutils::typeError(err + " (wrong number of args)");
 }
 if(!Attribute::checkType(nl->First(args))){
   return listutils::typeError(err);
 }
 return  listutils::basicSymbol<CcInt>(); 
}

/*
4.32.2 Value Mapping 

*/
int numOfFlobsVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
  Attribute* arg = (Attribute*) args[0].addr;
  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  res->Set(true, arg->NumOfFLOBs()); 
  return 0;
}

/*
4.32.3 Specification

*/

OperatorSpec numOfFlobsSpec(
           "DATA -> int",
           "numOfFlobs(_)",
           "Returns the number of flobs used by an attribute ",
           "query numOfFlobs(1)");


/*
4.32.4 Operator instance

*/

Operator numOfFlobs(
    "numOfFlobs",
    numOfFlobsSpec.getStr(),
    numOfFlobsVM, 
    Operator::SimpleSelect,
    numOfFlobsTM 
  );

/*
4.33 Operators flobSize / flobMemSize

4.33.1 Type Mapping

*/
ListExpr flobSizeTM(ListExpr args){
  string err = "DATA x int expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  if(!Attribute::checkType(nl->First(args)) ||
     !CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();
}

/*
4.33.2 Value Mapping

*/
template<bool memsize>
int flobSizeVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  Attribute* attr = (Attribute*) args[0].addr;
  CcInt* flob = (CcInt*) args[1].addr;
  if(!flob->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  int flobi = flob->GetValue();
  if((flobi<0) || (flobi>=attr->NumOfFLOBs())){
    res->SetDefined(false);
    return 0;
  }
  if(memsize){
    res->Set(true, attr->GetFLOB(flobi)->getUncontrolledSize());
  } else {
    res->Set(true, attr->GetFLOB(flobi)->getSize());
  }
  return 0;
}

/*
4.33.3 Specification

*/

OperatorSpec flobSizeSpec(
           "DATA x int -> int",
           "flobSize(_,int)",
           "Returns the size of a flob ",
           "query flobSize(thecenter,0)");


OperatorSpec flobMemSizeSpec(
           "DATA x int -> int",
           "flobSize(_,int)",
           "Returns the size of a flob in main memory",
           "query flobMemSize(thecenter,0)");

/*
4.33.4 Operator instances

*/
Operator flobSize(
    "flobSize",
    flobSizeSpec.getStr(),
    flobSizeVM<false>, 
    Operator::SimpleSelect,
    flobSizeTM 
  );

Operator flobMemSize(
    "flobMemSize",
    flobMemSizeSpec.getStr(),
    flobSizeVM<true>, 
    Operator::SimpleSelect,
    flobSizeTM 
  );

/*
4.34 Operator sizeOf

4.35.1 Type Mapping

*/
ListExpr sizeOfTM(ListExpr args){
 string err = "DATA expected";
 if(!nl->HasLength(args,1)){
   return listutils::typeError(err + " (wrong number of args)");
 }
 if(!Attribute::checkType(nl->First(args))){
   return listutils::typeError(err);
 }
 return  listutils::basicSymbol<CcInt>(); 
}

/*

4.35.2 Value Mapping

*/

int sizeOfVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
  Attribute* arg = (Attribute*) args[0].addr;
  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  res->Set(true, arg->Sizeof()); 
  return 0;
}


OperatorSpec sizeOfSpec(
           "DATA -> int",
           "sizeOf(_)",
           "Applies sizeof to an attribute. ",
           "query sizeOf(theCenter) > 0");


/*
4.35.4 Operator instance

*/

Operator sizeOf(
    "sizeOf",
    sizeOfSpec.getStr(),
    sizeOfVM, 
    Operator::SimpleSelect,
    sizeOfTM 
  );


/*
4.36 Operator tmcheck

This operator is for finding error within type map functions. 
It takes a text representing a valid query. If the query is not 
valid, the result stream will be empty. If the text represents a
valid query, the operator tree is built. After that, for each operator 
node in the tree, all matching operators are searched. If an expection 
occurs, the appropriate operator together with the input list will be 
put into the output stream. In case of an assertion, Secondo will crash. 


4.36.1 Type Mapping

The signature is: 

 {text,stream(text)} x string [x bool] -> 
stream(tuple([AlgName: string, OpName : string, Input : text]))

*/

ListExpr tmcheckTM(ListExpr args){
   if(    ! nl->HasLength(args,2)
       && !nl->HasLength(args,3)){
      return listutils::typeError("expected 1 or two argument");
   }
   if(!FText::checkType(nl->First(args)) &&
      !Stream<FText>::checkType(nl->First(args))){
      return listutils::typeError("expected text or "
                               "stream(text) as first argument");
   }
   if(!CcString::checkType(nl->Second(args))){
      return listutils::typeError("second argument must be a string");
   }

   ListExpr resType= nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                  nl->ThreeElemList(
                        nl->TwoElemList( nl->SymbolAtom("AlgName"),
                                 listutils::basicSymbol<CcString>()),
                        nl->TwoElemList( nl->SymbolAtom("OpName"),
                                 listutils::basicSymbol<CcString>()),
                        nl->TwoElemList( nl->SymbolAtom("Input"),
                                 listutils::basicSymbol<FText>()))));

   if(nl->HasLength(args,2)){ // optional bool missing
     return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->BoolAtom(false)), 
                resType);
   }
   if(!CcBool::checkType(nl->Third(args))){
     return listutils::typeError("third argument must be of type bool");
   }

   return nl->ThreeElemList(
               nl->SymbolAtom(Symbol::APPEND()),
               nl->OneElemList(nl->BoolAtom(false)),
               resType);
}


/*
4.36.2 tmcheckLocalInfo


*/
class tmcheckLocalInfo{

   public:

/*
~Constructor~

*/
      tmcheckLocalInfo(FText* text, CcString* _algName, 
                       CcBool* p, ListExpr resType){
        currentNL = nl->TheEmptyList();
        tt = new TupleType(resType);
        print = p && p->IsDefined() && p->GetValue();
        if(text && text->IsDefined()){
          findInputLists(text->GetValue());
        }
        this->algName = "";
        if(_algName && _algName->IsDefined()){
          this->algName = _algName->GetValue();
        }
      }

/*
~Destructor~

*/
      ~tmcheckLocalInfo(){
          tt->DeleteIfAllowed();
       }


/*
~next~

Returns the next pair of operator name and input list
throwing an exception during type mapping of the operator.

*/
      Tuple* next(){
       if(!failedOperators.empty()){
         return createTuple();
       }
       while(!inputLists.empty()){
          ListExpr list = inputLists.front();
          inputLists.pop();
          findFailures(list);
          if(!failedOperators.empty()){
            currentNL = nl->ToString(list);
            return createTuple();
          }
       }
       return 0;
      }



   protected:
     queue<ListExpr> inputLists;
     string currentNL;
     queue<pair<string,string> > failedOperators;
     TupleType* tt;
     bool print;
     string algName;


/*
Creates a new result typle from ~currentNL~ and 
the top of the failedOperators queue.

*/

   Tuple* createTuple(){
      Tuple* res = new Tuple(tt);
      string algName = failedOperators.front().first;
      string opName = failedOperators.front().second;
      failedOperators.pop();
      res->PutAttribute(0, new CcString(true,algName));
      res->PutAttribute(1, new CcString(true,opName));
      res->PutAttribute(2, new FText(true,currentNL));
      return res;
   } 



/*
~findInputLists~

Builds an operator tree from a command and stores all
argumentslists of all operator nodes within the tree 
into inputLists.

*/

   void findInputLists(string query){


     stringutils::trim(query);
     if(query.size()==0){ //empty
        return; 
     }

     if(print){
        cout << "process query :" + query << endl;
     }

     try{
       // first, build the nested list query from query string
       SecParser parser;
       string queryListText;
       ListExpr queryList;
       if(parser.Text2List(query,queryListText)!=0){
          cerr << "error in parsing query:" << query << endl;
          return;
       }
       if(!nl->ReadFromString(queryListText, queryList)){
         cerr << "SecParser has produced an invalid listi for query " 
              << query << endl;
         return;
       }
  
  
       if(!nl->HasLength(queryList,2)){  // ( query <expr> )
          cerr << "invalid command:" << query << endl; 
          return;
       }
       ListExpr exprList = nl->Second(queryList);
       // construct the operator tree
       OpTree tree = 0;
       bool correct = false;
       bool evaluable = false;
       bool defined= false;
       bool isFun = false;
       ListExpr resType;
       QueryProcessor qpp(nl,am);
       qpp.Construct(exprList, correct, evaluable, defined, 
                      isFun,tree,resType);
       if(!correct || !evaluable){
         cerr << " not a valid expression:" << query << endl;
         if(tree){
            qpp.Destroy(tree,true);
         }
         return;
       } 
       if(tree){
         findInputLists(tree,qpp);
         qpp.Destroy(tree,true);
       }
     } catch(runtime_error& err) {
        cerr << "ERROR" << err.what() << endl;
        cerr << "Query : " << query << endl << " --- " << endl;
     } catch(SI_Error err){
        cerr << "SI ERROR: " << SmiEnvironment::Err2Msg(err) << endl;
        cerr << "Query : " << query << endl << " --- " << endl;
     } catch(...){
        cerr << "Error occurred" << endl;
        cerr << "Query is " << query << endl;
     }
  }


/*
~findInputLists~

Traverses the operator tree rooted by s and stores the arguments 
of all operator nodes to inputLists.

*/
   void findInputLists(Supplier s, QueryProcessor& qpp){
     if(qpp.IsOperatorNode(s)){
        addList(s,qpp);
        for(int i=0;i<qpp.GetNoSons(s); i++){
           findInputLists(qpp.GetSon(s,i),qpp);
        }
     }
   }


/*
~addList~

Concatenates the type lists of all sons of __s__ and stores the 
result into inputLists.

*/

   void addList(Supplier s, QueryProcessor& qpp){
      if(qpp.GetNoSons(s)==0){
         inputLists.push(nl->TheEmptyList());
      } else {
        ListExpr r = nl->OneElemList( qpp.GetType(qpp.GetSon(s,0)));
        ListExpr last = r;
        for(int i=1;i<qpp.GetNoSons(s);i++){
           last = nl->Append(last, qpp.GetType(qpp.GetSon(s,1)));
        }
        inputLists.push(r);
      }
   }

/*
~findFailures~



*/
void findFailures(ListExpr args){
  am->findTMExceptions(algName, args,failedOperators, print);
}

};


class tmcheckLocalInfoStream: public  tmcheckLocalInfo{

public:
   tmcheckLocalInfoStream(Word _stream, CcString* _algName,
                          CcBool* print, ListExpr typeExpr): 
          tmcheckLocalInfo(0,_algName, print,typeExpr),
          stream(_stream) {
              stream.open();
          }

   ~tmcheckLocalInfoStream(){
        stream.close();
    }


    Tuple* next(){
       if(!failedOperators.empty()){
         return createTuple();
       }
       while(!inputLists.empty()){
          ListExpr list = inputLists.front();
          inputLists.pop();
          findFailures(list);
          if(!failedOperators.empty()){
            currentNL = nl->ToString(list);
            return createTuple();
          }
       }
       FText* text;
       while( (text = stream.request())!=0){
          if(text->IsDefined()){
             findInputLists(text->GetValue());
             text->DeleteIfAllowed();
             while(!inputLists.empty()){
                 ListExpr list = inputLists.front();
                 inputLists.pop();
                 findFailures(list);
                 if(!failedOperators.empty()){
                    currentNL = nl->ToString(list);
                    return createTuple();
                 }
              }
          } else {
             text->DeleteIfAllowed();
          }
       }
       return 0;
    }



private:
   Stream<FText> stream;

};



/*
4.36.3 Value Mapping

*/

int tmcheckVM1( Word* args, Word& result, int message,
               Word& local, Supplier s ){
   tmcheckLocalInfo* li = (tmcheckLocalInfo*) local.addr;
   switch(message){
     case OPEN: { if(li){
                     delete li;
                     local.addr = 0;
                  }
                  local.addr = new tmcheckLocalInfo((FText*)args[0].addr,
                                     (CcString*) args[1].addr,
                                     (CcBool*) args[2].addr,
                                     nl->Second(GetTupleResultType(s)));
                  return 0;
                 } 
     case REQUEST: {
                     result.addr = li?li->next():0;
                     return result.addr?YIELD:CANCEL;
                  }
     case CLOSE: {
                    if(li){
                       delete li;
                       local.addr = 0;
                    }
                    return 0;
                 }
   }
   return -1;
}


int tmcheckVM2( Word* args, Word& result, int message,
               Word& local, Supplier s ){
   tmcheckLocalInfoStream* li = (tmcheckLocalInfoStream*) local.addr;
   switch(message){
     case OPEN: { if(li){
                     delete li;
                     local.addr = 0;
                  }
                  local.addr = new tmcheckLocalInfoStream(args[0],
                                     (CcString*) args[1].addr,
                                     (CcBool*) args[2].addr,
                                     nl->Second(GetTupleResultType(s)));
                  return 0;
                 } 
     case REQUEST: {
                     result.addr = li?li->next():0;
                     return result.addr?YIELD:CANCEL;
                  }
     case CLOSE: {
                    if(li){
                       delete li;
                       local.addr = 0;
                    }
                    return 0;
                 }
   }
   return -1;
}





/*
4.36.4 Specification

*/
OperatorSpec tmcheckSpec(
         "{text, stream(text)} x string  [x bool ] -> strean(tuple([OpName : "
         "string, Input: text]))",
         "_ tmcheck[_]",
         "Calls matching operators for each operator  node of the  "
         " operator tree build by the query in the argument. "
         " All typemappings throwing an exception are put into "
         "the output stream together with the input which leads to"
         "that exception. The second argument specified a certain algebra"
         " for which the oprrators should be checks. If the value is undefined"
         " or empty, all algebras are checked. The third (optionally) argument"
         "specified if the currently processed operator should be printed out.",
         "query tmcheck('thecenter feed count') count = 0");


/*
4.36.5 ValueMapping array and selectionFunction

*/
int tmcheckSelect(ListExpr args){
   return FText::checkType(nl->First(args))?0:1;
}

ValueMapping tmcheckVM[]={tmcheckVM1,tmcheckVM2};



/*
4.36.6 Operator instance

*/




Operator tmcheckOp(
    "tmcheck",
    tmcheckSpec.getStr(),
    2,
    tmcheckVM, 
    tmcheckSelect,
    tmcheckTM 
  );

/*
4.37 Operator ~getObject~

This operator provides the database object given by its name
as a result.

4.37.1 ~getObject~ TypeMapping

Signature is:  {string,text} -> X where X depends on the 
type of the object named by the argument stored in the database.

*/

ListExpr getObjectTM(ListExpr args){

  if(!nl->HasLength(args,1)){
     return listutils::typeError("one argument expected");
  }
  ListExpr first = nl->First(args);
  if(!nl->HasLength(first,2)){
     return listutils::typeError("invalid ListStructure for an operator"
               "using evaluation in TypeMapping");
  }
  ListExpr type = nl->First(first);
  ListExpr query = nl->Second(first);
  string err = " string or text expected";

  if( !CcString::checkType(type) &&
      !FText::checkType(type)){
     return listutils::typeError(err);
  }

  // try to get the string value
  Word res; 
  bool success = QueryProcessor::ExecuteQuery(nl->ToString(query),res);
  if(!success){
     return listutils::typeError("could not evaluate the value of  " +
                                  nl->ToString(query) );
  }
  string objName = "";

  if(FText::checkType(first)){
     FText* resText = static_cast<FText*>(res.addr);
     if(!resText->IsDefined()){
         return listutils::typeError("ObjectName is undefined");
     }   
     objName = resText->GetValue();
     delete resText;
  } else { // a string
     CcString* resText = static_cast<CcString*>(res.addr);
     if(!resText->IsDefined()){
         return listutils::typeError("ObjectName is undefined");
     }   
     objName = resText->GetValue();
     delete resText;
  }
  // try to find the dataabase object
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  if(!ctlg->IsObjectName(objName)){
     return listutils::typeError("'"+objName+"' is not a database object");
  }
  return ctlg->GetObjectTypeExpr(objName);
}

/*
4.37.2 Value Mapping

*/
template<class T>
int getObjectVM1( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

   T* arg = static_cast<T*>(args[0].addr);
   if(!arg->IsDefined()){
      // should never happen, check in Type Mapping
      return 0;
   }
   string name = arg->GetValue();
   SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
   if(!ctlg->IsObjectName(name)){
      // should never happen
      return 0;
   }
   Word value;
   bool defined;
  // In windows.h GetObject is redefined.
  // The next lines deactivate that
#ifdef GetObject
#undef GetObject
#endif
  
   ctlg->GetObject(name,value,defined);
   if(!defined){
      return 0;
   }
   // remove the old stuff within the ResultStorage
   qp->DeleteResultStorage(s);

   // clone the object because now it's a result of an operator
   // first get algId and typeId 
   int algId = 0;
   int typeId = 0;
   ListExpr rtype = qp->GetType(s);
   string basicType;
   bool ok = ctlg->LookUpTypeExpr(rtype, basicType,algId,typeId);
   if(!ok){
       cerr << "Problem in LookUp TypeExpr";
       return 0;
   }

   AlgebraManager* am = SecondoSystem::GetAlgebraManager();
   Word valueClone =  am->CloneObj(algId,typeId)(rtype,value); 
   am->CloseObj(algId,typeId)(rtype,value);
   qp->ChangeResultStorage(s,valueClone);
   result = qp->ResultStorage(s);
   return 0;
}

/*
4.37.3 Value Mapping Array

*/
ValueMapping getObjectVM[] = {
  getObjectVM1<CcString>,
  getObjectVM1<FText>
};

/*
4.37.4 Selection function 

*/
int getObjectSelect(ListExpr args){

  return CcString::checkType(nl->First(args))?0:1;
}

/*
4.37.5 Specification

*/
OperatorSpec getObjectSpec(
  "{string,text} -> X ",
  "getObject(_)",
  "retrieves an Object from the database.",
  "query getObject('ten') feed count"
);


/*
4.37.6 Operator instance

*/
Operator getObjectOP(
  "getObject",
  getObjectSpec.getStr(),
  2,
  getObjectVM,
  getObjectSelect,
  getObjectTM
);

/*
4.37 Operator ~getObjects~

*/
ListExpr getObjectsTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argment expected");
  }
  ListExpr arg1 = nl->First(args);
  if(!Attribute::checkType(arg1)){
    return listutils::typeError("argument not in kind DATA");
  }
  ListExpr attrList = nl->TwoElemList(
                          nl->TwoElemList(nl->SymbolAtom("Object"), arg1),
                          nl->TwoElemList(nl->SymbolAtom("Name"), 
                                          listutils::basicSymbol<CcString>())
                      );
  return nl->TwoElemList(
           nl->SymbolAtom(Stream<Tuple>::BasicType()),
           nl->TwoElemList(
              nl->SymbolAtom(Tuple::BasicType()),
              attrList));

}

class getObjectsInfo{

 public:
    getObjectsInfo(ListExpr _type, TupleType* _tt): type(_type), tt(_tt){
       ctlg = SecondoSystem::GetCatalog();
       theObjects = ctlg->ListObjects();
       theObjects = nl->Rest(theObjects); // remove leading OBJECTS
       tt->IncReference();
    }

    ~getObjectsInfo(){
       tt->DeleteIfAllowed();
    }


    Tuple* next(){
       while(!nl->IsEmpty(theObjects)){
         ListExpr object  = nl->First(theObjects);
         theObjects = nl->Rest(theObjects);
         ListExpr otype = nl->First(nl->Fourth(object));
         if(nl->Equal(type,otype)){
           Tuple* t = createTuple(nl->SymbolValue(nl->Second(object)));
           if(t) return t;
         }
       }
       return 0;
    }

 private:
    ListExpr type;
    SecondoCatalog* ctlg;
    ListExpr theObjects;
    TupleType* tt;

    Tuple* createTuple(const string& name){
      Word word;
      bool defined;
      if(!ctlg->GetObject(name,word,defined)){
         return 0;
      }
      if(!defined) return 0;
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, (Attribute*) word.addr);
      res->PutAttribute(1, new CcString(true,name));
      return res;     
 
    }


};


int getObjectsVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

   getObjectsInfo* li = (getObjectsInfo*) local.addr;
   switch(message){
      case OPEN: {
         ListExpr theType = qp->GetType(qp->GetSon(s,0));
         TupleType* resType = new TupleType(nl->Second(GetTupleResultType(s)));
         if(li) delete li;
         local.addr = new getObjectsInfo(theType, resType);
         resType->DeleteIfAllowed();
         return 0;
      }
      case REQUEST: 
         result.addr = li?li->next():0;
         return result.addr?YIELD:CANCEL;
      case CLOSE:
         if(li){
           delete li;
           local.addr = 0;
         }
         return 0;
         
   }
   return -1;
}

OperatorSpec getObjectsSpec(
   "DATA x stream(tuple(DATA, string))",
   "getObjects(_)",
   "Returns all objects having given type as tuple stream. "
   "Restricted to attribute data types.",
   "query getObjects(theCenter) count"
);

Operator getObjectsOp(
  "getObjects",
  getObjectsSpec.getStr(),
  getObjectsVM,
  Operator::SimpleSelect,
  getObjectsTM
);



/*
4.38 Operator ~flobInfo~

This operator create a stream of text from an attribute describing the 
contained FLOB-Ids.

4.38.1 Type Mapping

*/

ListExpr flobInfoTM(ListExpr args){
  string err = "DATA expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(!Attribute::checkType(arg)){
    return listutils::typeError(err);
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<FText> >(),
                          listutils::basicSymbol<FText>());
}

/*
4.38.1  Value Mapping

*/
class flobInfoInfo{
   public:
    flobInfoInfo(Attribute* a): attr(a), pos(0), max(a->NumOfFLOBs()) {}
    FText* getNext(){
       if(pos<max){
          Flob* f = attr->GetFLOB(pos);
          pos++;
          stringstream ss;
          ss << *f;
          return new FText(true, ss.str());
       }
       return 0;
    }

  private:
     Attribute* attr;
     int pos;
     int max;
};


int flobInfoVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

   flobInfoInfo* li = (flobInfoInfo*) local.addr;
   switch(message){
      case OPEN:  if(li){
                    delete li;
                  }
                  local.addr = new flobInfoInfo((Attribute*) args[0].addr);
                   return 0;
      case REQUEST:
                  result.addr = li?li->getNext():0;
                  return result.addr?YIELD:CANCEL;
      case CLOSE:
                 if(li){
                    delete li;
                    local.addr = 0;
                 }
                 return 0;
   }
   return  -1;
}

/*
4.38.3 Specification

*/
OperatorSpec flobInfoSpec(
  "DATA -> stream(text)",
  "flobInfo(_)",
  "Returns the flob ID's of an attribute data type.",
  "query flobInfo(msnow) count"
);

/*
4.38.4 Operator instance

*/

Operator flobInfoOP(
  "flobInfo",
  flobInfoSpec.getStr(),
  flobInfoVM,
  Operator::SimpleSelect,
  flobInfoTM
);


/*
4.39 Operator correctFileId

This operator may be useful make a corrupt database usuable. 
It sets the internal counter of the file creation to the
highest used number in file names.

4.39.1 Type Mapping

Signature is: -> bool

*/
ListExpr correctFileIdTM(ListExpr args){
  if(nl->IsEmpty(args)){
    return listutils::basicSymbol<CcBool>();
  } else {
    return listutils::typeError("No argument allowed");
  }
}

/*
4.39.2 Value Mapping

*/
int correctFileIdVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   res->Set(true, SmiEnvironment::correctFileId());
   return 0;
}


/*
4.39.3 Specification

*/
OperatorSpec correctFileIdSpec(
  "-> bool",
  "correctFileId()",
  "Internal use only. May be useful to repair corrupt databases",
  "query correctFileId()"
);

/*
4.38.4 Operator instance

*/

Operator correctFileIdOP(
  "correctFileId",
  correctFileIdSpec.getStr(),
  correctFileIdVM,
  Operator::SimpleSelect,
  correctFileIdTM
);


/*
4.39 Operator charCodes

4.39.1 Type Mapping

The type mapping is 

{text,string} x string -> stream(longint)

The first argument is the text, the second argument the encoding.
See LocalInfo for implemented encodings.

*/
ListExpr charCodesTM(ListExpr args){
  string err = "{string,text} x string expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr arg1 = nl->First(args);
  if(   !CcString::checkType(arg1)
     && !FText::checkType(arg1)){
     return listutils::typeError(err + " (first argument has wrong type)");
  }
  if(!CcString::checkType(nl->Second(args))){
     return listutils::typeError(err + " (second argument has wrong type)");
  }

  return nl->TwoElemList(
               listutils::basicSymbol<Stream<LongInt> >(),
               listutils::basicSymbol<LongInt>());
}

/*
4.39.2 LocalInfo

*/
enum charCodeEncoding { ASCII, UTF8 };

template<class T>
class charCodesLI{
   public:
      charCodesLI(T* text, CcString* encoding){
         if(!text->IsDefined() || !encoding->IsDefined()){
            done = true;
            return;
         }
         string enc = encoding->GetValue();
         if(enc =="ASCII"){
            this->encoding = ASCII;
            done = false;
         } else if(enc=="UTF-8"){
            this->encoding = UTF8;
            done = false;
         } else {
            done = true;
         }
         if(done){
            return;
         }
         this->text = text->GetValue();
         pos = 0;
         length = this->text.length();
         done = pos==length;
      }

      LongInt* nextInt(){
          if(done){
            return 0;
          }
          switch(encoding){
            case ASCII: {
                   unsigned char res = text[pos];
                   pos++;
                   done = pos==length;
                   return new LongInt(true,res); 
                 }
            case UTF8 : {
                   return nextUTF8();
                 }
            default: return 0;
          }
      }


   private:
      string text;
      charCodeEncoding encoding;
      bool done;
      size_t pos;
      size_t length;

      LongInt* nextUTF8(){
         int numFollows = 0;
         unsigned char mask = 0x80;
         unsigned char first = text[pos];
         while( ((mask & first) > 0)){
            mask = mask >> 1;
            numFollows++;
         }

         if(numFollows == 0){ // single char symbol
            pos++;
            done = pos==length;
            return new LongInt(true,first);
         }
         numFollows--; // in the counting abouve also the first byte is counted
         if(pos + numFollows >=length){ // not enough chars available
            done = true;
            return new LongInt(false,0);
         }
         // extract the used bits from the first character;
         unsigned char mask2 = mask;
         while(mask>0){
           mask2 = mask2 | mask;
           mask = mask >> 1;
         } 
         int64_t value = first & mask2;
         // read follow bytes
         pos++;
         for(int i=0;i<numFollows;i++){
            int64_t follow = text[pos];  
            value = (value << 6) | (follow & 0x3F);
            pos++;
            // check the first 2 bits
            follow = follow & 0xC0;
            if(follow != 0x80){ // error in UTF8
                done = true;
                return new LongInt(false,0);
            } 
         } 
         done = pos==length;
         return new LongInt(true,value);
      }
};

template<class T>
int charCodesVM1( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

 charCodesLI<T>* li = (charCodesLI<T>*) local.addr;
 switch(message){
    case OPEN: if(li){
                  delete li;
               }
               local.addr = new charCodesLI<T>((T*) args[0].addr, 
                                       (CcString*) args[1].addr);
               return 0;
    case REQUEST: result.addr=li?li->nextInt():0;
                  return result.addr?YIELD:CANCEL;
    case CLOSE: if(li){
                  delete li;
                }
                local.addr = 0;
                 return 0; 
 }
 return -1;

}

/*
4.37.4 Selection function 

*/

ValueMapping charCodesVM[] = {
  charCodesVM1<CcString>,
  charCodesVM1<FText>
};

int charCodesSelect(ListExpr args){

  return CcString::checkType(nl->First(args))?0:1;
}

/*
4.37.5 Specification

*/
OperatorSpec charCodesSpec(
  "{string,text} x string  -> stream(longint) ",
  " charCodes( t, encoding)",
  "Returns the stream of character codes of t using the "
  " specified encoding. Implemented encodings are ASCII and "
  " UTF-8. If another encoding is used as an argument, an "
  " empty stream is returned. If the text does not fit the "
  " encoding as much as possible character codes are returned "
  " At the position where the error occurs, the result char code"
  " will be undefined and the output is stopped." ,
  "query charCodes('abc',\"ASCII\") count"
);


/*
4.37.6 Operator instance

*/
Operator charCodesOP(
  "charCodes",
  charCodesSpec.getStr(),
  2,
  charCodesVM,
  charCodesSelect,
  charCodesTM
);


/*
4.38 Operator morseWav

4.38.1 Type Mapping

   {string, text} x text -> bool

*/
ListExpr morseWavTM(ListExpr args){

    string err = "{string,text} x text  x int x int expected";
    if(!nl->HasLength(args,4)){
      return listutils::typeError(err);
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if(!CcString::checkType(arg1) &&
       !FText::checkType(arg1)){
      return listutils::typeError(err);
    }
    if(!FText::checkType(arg2)){
      return listutils::typeError(err);
    }
    if(!CcInt::checkType(nl->Third(args))){
      return listutils::typeError(err);
    }
    if(!CcInt::checkType(nl->Fourth(args))){
      return listutils::typeError(err);
    }
    return listutils::basicSymbol<CcBool>();
}

template<class T>
class createMorse{

  public:
    createMorse(T* _text, FText* _fileName, int _speed, int _freq, 
                int _sampleRate, int _sampleSize):
     defined(true), text(""),fileName(""), speed(_speed), freq(_freq), 
     sampleRate(_sampleRate),
     sampleSize(_sampleSize), dit(0), dah(0), pause(0), morseMap() {
       // check validity
       if(!_text->IsDefined() || !_fileName->IsDefined()){
         defined= false;
         return;
       }
       if(_speed < 20 || _speed > 300){
          defined = false;
          return;
       } 
       if(_freq<20 || _freq > 20000){
          defined = false;
          return;
       }
       if(_sampleRate< 2000 || _sampleRate > 60000 || _sampleRate < _freq*2){
          defined = false;
          return;
       } 
       if(   sampleSize!=8 && sampleSize!=16 && sampleSize != 24 
          && sampleSize!=32){
          defined = false;
          return;
       }
       this->text = _text->GetValue();
       this->fileName = _fileName->GetValue();
       init();
    }

    ~createMorse(){
       if(dit) { delete[] dit;}
       if(dah){ delete[] dah; }
       if(pause){ delete[] pause; }
     }

     bool createWav(){
        if(!defined){
           return false;
        }
        ofstream out(fileName.c_str(),ios::binary|ios::trunc);
        if(!out.good()){
          return false;
        }
        writeHeader(out);

        bool lastSpace = true;
        for(size_t pos=0;pos<text.length() && out.good();pos++){
          try{
            char c = tolower(text[pos]);
            string code = morseMap.at(c);
            writeCode(out,code, lastSpace); 
          } catch(...){}
        }

        out.flush();
        streampos length = out.tellp();
        out.seekp(4,ios::beg);
        int32_t l1 = (int32_t) length - 8;
        out.write((char*)&l1,4);
        out.seekp(40,ios::beg);
        int32_t l2 = (int32_t) length -44;
        out.write((char*) &l2, 4); 
        bool res = out.good();
        out.close();
        return res;             
     }
 

  private:
    bool defined;
    string text;
    string fileName;
    int speed;
    int freq;
    int32_t sampleRate;
    int16_t sampleSize;
    char* dit;
    char* dah;
    char* pause;
    int ditSize;
    int dahSize;
    int pauseSize;
    map<char,string> morseMap; 


    void init(){
      fillMap();
      int ditsPerMinute = (speed*50)/6;
      int t = 60000/ditsPerMinute; // ms for a dit
      dit = createTone(t, ditSize);
      dah = createTone(3*t, dahSize);
      pause = createPause(t, pauseSize);
    }

    void fillMap(){
       morseMap['a'] = ".-";
       morseMap['b'] = "-...";
       morseMap['c'] = "-.-.";
       morseMap['d'] = "-..";
       morseMap['e'] = ".";
       morseMap['f'] = "..-.";
       morseMap['g'] = "--.";
       morseMap['h'] = "....";
       morseMap['i'] = "..";
       morseMap['j'] = ".---";
       morseMap['k'] = "-.-";
       morseMap['l'] = ".-..";
       morseMap['m'] = "--";
       morseMap['n'] = "-.";
       morseMap['o'] = "---";
       morseMap['p'] = ".--.";
       morseMap['q'] = "--.-";
       morseMap['r'] = ".-.";
       morseMap['s'] = "...";
       morseMap['t'] = "-";
       morseMap['u'] = "..-";
       morseMap['v'] = "...-";
       morseMap['w'] = ".--";
       morseMap['x'] = "-..-";
       morseMap['y'] = "-.--";
       morseMap['z'] = "--..";
       morseMap['0'] = "-----";
       morseMap['1'] = ".----";
       morseMap['2'] = "..---";
       morseMap['3'] = "...--";
       morseMap['4'] = "....-";
       morseMap['5'] = ".....";
       morseMap['6'] = "-....";
       morseMap['7'] = "--...";
       morseMap['8'] = "---..";
       morseMap['9'] = "----.";
       morseMap[' '] = " ";
    }

    char* createTone(int ms,int& size){
      int bytesPerSample = (sampleSize +7) / 8;
      int size1 = (ms*sampleRate)/  1000 ;
      size = bytesPerSample*size1;
      char* res = new char[size];
      double step = (2*M_PI*freq)/((double)sampleRate);
      double x = 0;
      double factor = 0.9*( 1 << (sampleSize-1));
      char data[bytesPerSample];
      for(int i=0;i<size;i += bytesPerSample){
          int64_t value = (int64_t) (sin(x)*factor);
          fillData(data,bytesPerSample,value);
          for(int b=0;b<bytesPerSample;b++){
            res[i+b] = data[b];
          }   
          x+= step;
      }   
      return res;
    }

    void fillData(char* res,int b, int64_t value){
      switch(b){
         case 1: res[0] = (char) (value);
                 break;
         case 2: { int16_t v = (int16_t) value;
                   res[0] = (char)(v & 0xFF);
                   res[1] = (char) ((v & 0xFF00) >> 8);
                 }
                   break;
         case 3:
         case 4: { int32_t v2 = (int32_t) value;
                   res[0] = (char) (v2& 0xFF);
                   res[1] = (char) ((v2  & 0xFF00) >> 8);
                   res[2] = (char) ((v2 & 0xFF0000) >> 16);
                   if(b==4){
                      res[3] = (char) ((v2 &0xFF000000) >> 24);
                   }
                 }
                 break;
       }
   }


    char* createPause(int ms,int& size){
      int bytesPerSample = (sampleSize +7) / 8;
      int size1 = (ms*sampleRate)/  1000 ;
      size = bytesPerSample*size1;
      char* res = new char[size];
      memset(res,0,size);
      return res;
    }

    void writeHeader(ofstream& out){
       out << 'R' << 'I' << 'F' << 'F';
       int32_t l = 0; // placeholder for lenght
       out.write((char*) &l, 4);
       out << 'W' << 'A' << 'V' << 'E';
       out << 'f' << 'm' << 't' << ' ';
       int32_t hl = 16;
       out.write((char*) &hl, 4); // header length 
       int16_t format = 1;
       out.write((char*) &format, 2);;  // format PCM
       int16_t channels = 1;
       out.write((char*) &channels, 2);  // number of channels
       out.write((char*) & sampleRate, 4);
       int32_t byteRate = sampleRate * ((sampleSize+7)/8);     
       out.write((char*) &byteRate,4);
       int16_t blockAlign = ((sampleSize+7)/8);
       out.write((char*) &blockAlign, 2);
       out.write((char*) &sampleSize,2);
       out << 'd' << 'a' << 't' << 'a';
       int32_t ph=0;
       out.write((char*)&ph,4); // placeholder for data size
    } 

    void writeCode(ofstream& out, string& code, bool& lastSpace){
       for(size_t pos=0;pos<code.length();pos++){
          char c = code[pos];
          switch(c){
            case '.' : 
                  out.write(dit,ditSize);
                  out.write(pause,pauseSize);
                  break;
            case '-' : out.write(dah, dahSize);
                  out.write(pause,pauseSize);
                  break;
            case ' ' : if(!lastSpace){
                     for(int i=0;i<6;i++){
                       out.write(pause,pauseSize);
                     }
                  };
                  lastSpace = true;
                  break;
            case '#' : out.write(pause,pauseSize);
                  out.write(pause,pauseSize);
          }
       } 
       // mark end of character
       out.write(pause,pauseSize);
       out.write(pause,pauseSize);
    }
};


template<class T>
int morseWavVM1( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

   T* arg = (T*) args[0].addr;
   FText* fn = (FText*) args[1].addr;
   CcInt* speed1 = (CcInt*) args[2].addr;
   CcInt* freq1 = (CcInt*) args[3].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(    !arg->IsDefined() || !fn->IsDefined() 
       || !speed1->IsDefined() || !freq1->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   int sampleRate= 4000;
   int freq = freq1->GetValue();
   int speed = speed1->GetValue();
   if(freq<20 || speed < 10){
     res->Set(true,false);
     return 0;
   }
 

   if(freq>2000){
    sampleRate = 8000;
   }
   if(freq>4000){
     sampleRate = 16000;
   }
   if(freq > 8000){
      sampleRate = 44200;
   }
   createMorse<T> cm(arg,fn, speed, freq,  sampleRate, 16);
   res->Set(true, cm.createWav());
   return 0;
}


ValueMapping morseWavVM[] = {
  morseWavVM1<CcString>,
  morseWavVM1<FText>
};

int morseWavSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}



/*
4.37.5 Specification

*/
OperatorSpec morseWavSpec(
  "{string,text} x string  -> bool ",
  " morseWav(text, fileName, speed, frequency)",
  " Creates a wav file containing the  morse code of the text.",
  "query morseWav('test',test.wav,40,440)"
);


/*
4.37.6 Operator instance

*/
Operator morseWavOP(
  "morseWav",
  morseWavSpec.getStr(),
  2,
  morseWavVM,
  morseWavSelect,
  morseWavTM
);


/*
4.38 Operator computeRegEx

This operator computes a regular expression from a finite automaton even if the source was lost.
Note that the source for the automaton and the result msut not be the same, but both decribe the
same language.

*/
ListExpr computeRegExTM(ListExpr args){
  string err = "regex or regex2 expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr a = nl->First(args);
  if(RegExPattern::checkType(a) ||
     RegExPattern2::checkType(a)){
     return listutils::basicSymbol<FText>();
  }
  return listutils::typeError(err);
}


template<class T>
int computeRegExVM1( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

   T* arg = (T*) args[0].addr;
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   if(!arg->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   stringstream ss;
   if(arg->computeRegEx(ss)){
      res->Set(true,ss.str());
   } else {
       res->SetDefined(false);
   }
   return  0;
}

ValueMapping computeRegExVM[] = {
  computeRegExVM1<RegExPattern>,
  computeRegExVM1<RegExPattern2>
};

int computeRegExSelect(ListExpr args){
  return RegExPattern::checkType(nl->First(args))?0:1;
}



/*
4.37.5 Specification

*/
OperatorSpec computeRegExSpec(
  "{regex, regex2} -> text ",
  " computeRegEx(_)",
  " Computes a regular expression form a finite automaton",
  "query computeRegEx([const regex2 value 'a'])"
);


/*
4.37.6 Operator instance

*/
Operator computeRegExOP(
  "computeRegEx",
  computeRegExSpec.getStr(),
  2,
  computeRegExVM,
  computeRegExSelect,
  computeRegExTM
);


/*
4.38 letObject2

This operator stores an arbritrary object to the database.


4.38.1 Type Mapping

signature is: string x any -> bool

where any is not a stream.

*/
ListExpr letObject2TM(ListExpr args){

  string err = " {string,text} x ANY  expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  ListExpr name = nl->First(args);
  ListExpr value = nl->Second(args);

  if(!CcString::checkType(name) && !FText::checkType(name)){
    return listutils::typeError(err);
  }
  if(nl->HasLength(value,2) &&
     nl->IsEqual(nl->First(value), Stream<Tuple>::BasicType())){
    return listutils::typeError("second argument cannot be a stream");
  }
  return listutils::basicSymbol<CcBool>();
}


/*
4.38.2 ValueMapping

*/
template <class T>
int letObject2VM1( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  T* name = (T*) args[0].addr;

  // objectname must be defined
  if(!name->IsDefined()){
     res->Set(true,false);
     return 0;
  }
  string n = name->GetValue();
  if( n == "" ){
    res->Set(true, false);
    return 0;
  } 
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  if ( ctlg->IsSystemObject(n) ) {
    res->Set(true,false);
    return 0;
  }
  if( ctlg->IsObjectName(n )) {
    res->Set(true, false);
    return 0;
  } 
  if(!ctlg->IsValidIdentifier(n)){
    res->Set(true,false);
    return 0;
  }

  // InsertObject(Name, typename, resultType, value, defined)
  ListExpr resultType = qp->GetType(qp->GetSon(s,1));

   int algId = 0;
   int typeId = 0;
   string basicType;
   bool ok = ctlg->LookUpTypeExpr(resultType, basicType,algId,typeId);
   if(!ok){
       res->Set(true,false);
       cerr << "Problem in LookUp TypeExpr";
       return 0;
   }
   AlgebraManager* am = SecondoSystem::GetAlgebraManager();
   Word valueClone =  am->CloneObj(algId,typeId)(resultType,args[1]);
   bool success =  ctlg->InsertObject( n, "",resultType,valueClone,true);
   res->Set(true,success);
  return 0;
}


ValueMapping letObject2VM[] = {
  letObject2VM1<CcString>,
  letObject2VM1<FText>
};

int letObject2Select(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}



/*
4.37.5 Specification

*/
OperatorSpec letObject2Spec(
  "{string,text} x any -> text ",
  "letObject2(name, value)",
  "Stores an object to the database",
  "query letObject2('sum', 4 + 3)"
);


/*
4.37.6 Operator instance

*/
Operator letObject2OP(
  "letObject2",
  letObject2Spec.getStr(),
  2,
  letObject2VM,
  letObject2Select,
  letObject2TM
);



/*
4.38 Operator ~globalMemory~

4.38.1 Type Mapping
  
   -> int

*/

ListExpr globalMemoryTM(ListExpr args){

  if(nl->IsEmpty(args)){
    return listutils::basicSymbol<LongInt>();
  }
  return listutils::typeError("no argument expected");
}

/*
4.38.2 Value Mapping

*/

int globalMemoryVM( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  LongInt* res = (LongInt*) result.addr;
  int gm = qp->GetGlobalMemory();
  res->SetValue(gm);
  return 0;
}


/*
4.38.3 Specification

*/
OperatorSpec globalMemorySpec(
  " -> longint",
  " globalMemory() ",
  "Returns the size of memory available for operators",
  "query globalMemory()"
);


/*
4.38.4 Operator instance

*/

Operator globalMemoryOP
(
  "globalMemory",             //name
  globalMemorySpec.getStr(),           //specification
  globalMemoryVM,        //value mapping
  Operator::SimpleSelect,         //trivial selection function
  globalMemoryTM        //type mapping
);

/*
4.39 Operator ~fileExtension~

This operator returns the fileExtension of a certain path. 

4.39.1 Type Mapping

*/
ListExpr fileExtensionTM(ListExpr args){
  string err ="string or text [x bool] expected";
  if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
    return  listutils::typeError("wrong number of arguments");
  }
  if(nl->HasLength(args,2)){
    if(!CcBool::checkType(nl->Second(args))){
      return listutils::typeError(err);
    }
  }
  ListExpr arg = nl->First(args);
  if(CcString::checkType(arg) || FText::checkType(arg)){
     if(nl->HasLength(args,2)){
         return arg;
     } else {
        return nl->ThreeElemList(
                  nl->SymbolAtom(Symbols::APPEND()),
                  nl->OneElemList(nl->BoolAtom(false)),
                  arg);
     }
  }
  return listutils::typeError(err);
}

/*
4.39.2 Value Mapping

*/
template<class T>
int fileExtensionVM1( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

  T* arg = (T*) args[0].addr;
  result = qp->ResultStorage(s);
  T* res = (T*) result.addr;
  if(!arg->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  bool acceptOnlyDot = false;
  CcBool* b = (CcBool*) args[1].addr;
  if(b->IsDefined()){
     acceptOnlyDot = b->GetValue();
  }
  string v = arg->GetValue();
  string seps = "." ;
  seps += CFile::pathSepWin32;
  seps += CFile::pathSepUnix;
  size_t pos = v.find_last_of(seps);
  if(pos==string::npos){ 
    if(acceptOnlyDot){
       res->Set(true,"");   
    } else {
       res->Set(true,v);
    }
  } else {
    if(acceptOnlyDot){
      if(v[pos]!='.'){
        res->Set(true,"");
      } else {
       res->Set(true, v.substr(pos+1));
      }
    } else {
       res->Set(true, v.substr(pos+1));
    }
  }
  return 0;
}

/*
4.39.3 ValueMappung Array and Selection function

*/
ValueMapping fileExtensionVM[] = {
  fileExtensionVM1<CcString>,
  fileExtensionVM1<FText>
};

int fileExtensionSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

/*
4.39.4 Specification

*/

OperatorSpec fileExtensionSpec(
  "T [x bool] -> T for T in {string,text}",
  "fileExtension(_)",
  "Returns the file extension of a path name."
  "If the boolean  parameter is true, the extension of a file"
  " without a dot is empty otherwide the file name",
  "query fileExtension('Secondo.exe')"
);

/*
4.39.5 Operator instance

*/

Operator fileExtensionOP(
  "fileExtension",
  fileExtensionSpec.getStr(),
  2,
  fileExtensionVM,
  fileExtensionSelect,
  fileExtensionTM
);


/*
4.40 Operator maskBaskslask

*/
ListExpr maskBackslashTM(ListExpr args){

  string err = "string or text expected";
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(   !CcString::checkType(arg) 
     && !FText::checkType(arg)){
     return listutils::typeError(err);
  }
  return arg;
}


template<class T>
int maskBackslashVM1( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

   T* arg = (T*) args[0].addr;
   result = qp->ResultStorage(s);
   T* res = (T*) result.addr;
   if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string v = arg->GetValue();
   v = stringutils::replaceAll(v,"\\","\\\\");
   res->Set(true,v);
   return 0;
}
/*
4.40.3 ValueMappung Array and Selection function

*/
ValueMapping maskBackslashVM[] = {
  maskBackslashVM1<CcString>,
  maskBackslashVM1<FText>
};

int maskBackslashSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

/*
4.39.4 Specification

*/

OperatorSpec maskBackslashSpec(
  "T  -> T for T in {string,text}",
  "_ maskBackslash)",
  " Doubles all backslashes in the argument",
  "query 'abcd\\' maskBacks"
);

/*
4.39.5 Operator instance

*/

Operator maskBackslashOP(
  "maskBackslash",
  maskBackslashSpec.getStr(),
  2,
  maskBackslashVM,
  maskBackslashSelect,
  maskBackslashTM
);

/*
4.40 Operator ~messageTest~

This operator send a message about the number of transferred tuples of a stream to the 
user as a nested list. The operator has two arguments, the first argument is the stream,
the second one is after how many stream elements a message should be sent.

*/

ListExpr messageTestTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("expected 2 arguments");
  }
 
  ListExpr s = nl->First(args);
  ListExpr i = nl->Second(args);

  if(!nl->HasLength(s,2) ||
     !listutils::isSymbol(nl->First(s),Symbols::STREAM())){
    return listutils::typeError("first argument must be a stream");
  }
  if(!CcInt::checkType(i)){
    return listutils::typeError("first argument must be an int");
  }
  return s;
}

class messageTestLocalInfo{

  public:

    messageTestLocalInfo(Word& _stream, Word& c): 
       stream(_stream), counter(0), number(0){
       CcInt* i = (CcInt*) c.addr;
       if(i->IsDefined()){
          int v = i->GetValue();
          number=v>0?v:0;
       } 
       qp->Open(stream.addr);
       msg = MessageCenter::GetInstance();
    }

    ~messageTestLocalInfo(){
       qp->Close(stream.addr);
     }

    Word next(){
       Word res;
       qp->Request(stream.addr,res);
       if(!qp->Received(stream.addr)){
         return SetWord((void*)0);
       }
       counter++;
       if(number>0 && counter%number==0){
          ListExpr list = nl->TwoElemList(
                             nl->SymbolAtom("messageTest"),
                             nl->IntAtom(counter));
          msg->Send(nl,list);
          msg->Flush();
       }
       return res;
    }

  private:
    Word stream;
    int counter;
    int number;
    MessageCenter* msg;
};


int messageTestVM( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

  messageTestLocalInfo* li = (messageTestLocalInfo*) local.addr;
  switch(message){
    case OPEN: 
            cout << "OPEN" << endl;
            if(li) {
               delete li;
            }
            local.addr=new messageTestLocalInfo(args[0],args[1]);
            return 0;
    case REQUEST:
            result.addr = li?li->next().addr:0;
            return result.addr?YIELD:CANCEL;
    case CLOSE:
           if(li) {
             delete li;
             local.addr=0;
           }
           return 0;
    default: assert(false);
             return 0;
  }
}

OperatorSpec messageTestSpec(
  "stream x int -> stream",
  "_ messageTest[_])",
  " Sends a message to the user after processing x elements, where x"
  " is specified by the second argument.",
  "query intstream(1,1000) messageTest[50] count"
);


Operator messageTestOP(
  "messageTest",
  messageTestSpec.getStr(),
  messageTestVM,
  Operator::SimpleSelect,
  messageTestTM
);


/*
4.40 simpleMessage
 
*/
ListExpr simpleMessageTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one argument required");
   }
   ListExpr arg = nl->First(args);
   if(!Attribute::checkType(arg)){
     return listutils::typeError("Argument not in kind DATA");
   }
   return arg;
}

template<class T>
int simpleMessageVMT( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

   MessageCenter* msg = MessageCenter::GetInstance();
   stringstream ss;
   Attribute* arg = (Attribute*) args[0].addr;
   if(arg->IsDefined()){
        ss << ((T*)arg)->GetValue();
   } else {
        ss << "UNDEFINED" ;
   }
   result = qp->ResultStorage(s);
   Attribute* res = (Attribute*) result.addr;
   res->CopyFrom(arg);
   ListExpr se = nl->TwoElemList(nl->SymbolAtom("simple2"), 
                                 nl->TextAtom(ss.str()));
   msg->Send(nl,se);
   msg->Flush();
   nl->Destroy(se);
   return 0;
}

int simpleMessageVMA( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

   MessageCenter* msg = MessageCenter::GetInstance();
   stringstream ss;
   Attribute* arg = (Attribute*) args[0].addr;
   arg->Print(ss);
   result = qp->ResultStorage(s);
   Attribute* res = (Attribute*) result.addr;
   res->CopyFrom(arg);
   ListExpr se = nl->TwoElemList(nl->SymbolAtom("simple2"), 
                                 nl->TextAtom(ss.str()));
   msg->Send(nl,se);
   msg->Flush();
   nl->Destroy(se);
   return 0;
}

int simpleMessageSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(CcString::checkType(a1)) return 0;
  if(FText::checkType(a1)) return 1;
  return 2;
}

ValueMapping simpleMessageVM [] = {
   simpleMessageVMT<CcString>,
   simpleMessageVMT<FText>,
   simpleMessageVMA
};




OperatorSpec simpleMessageSpec(
  "X -> X , where X in DATA",
  "simpleMessage(_))",
  " Sends a message to the user."
  " The message is a string representation of the argument.",
  "query plt feed replaceattr[Plz : simpleMessage(.Plz)] count"
);


Operator simpleMessageOP(
  "simpleMessage",
  simpleMessageSpec.getStr(),
  3,
  simpleMessageVM,
  simpleMessageSelect,
  simpleMessageTM
);


/*
4.41 Operator ~errorMessage~

This operator decodes error codes used internally in Secondo
into the human redable message.

4.41.1 Type Mapping


The operator gets an int and returns a text.

*/
ListExpr errorMessageTM(ListExpr args){
  string err = " expected int";
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<FText>();
}

/*
4.41.2 Value Mapping

*/

int errorMessageVM( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

   CcInt* arg = (CcInt*) args[0].addr;
   result = qp->ResultStorage(s);
   FText* res = (FText*) result.addr;
   if(!arg->IsDefined()){
      res->SetDefined(false);
   } else {
      res->Set(true, SecondoInterface::GetErrorMessage(arg->GetValue()));
   }
   return 0;
}

/*
4.41.3 Specification

*/
OperatorSpec errorMessageSpec(
  " int -> text",
  " errorMessage(_)",
  " transfers an error code into an error message",
  "query errorMessage(0)"
);


Operator errorMessageOP(
  "errorMessage",
  errorMessageSpec.getStr(),
  errorMessageVM,
  Operator::SimpleSelect,
  errorMessageTM
);


/*
4.42 Conversion between filepath and ftext

*/
ListExpr text2filepathTM(ListExpr args){
  string err = "text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  } 
  if(!FText::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom("filepath");
}

ListExpr filepath2textTM(ListExpr args){
  string err = "text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  } 
  if(!listutils::isSymbol(nl->First(args),"filepath")){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<FText>();
}


int filepath2textVM( Word* args, Word& result, int message,
                 Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  FText* arg = (FText*) args[0].addr;
  res->CopyFrom(arg);
  return 0;
}

OperatorSpec text2filepathSpec(
  " text -> filepath",
  " text2filepath(_)",
  " converts a text into a filepath",
  " query filepath2text(text2filepath('C/home'))"
);

OperatorSpec filepath2textSpec(
  " filepath -> text",
  " filepath2text(_)",
  " converts a filepath into a text",
  " query filepath2text(text2filepath('C/home'))"
);



Operator text2filepathOP(
  "text2filepath",
  text2filepathSpec.getStr(),
  filepath2textVM,
  Operator::SimpleSelect,
  text2filepathTM
);


Operator filepath2textOP(
  "filepath2text",
  filepath2textSpec.getStr(),
  filepath2textVM,
  Operator::SimpleSelect,
  filepath2textTM
);



/*
4.43 Operator ~secondoHome~

4.43.1 Type Mapping

*/
ListExpr secondoHomeTM(ListExpr args){

 if(!nl->IsEmpty(args)){
   return listutils::typeError("no arguments expected");
 }
 return listutils::basicSymbol<FText>();
}

/*
4.43.2 Value Mapping

*/
int secondoHomeVM( Word* args, Word& result, int message,
                 Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  res->Set(true, SmiEnvironment::GetSecondoHome());
  return 0;
}

/*
1.43.3 Specification

*/
OperatorSpec secondoHomeSpec(
  " -> text",
  " secondoHome()",
  " Return the home of the databases.",
  " query secondoHome()"
);

/*
1.43.4 Operator Instance

*/
Operator secondoHomeOP(
  "secondoHome",
  secondoHomeSpec.getStr(),
  secondoHomeVM,
  Operator::SimpleSelect,
  secondoHomeTM
);

/*
4.44 Operator ~query2list~

4.44.1 Type Mapping is TypeMap_text__text

4.44.2 Value Mapping

*/

int
query2ListVM( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecParser mySecParser;
  string querystringParsed = "";

  result = qp->ResultStorage( s );
  FText* res = (FText*) result.addr;
  if ( ((FText*)args[0].addr)->IsDefined() )
  {
    string querystring = ((FText*)args[0].addr)->GetValue();
    int returnValue = 0;
    returnValue = mySecParser.Text2List(querystring, querystringParsed);
    if(returnValue != 0)
    {
      res->Set(true, "INVALID QUERY " + SecParser::error);
    }
    else
    {
      res->Set(true, querystringParsed);
    }
  } else {
     res->SetDefined(false);
  }
  return (0);
}

/*
4.44.3 Specification

*/

OperatorSpec query2ListSpec(
  " text -> text",
  " query2list ( _ )",
  " Converts query to NL.",
  " query query2list('query 3 + 4')"
);

/*
4.44.4 Operator Instance

*/
Operator query2ListOP(
  "query2list",
  query2ListSpec.getStr(),
  query2ListVM,
  Operator::SimpleSelect,
  TypeMap_text__text
);


/*
4.45 Operator ~substrw~

This operators takes two positions within a text and returns the substring
between these positions. The given range is extended to the beginning 
of the word at the start and the end of a word at the end using a quite simple
stategy for recognizing a word. If the range is outside the text, it is changed
to the text's borders.

*/

ListExpr substrwTM(ListExpr args){

  string err = "{string.text} x int x int expected";
  if(!nl->HasLength(args,3)){
   return listutils::typeError(err);
  }
  if(  !CcString::checkType(nl->First(args)) 
     &&!FText::checkType(nl->First(args))){
   return listutils::typeError(err);
  }
  if(  !CcInt::checkType(nl->Second(args))
     ||!CcInt::checkType(nl->Third(args))){
   return listutils::typeError(err);
  }
  return nl->First(args);
}

template<class T>
int substrwVMT( Word* args, Word& result, int message, 
                  Word& local, Supplier s ) {

   T* arg = (T*) args[0].addr;
   CcInt* S = (CcInt*) args[1].addr;
   CcInt* E = (CcInt*) args[2].addr;
   result = qp->ResultStorage(s);
   T* res = (T*) result.addr;
   if(!arg->IsDefined() || !S->IsDefined() || !E->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   string value = arg->GetValue();
   int start = S->GetValue();
   int end = E->GetValue();
   if(start>=end){
     res->Set(true,"");
     return 0;
   }
   if(start<0){
     start = 0;
   }
   if(!stringutils::isWordSpace(value[start])){
      while((start>0) && !stringutils::isWordSpace(value[start-1])){
         start--;
      }
   }
   if((size_t)end>=value.length()){
      end = value.length()-1;
   }
   if(!stringutils::isWordSpace(value[end])){
       while( ((size_t)end<value.length()-1) 
          && !stringutils::isWordSpace(value[end+1]) ){
           end++;
      }          
   }
   res->Set(true, value.substr(start, (end-start)+1));
   return 0;
}

ValueMapping substrwVM[] = {
    substrwVMT<CcString>,
    substrwVMT<FText>
};

int substrwSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec substrwSpec(
  " T x int x int -> T, T in {text, string}",
  " text substrw[pos1,pos2]",
  "Returns a substring of a text. the given range is extended "
  "to word breaks.",
  " query 'hello world, here i am' substrw[7,12] "
);

Operator substrwOP(
  "substrw",
  substrwSpec.getStr(),
  2,
  substrwVM,
  substrwSelect,
  substrwTM
);


/*
4.46 Operator ~getConfig~

*/

ListExpr getConfigTM(ListExpr args){
  if(!nl->IsEmpty(args)){
     return listutils::typeError("no arguments expected");
  }
  return listutils::basicSymbol<FText>();
}


int getConfigVM( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  res->Set(true, SmiEnvironment::ConfigFile());
  return 0;
}

OperatorSpec getConfigSpec(
  " -> text",
  " getConfig() ",
  "Returns the configuration file used by the current Secondo instance",
  "query getConfig() "
);

Operator getConfigOP(
  "getConfig",
  getConfigSpec.getStr(),
  getConfigVM,
  Operator::SimpleSelect,
  getConfigTM
);

/*
4.47 Operator ~getParam~

*/
ListExpr getParamTM(ListExpr args){

  if(!nl->HasLength(args,3) && !nl->HasLength(args,4)){
    return listutils::typeError("3 or 4 arguments expected");
  }
  if(   !CcString::checkType(nl->First(args)) 
     && !FText::checkType(nl->First(args))){
    return listutils::typeError("first arg not of type string or text");
  } 
  if(   !CcString::checkType(nl->Second(args)) 
     && !FText::checkType(nl->Second(args))){
    return listutils::typeError("second arg not of type string or text");
  } 
  if(   !CcString::checkType(nl->Third(args)) 
     && !FText::checkType(nl->Third(args))){
    return listutils::typeError("third arg not of type string or text");
  } 

  if(nl->HasLength(args,4)){
     if(   !CcString::checkType(nl->First(args)) 
         && !FText::checkType(nl->Second(args))){
        return listutils::typeError("first arg not of type string or text");
     } 
     return listutils::basicSymbol<FText>();
  }
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList( nl->TextAtom( SmiEnvironment::ConfigFile())),
           listutils::basicSymbol<FText>());
}

template<class A1, class A2, class A3, class A4>
int getParamVMT( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  A1* a1 = (A1*) args[0].addr;
  A2* a2 = (A2*) args[1].addr;
  A3* a3 = (A3*) args[2].addr;
  A4* a4 = (A4*) args[3].addr;

  if(   !a1->IsDefined() || !a2->IsDefined() || !a3->IsDefined()
     || !a4->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  res->Set(true, SmiProfile::GetParameter(
                    a1->GetValue(),
                    a2->GetValue(),
                    a3->GetValue(),
                    a4->GetValue()));
  return 0;
}

ValueMapping getParamVM[] = {
  getParamVMT<CcString, CcString, CcString, CcString>,
  getParamVMT<CcString, CcString, CcString, FText>,
  getParamVMT<CcString, CcString, FText,    CcString>,
  getParamVMT<CcString, CcString, FText,    FText>,
  getParamVMT<CcString, FText,    CcString, CcString>,
  getParamVMT<CcString, FText,    CcString, FText>,
  getParamVMT<CcString, FText,    FText,    CcString>,
  getParamVMT<CcString, FText,    FText,    FText>,
  getParamVMT<FText   , CcString, CcString, CcString>,
  getParamVMT<FText   , CcString, CcString, FText>,
  getParamVMT<FText   , CcString, FText,    CcString>,
  getParamVMT<FText   , CcString, FText,    FText>,
  getParamVMT<FText   , FText,    CcString, CcString>,
  getParamVMT<FText   , FText,    CcString, FText>,
  getParamVMT<FText   , FText,    FText,    CcString>,
  getParamVMT<FText   , FText,    FText,    FText>
};

int getParamSelect(ListExpr args){
  int n1 = CcString::checkType(nl->First(args))?0:8;
  int n2 = CcString::checkType(nl->Second(args))?0:4;
  int n3 = CcString::checkType(nl->Third(args))?0:2;
  int n4 = 0;
  if(nl->HasLength(args,3)){
     n4 = 1;
  } else {
    n4 = CcString::checkType(nl->Fourth(args))?0:1;
  }
  return n1 + n2 + n3 + n4;
}

OperatorSpec getParamSpec(
  "{string,text} x {string,text} x {string, text} [ x {string,text} ] -> text",
  "getParam(section, key, default [, file ] ) ",
  "Retrieves a value from a secondo configuration file. "
  "If the file argument is omitted, the currently used conifigration file "
  "will be used. ",
  "query getParam('Environment', 'AlgebraLevel', 'Descriptive' ) "
);

Operator getParamOP(
  "getParam",
  getParamSpec.getStr(),
  16,
  getParamVM,
  getParamSelect,
  getParamTM
);

/*
Operator saveto

*/
ListExpr savetoTM(ListExpr args){
   string err = "{string,text} x {string,text} expected";
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }
   if(   !CcString::checkType(nl->First(args)) 
      && !FText::checkType(nl->First(args))
      && !Stream<CcString>::checkType(nl->First(args))
      && !Stream<FText>::checkType(nl->First(args))){
     return listutils::typeError(err);
   }
   if(!CcString::checkType(nl->Second(args)) 
      && !FText::checkType(nl->Second(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<CcBool>();
}


template<class S, class F>
int saveto_sVMT( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  F* filename = (F*) args[1].addr;
  if(!filename->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
 string fn = filename->GetValue();
  try{
     ofstream out(fn.c_str(), ios::out| ios::trunc);
     if(!out){
        res->Set(false,false);
     }
     Stream<S> stream(args[0]);
     stream.open();
     S* elem;
     while( (elem=stream.request())!=0){
         if(elem->IsDefined()){
            out << elem->GetValue() << endl;
         }
         elem ->DeleteIfAllowed();
     }
     stream.close(); 
     out.close();
     res->Set(true,true);
  } catch(...){
     res->Set(true,false);
  }
  return 0;
}


template<class S, class F>
int savetoVMT( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  S* source = (S*) args[0].addr;
  F* filename = (F*) args[1].addr;
  if(!source->IsDefined() || !filename->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  string fn = filename->GetValue();
  try{
     ofstream out(fn.c_str(), ios::out| ios::trunc);
     if(!out){
        res->Set(false,false);
     }
     out << source->GetValue();
     out.close();
     res->Set(true,true);
  } catch(...){
     res->Set(true,false);
  }
  return 0;
}

OperatorSpec savetoSpec(
  "{string,text} x {string,text} -> bool",
  "_ saveto _",
  "saves the content of a text or string into a file",
  "query 'hello world' saveto 'hello.txt'"
);

ValueMapping savetoVM[] = {
   savetoVMT<CcString,CcString>,
   savetoVMT<CcString,FText>,
   savetoVMT<FText,CcString>,
   savetoVMT<FText,FText>,
   saveto_sVMT<CcString,CcString>,
   saveto_sVMT<CcString,FText>,
   saveto_sVMT<FText,CcString>,
   saveto_sVMT<FText,FText>
};

int savetoSelect(ListExpr args){
   int n1=0;
   ListExpr a1 = nl->First(args);
   if(CcString::checkType(a1)){
     n1 = 0;
   }
   if(FText::checkType(a1)){
     n1 = 2;
   }
   if(Stream<CcString>::checkType(a1)){
     n1 = 4;
   }
   if(Stream<FText>::checkType(a1)){
     n1 = 6;
   }
   int n2 = CcString::checkType(nl->Second(args))?0:1;
   return n1+n2;
}

Operator savetoOp(
  "saveto",
  savetoSpec.getStr(),
  8,
  savetoVM,
  savetoSelect,
  savetoTM
);


/*
Operator ~executeScript~

*/

ListExpr executeScriptTM(ListExpr args){
  string err =" {string,text} x bool x bool expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err+" (wrong number of args)");
  }
  if(!CcString::checkType(nl->First(args))
     && !FText::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcBool::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  if(!CcBool::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }

  return listutils::basicSymbol<CcBool>();

}


class CommandExecuter{
  public:
     CommandExecuter(): qp(nl,am) {}


     int execute( const string& cmd, ListExpr& outList, 
                  string& errorMessage){

          SecParser sp;
          string listCommand;
          if(sp.Text2List(cmd, listCommand)!=0){
             cout << "parse not able to parse the command" << endl;
             return ERR_SYNTAX_ERROR;
          }
          ListExpr cmdList;
          if(!nl->ReadFromString(listCommand, cmdList)){
              cerr << "SecondoParser produced an invalid nested list" << endl;
              return ERR_SYNTAX_ERROR;
          }
          return execute(cmdList, outList, errorMessage);
     }


     int execute(const ListExpr cmd, ListExpr& outlist, 
                 string& errorMessage){
         
         if(nl->IsEmpty(cmd)){
            return 0;  // nop
         }
         if(nl->AtomType(nl->First(cmd))!=SymbolType){
            // first element of a command has to be an symbol
            return ERR_SYNTAX_ERROR;
         }
         string c = nl->SymbolValue(nl->First(cmd));
         if(c=="type") return cmdType(cmd, outlist, errorMessage);
         if(c=="delete") return deleteCmd(cmd, outlist, errorMessage);
         if(c=="create") return createCmd(cmd, outlist, errorMessage); 
         if(c=="update") return updateCmd(cmd, outlist, errorMessage);
         if(c=="let") return letCmd(cmd, outlist, errorMessage);
         if(c=="kill") return killCmd(cmd, outlist, errorMessage);
         if(c=="query") return queryCmd(cmd, outlist, errorMessage);
         if(c=="if") return ifCmd(cmd, outlist, errorMessage);
         if(c=="while") return whileCmd(cmd, outlist, errorMessage);
         if(c=="beginseq") return seqCmd(cmd,false, outlist, errorMessage);
         if(c=="beginseq2") return seqCmd(cmd,true, outlist, errorMessage);
         if(c=="list") return listCmd(cmd, outlist, errorMessage);
         if(c=="open") return openCmd(cmd, outlist, errorMessage);
         if(c=="close") return closeCmd(cmd, outlist, errorMessage);
         if(c=="save") return saveCmd(cmd, outlist, errorMessage);
         if(c=="restore") return restoreCmd(cmd, outlist, errorMessage);
         if(nl->HasLength(cmd,2) 
            && listutils::isSymbol(nl->Second(cmd),"transaction")){
             return transactionCmd(cmd, outlist, errorMessage);
         }  
         
         return ERR_CMD_NOT_RECOGNIZED; 
     }


     private:
         QueryProcessor qp;
       
        int cmdType(ListExpr cmd, ListExpr& outList, string& errorMessage){
            // creating types not supported yet
            outList = nl->TheEmptyList();
            errorMessage = "creation of type not supported";
            return -1;
        }
 
        int deleteCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
           // delete <obj>
           // delete database <db>
           // delete type <typename>
           // currently only delting of objects is supported
           outList = nl->TheEmptyList();
           if(!nl->HasLength(cmd,2)){
              errorMessage="deleting supported for objects only";
              return -1;
           } 
           if(nl->AtomType(nl->Second(cmd))!=SymbolType){
              return ERR_SYNTAX_ERROR;
           }
           SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
           bool ok = ctlg->DeleteObject(nl->SymbolValue(nl->Second(cmd)));
           ctlg->CleanUp(true);
           return ok?0:ERR_IDENT_UNKNOWN_OBJ;
        } 

        int createCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
           // create <id> : <type expr>
           // create database <dbname>
           // creation of databases not supported
           outList = nl->TheEmptyList();
           if(!nl->HasLength(cmd,4)){
                errorMessage="create supported for objects only";
               return -1;
           }
           if(nl->AtomType(nl->Second(cmd))!=SymbolType){
               errorMessage = "invalid object identifier";
               return ERR_NO_OBJ_CREATED;
           }
           SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
           bool ok = ctlg->CreateObject(nl->SymbolValue(nl->Second(cmd)),
                              "",
                              nl->Fourth(cmd),
                               0);           
           if(!ok) errorMessage="object already exists";
           return ok?0:ERR_OBJ_NAME_DOUBLY_DEFINED;
        } 

        int updateCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
          // update <id> := <expr>
          outList = nl->TheEmptyList();
          if(!nl->HasLength(cmd,4)){
             return ERR_SYNTAX_ERROR;
          }
          if(  (nl->AtomType(nl->Second(cmd))!=SymbolType)
            || (nl->AtomType(nl->Third(cmd))!=SymbolType)){
             return ERR_SYNTAX_ERROR;
          }
          string objName = nl->SymbolValue(nl->Second(cmd));
          SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
          if(!ctlg->IsObjectName(objName)){
            return ERR_IDENT_UNKNOWN_OBJ;
          } 
          bool correct = false;
          bool evaluable = false;
          bool defined = false;
          bool isFunction = false;
          Word result = SetWord( Address(0) );
          ListExpr resultType;
          OpTree tree = 0; 
          qp.Construct( nl->Fourth( cmd ), correct, evaluable, defined,
                         isFunction, tree, resultType );
          if(!evaluable){
            return ERR_EXPR_NOT_EVALUABLE;
          }
          // check for equal typoe of db-object and expr type
          ListExpr dbtype = ctlg->GetObjectTypeExpr(objName);
          if(!nl->Equal(dbtype, resultType)){
              return ERR_EXPR_TYPE_NEQ_OBJ_TYPE;
          }  
          qp.EvalS(tree, result,1);
          qp.Destroy(tree,false);
          // put result into catalog
          ctlg->UpdateObject(objName, result);
          ctlg->CleanUp(true);
          return 0;
        } 


        int letCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
          outList = nl->TheEmptyList();
          // let <id> := <expr>
          if(!nl->HasLength(cmd,4)){
             return ERR_SYNTAX_ERROR;
          }
          if( (nl->AtomType(nl->Second(cmd))!=SymbolType)
            || (nl->AtomType(nl->Third(cmd))!=SymbolType)){
             return ERR_SYNTAX_ERROR;
          }
          string objName = nl->SymbolValue(nl->Second(cmd));
          SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
          if(ctlg->IsObjectName(objName)){
            return ERR_NAME_DOUBLY_DEFINED;
          } 
          bool correct = false;
          bool evaluable = false;
          bool defined = false;
          bool isFunction = false;
          Word result = SetWord( Address(0) );
          ListExpr resultType;
          OpTree tree = 0; 
          qp.Construct( nl->Fourth( cmd ), correct, evaluable, defined,
                         isFunction, tree, resultType );
          if(!evaluable ){
            return ERR_EXPR_NOT_EVALUABLE;
          }
          bool ok;
          if (!isFunction){
            qp.EvalS( tree, result, 1 );
            if( IsRootObject( tree ) && !IsConstantObject( tree ) ){
                ok = ctlg->CloneObject( objName, result );
                qp.Destroy( tree, true );
            } else {
              qp.Destroy( tree, false );
              ok = ctlg->InsertObject( objName, "",resultType, result,true);
            }
            tree = 0;
          } else  { // abstraction or function object
            ctlg->CreateObject(objName, "", resultType, 0);
            if ( nl->IsAtom( nl->Fourth(cmd) ) ) { // function object
               ListExpr functionList = ctlg->GetObjectValue(
                       nl->SymbolValue( nl->Fourth(cmd)) );
               ok = ctlg->UpdateObject( objName, SetWord( functionList ) );
            } else {
               ok = ctlg->UpdateObject( objName, SetWord( nl->Fourth(cmd) ) );
            }
            if( tree ) {
              qp.Destroy( tree, true );
              tree = 0;
            }
          }
          ctlg->CleanUp(false);
          return ok?0:ERR_NO_OBJ_CREATED;
        } 


        int killCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
            outList = nl->TheEmptyList();
            if(!nl->HasLength(cmd,2)){
               return ERR_SYNTAX_ERROR;
            }
            if(nl->AtomType(nl->Second(cmd))!=SymbolType){
                return ERR_SYNTAX_ERROR;
            }
            SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
            if(ctlg->KillObject(nl->SymbolValue(nl->Second(cmd)))){
              ctlg->CleanUp(true);
              return 0;
            } else {
              return ERR_IDENT_UNKNOWN_OBJ;
            }
        } 


        int queryCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
           outList = nl->TheEmptyList();
           if(!nl->HasLength(cmd,2)){
             return  ERR_SYNTAX_ERROR;
           }
           bool correct = false;
           bool evaluable = false;
           bool defined = false;
           bool isFunction = false;
           Word result = SetWord( Address(0) );
           ListExpr resultType;
           OpTree tree = 0; 
           qp.Construct( nl->Second( cmd ), correct, evaluable, defined,
                         isFunction, tree, resultType );
           if(!evaluable){
              return ERR_EXPR_NOT_EVALUABLE;
           }
           qp.EvalS(tree, result,1);
           SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
           outList = nl->TwoElemList( resultType,
                               ctlg->OutObject(resultType, result));
           qp.Destroy(tree,true);
           if(nl->HasLength(outList,2)){
             NestedList* oldnl =  DisplayFunction::nl;
             DisplayTTY::Set_NL(nl);
             DisplayTTY::GetInstance().DisplayResult(nl->First(outList), 
                                                     nl->Second(outList));
             DisplayTTY::Set_NL(oldnl);
           }  
           return 0; 
        } 


        int ifCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
           if(!nl->HasLength(cmd,5) && !nl->HasLength(cmd,7)){
             return ERR_SYNTAX_ERROR;
           }
           ListExpr cond = nl->Second(cmd);
           ListExpr cmd1 = nl->Fourth(cmd);
           bool hasElse = false;
           ListExpr cmd2 = nl->TheEmptyList();
           if(nl->HasLength(cmd,7)){
              hasElse = true;
              cmd2 = nl->Sixth(cmd);
           }
           // evaluate condition
            bool correct = false;
            bool evaluable = false;
            bool defined = false;
            bool isFunction = false;
            Word result = SetWord( Address(0) );
            ListExpr resultType;
            OpTree tree = 0;
            qp.Construct( cond, correct, evaluable, defined,
                          isFunction, tree, resultType );
            if(!evaluable){
              errorMessage="condition not evaluable";
              return ERR_EXPR_NOT_EVALUABLE;
            }
            if(!CcBool::checkType(resultType)){
              errorMessage =" condition is not a predicate";
              return ERR_UNKNOWN_RETURN_CODE;
            }
            qp.EvalS(tree, result,1);
            CcBool* r = (CcBool*) result.addr;
            bool c = r->IsDefined() && r->GetValue();
            qp.Destroy(tree,true);
            if(c){
               return execute(cmd1,outList, errorMessage);
            } else if(hasElse){
               return execute(cmd2,outList, errorMessage);
            }
            return 0;
        } 


        int whileCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
           if(!nl->HasLength(cmd,5)){
             return ERR_SYNTAX_ERROR;            
           }
           ListExpr cond = nl->Second(cmd);
           ListExpr wcmd = nl->Fourth(cmd);

           bool cont = true;
           int rc = 0;
           while(cont){
              // evaluate condition
              bool correct = false;
              bool evaluable = false;
              bool defined = false;
              bool isFunction = false;
              Word result = SetWord( Address(0) );
              ListExpr resultType;
              OpTree tree = 0;
              qp.Construct( cond, correct, evaluable, defined,
                            isFunction, tree, resultType );
              if(!evaluable){
                errorMessage="condition not evaluable";
                return ERR_EXPR_NOT_EVALUABLE;
              }
              if(!CcBool::checkType(resultType)){
                errorMessage =" condition is not a predicate";
                return ERR_UNKNOWN_RETURN_CODE;
              }
              qp.EvalS(tree, result,1);
              CcBool* r = (CcBool*) result.addr;
              bool c = r->IsDefined() && r->GetValue();
              qp.Destroy(tree,true);
              if(!c){
                cont = c;
              } else {
                cont = (rc = execute(wcmd,outList, errorMessage)) != 0;
              }
            }
            return rc;
        } 

        int seqCmd(ListExpr cmd, bool haltOnError, ListExpr& outList, 
                   string& errorMessage ){
          outList = nl->TheEmptyList();
          if(!nl->HasLength(cmd,3)){
             return ERR_SYNTAX_ERROR;            
          }
          ListExpr cmdList = nl->Second(cmd);
          int rc = 0;
          while(!nl->IsEmpty(cmdList)){
            errorMessage="";
            rc = execute(nl->First(cmdList), outList, errorMessage);
            if(haltOnError && (rc!=0)){
              return rc;
            }
            cmdList = nl->Rest(cmdList);
          }
          return rc;
        } 


        int listCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
          outList = nl->TheEmptyList();
          if(!nl->HasMinLength(cmd,2) ){
              return ERR_SYNTAX_ERROR;
          }
          if(nl->AtomType(nl->Second(cmd))!=SymbolType){
              return ERR_SYNTAX_ERROR;
          }
          string se = nl->SymbolValue(nl->Second(cmd));
          SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
          if(nl->HasLength(cmd,2)){
            if(se=="operators"){
               outList = nl->TwoElemList(
                             nl->SymbolAtom("inquiry"),
                             nl->TwoElemList(
                                  nl->SymbolAtom("operators"),   
                                  ctlg->ListOperators()));;
               return 0; 
            } 
            if(se=="algebras"){
             outList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList(
                                nl->SymbolAtom("algebras"),   
                                am->ListAlgebras()));;
               return 0; 
            } 
            if(se=="databases"){
               outList = SecondoSystem::GetInstance()->ListDatabaseNames();
               outList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList(
                                nl->SymbolAtom("databases"),   
                                outList));;


               return 0; 
            } 
            if(se=="types"){
               outList = ctlg->ListTypes();
               outList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList(
                                nl->SymbolAtom("types"),   
                                outList));;
               return 0; 
            } 
            if(se=="objects"){
               outList = ctlg->ListObjects();
               outList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList(
                                nl->SymbolAtom("objects"),   
                                outList));;
               return 0; 
            } 
            return ERR_SYNTAX_ERROR;
          
          } else if(nl->HasLength(cmd,3)){
             if(nl->AtomType(nl->Third(cmd))!=SymbolType){
                return ERR_SYNTAX_ERROR;
             }
             string th = nl->SymbolValue(nl->Third(cmd));

             if(se=="type" && th=="constructors"){
                outList = ctlg->ListTypeConstructors();
                outList = nl->TwoElemList(
                           nl->SymbolAtom("inquiry"),
                           nl->TwoElemList(
                                nl->SymbolAtom("constructors"),   
                                outList));;
                return 0;
             }else if (se=="algebra"){
                int algId = am->GetAlgebraId(th);
                if(algId==0){
                   errorMessage = "algebra " + th + " currently bot included"; 
                   return ERR_ALGEBRA_UNKNOWN;
                } 
                ListExpr constOp =  nl->TwoElemList( 
                                        ctlg->ListTypeConstructors( algId ),
                                        ctlg->ListOperators( algId ));
                outList = nl->TwoElemList( nl->SymbolAtom("inquiry"),
                             nl->TwoElemList( nl->SymbolAtom("algebra"),
                                              nl->TwoElemList( nl->Third(cmd),
                                                               constOp ))); 
                return 0;
             } else {
                return ERR_SYNTAX_ERROR;
             }
          } else {
              return ERR_SYNTAX_ERROR;
          }
        } 

        int openCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
          errorMessage = "open database not supported";
          return -1;
        } 

        int closeCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
          errorMessage = "close database not supported";
          return -1;
        } 

        int saveCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
          if(!nl->HasLength(cmd,4)){
             return ERR_SYNTAX_ERROR;
          }
          if(   (nl->AtomType(nl->Second(cmd))!=SymbolType)
             || (!listutils::isSymbol(nl->Third(cmd),"to"))
             || (    (nl->AtomType(nl->Fourth(cmd))!=SymbolType)
                  && (nl->AtomType(nl->Fourth(cmd))!=TextType))){
             return ERR_SYNTAX_ERROR;
          }
          string se = nl->SymbolValue(nl->Second(cmd));
          if(se=="database"){
             errorMessage = "saving databases not supported";
             return -1;
          }
          SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
          bool ok = ctlg->IsValidIdentifier(se,true);
          if(!ok){
             errorMessage = "object " + se 
                          + " already stored in databases or a "
                            "reserved word is used";
             return ERR_NAME_DOUBLY_DEFINED;
          }
          string fn = nl->AtomType(nl->Fourth(cmd))==SymbolType
                    ? nl->SymbolValue(nl->Fourth(cmd))
                    : nl->Text2String(nl->Fourth(cmd));
          return SecondoSystem::GetInstance()->SaveObject(
                                        se, fn );
        } 

        int restoreCmd(ListExpr cmd, ListExpr& outList, string& errorMessage){
          if(   nl->HasLength(cmd,5) 
             && listutils::isSymbol(nl->Second(cmd), "database")){
              errorMessage = "restoring databases not supported";
              return -1;
          } 
          if(    !nl->HasLength(cmd,4)
              || (nl->AtomType(nl->Second(cmd))!=SymbolType)
              || !listutils::isSymbol(nl->Third(cmd),"from")
              || (   (nl->AtomType(nl->Fourth(cmd))!=SymbolType)
                  && (nl->AtomType(nl->Fourth(cmd))!=TextType))){
             return ERR_SYNTAX_ERROR;
          }
          string se = nl->SymbolValue(nl->Second(cmd));
          string fn = nl->AtomType(nl->Fourth(cmd))==SymbolType
                    ? nl->SymbolValue(nl->Fourth(cmd))
                    : nl->Text2String(nl->Fourth(cmd));
          ListExpr errorInfo = listutils::emptyErrorInfo();
          return SecondoSystem::GetInstance()->RestoreObjectFromFile(
                                     se, fn, errorInfo);
        } 
        int transactionCmd(ListExpr cmd, ListExpr& outList, 
                           string& errorMessage){
          errorMessage = "tracactions are not supported";
          return -1;
        } 

};



class executeScriptInfo{
  public:
     executeScriptInfo(const string& _filename, bool _stopOnError, bool _isPD):
       filename(_filename), stopOnError(_stopOnError), isPD(_isPD) {}




     bool exec(){
        return exec(filename, stopOnError,isPD);
     }    



  private:
     string filename;
     bool stopOnError;
     bool isPD;
     bool quit;
     CommandExecuter qe;


     bool exec(const string& filename, const bool stopOnError, const bool isPD){
        ifstream in(filename.c_str());
        if(!in){
          return false;
        }
        quit = false;
        return execute(in, stopOnError, isPD);

     }


     bool execute(ifstream& in, bool stopOnError, bool isPD){
        bool errorFound = false;
        string cmd;
        while (!in.eof() && !quit && ( !stopOnError || !errorFound )) {
           if ( getCommand(in,isPD, cmd) ) {  
              try {
                 if( !processCommand(cmd)){
                   errorFound = true;
                 }   
              }   catch (SecondoException& e) {
                 cerr << "Exception caught: " << e.msg() << endl;
              }   
          } 
        }
        return !errorFound;
     }

     bool getCommand(ifstream& in, bool isPD, string& cmd){
         bool complete = false;
         bool first = true;
         string line = "";
         cmd = "";
         bool inPD = false;
         while (!complete && !in.eof() && !in.fail()) {
            line = "";
            getline( in, line );
            if ( line.length() > 0  || inPD) {
               bool comment = false;
               if(!isPD){
                 comment = line[0] == '#';
               } else {
                 if(!inPD){
                   if(line.length()>0){
                      comment = line[0] == '#';
                   } 
                   if(!comment) {
                     if(line.length()>1){
                        if((    line[0]=='/') 
                            && (line[1]=='/')){ // single line comment
                          comment = true;
                        } else if((line[0]=='/') 
                                   && (line[1]=='*')) { // big comment
                          comment = true;
                          inPD = true;
                        }
                    }
                 }
                } else {
                   comment = true;
                   if(line.length()>1){
                     if( (line[0]=='*') && (line[1]=='/')){
                        inPD = false;
                        line = line.substr(2);
                        stringutils::trim(line);
                        comment = line.empty();
                     }
                  }
                }
              }

              if ( !comment )    {
                 if ( line[line.length()-1] == ';' ) {
                   complete = true;
                   line.erase( line.length()-1 );
                 }
                 if ( first ) {              // Check for single line command
                    if ( !complete ) {
                      complete = isInternalCommand( line );
                    }
                    cmd = line + " ";
                    first = false;
                } else {
                    cmd = cmd + "\n" + line + " ";
                }
            }
         } else {                           // Empty line ends command
            complete = cmd.length() > 0;
            first = true;
         }
       }

       stringutils::trim(cmd);
       return (complete);
     }

     void showPrompt(bool first){
        if (first){
            cout << "Secondo =>";
        } else {
            cout << "Secondo ->";
        }
     }

     bool isInternalCommand(const string& cmd){
        string cmd2 = cmd;
        stringutils::toUpper(cmd2);
        stringutils::trim(cmd2);
        if(cmd2.empty()){
          return false;
        }
        return ( cmd2 == "?"     || cmd2 == "HELP"        ||
                 cmd2 == "Q"     || cmd2 == "QUIT"        ||
                 cmd2 == "DEBUG" || cmd2 == "SHOW"        || 
                 cmd2[0] == '@' );
     }

     bool processCommand( const string& cmd) {
        // first step, handle internal commands
        string cmd2 = cmd;
        stringutils::toUpper(cmd2);
        stringutils::trim(cmd2);      
        if(cmd2.empty()){
          return true;
        }
        if ( cmd2 == "?" || cmd2 == "HELP" ) {
           cout << "NO help available";
           return true;
        } 
        if ( stringutils::startsWith(cmd2, "DEBUG" )) {
           cout << "*** Debugging  level change not supported in operator " 
                << endl;
           return true;
        }
       if ( cmd2 == "Q" || cmd2 == "QUIT" ) {
           cout << "*** Thank you for using execScript operator!" 
                << endl << endl;
           quit = true;
           return true;
       }
       if ( cmd[0] == '@' ) {
           bool stopOnError = false;
           int start=1;
           bool pdfile=false;
           if(cmd2.length()>1 && cmd2[1] == '@'){
              start = 2;
              stopOnError = true;
           }
           if(cmd2.length()>1 && cmd2[1] == '%'){
              start = 2;
              pdfile = true;       
           }
           if(cmd2.length()>1 && cmd2[1] == '&'){
              start = 2;
              pdfile = true;       
              stopOnError = true;
           }
           return exec( cmd.substr( start, ( cmd.length() - start ) ),
                           stopOnError, pdfile );
       }
       if ( stringutils::startsWith(cmd2, "REPEAT" ) ){
           cout << "REPEAT not supported by execScript operator" << endl;
           return true;
       }
       // check for supported commands
       return callSecondo(cmd); // ususal secondo command
 }


  bool callSecondo(const string& cmd) {
       int errorCode = 0;
       ListExpr cmdList = nl->TheEmptyList();
       ListExpr outList = nl->TheEmptyList();
       string errorMessage = "";
       string errorText = ""; 


       cout << "command " << cmd << endl;

       if ( cmd[cmd.find_first_not_of(" \n\r\t\v\b\a\f")] == '(' ) {
          if ( nl->ReadFromString( cmd, cmdList ) ) {  
             errorCode = qe.execute(cmdList, outList,errorMessage); 
          }   else {   
             cmsg.error() << endl << "*** Error: list expression expected!"
                          << endl;
             cmsg.send();
          }   
       } else {
          errorCode = qe.execute(cmd, outList, errorMessage); 
       }

       nl->Destroy( outList );
       if ( errorCode != 0 ) {
          nl->Destroy( outList );
          if(errorMessage.empty()){
            errorMessage=SecondoInterface::GetErrorMessage(errorCode);
          }
          cout << "command failed with code " << errorCode << endl
               << "'" << errorMessage << "'" << endl;
          cout << endl << endl;
          return false;
       } else {
         cout << "success" << endl;
       }
       if ( cmdList != nl->TheEmptyList() ) {
          nl->Destroy( cmdList );
       }
       cout << endl << endl;
       return true;
  }
};



template<class F>
int executeScriptVMT( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {

  F* file = (F*) args[0].addr;
  CcBool* haltOnError = (CcBool*) args[1].addr;
  CcBool* isPD = (CcBool*) args[2].addr;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  if(!file->IsDefined() || !haltOnError->IsDefined() || !isPD->IsDefined()){
    res->SetDefined(false);
    return  0;
  }
  executeScriptInfo info (file->GetValue(), haltOnError->GetValue(), 
                          isPD->GetValue());
  
  res->Set(true, info.exec());
  return 0;

}

ValueMapping executeScriptVM[] = {
     executeScriptVMT<CcString>,
     executeScriptVMT<FText>
};

int executeScriptSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec executeScriptSpec(
   "{string,text} x bool x bool -> bool",
   "executeScript(filename, haltOnErrors, isPD)",
   "Executes a script.",
   "query executeScript('TheScript',TRUE, FALSE)"
);

Operator executeScriptOP(
   "executeScript",
   executeScriptSpec.getStr(),
   2,
   executeScriptVM,
   executeScriptSelect,
   executeScriptTM
);


/*
5.100 Operator ~like2regex~

Creates a string from a LIKE pattern which can be used
in pattern matching.
 

*/

ListExpr like2regexTM(ListExpr args){
  if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
    return listutils::typeError("three arguments expected");
  }
  if(!CcString::checkType(nl->First(args))
     &&!FText::checkType(nl->First(args))){
    return listutils::typeError("first arg not of type string or test");
  }
  if(nl->HasLength(args,2)){
    if(!CcString::checkType(nl->Second(args))
        &&!FText::checkType(nl->Second(args))){
       return listutils::typeError("Second arg not of type string or test");
    }
  }
  return listutils::basicSymbol<RegExPattern2>();
}

string rewrite_like(const string& pattern,
          const string& escapes){

   bool es[256];
   for(int i=0;i<245;i++){
     es[i] = false;
   } 
   for(size_t i=0;i<escapes.size();i++){
     es[(int)escapes[i]] = true;
   }

   stringstream ss;
   int state = 0;
   for(size_t i=0;i<pattern.size();i++){
      char c = pattern[i];
      switch(state){
         case 0 :   if(es[(int)c]){ // found escape
                      state = 1;
                    } else if(c=='['){
                        ss << c;
                        state=2;
                    } else if(c=='_'){
                        ss << ".";
                    } else if(c=='('){
                        ss << "\\(";
                    } else if(c==')'){
                        ss << "\\)";
                    } else if(c=='%'){
                        ss << ".*";
                    } else if(c=='.'){
                        ss << "\\.";
                    } else if(c=='*'){
                        ss << "\\*";
                    } else if(c=='+'){
                        ss << "\\+";
                    } else if(c=='?'){
                        ss << "\\?";
                    } else {
                        ss << c;
                    }
                    break;
          case 1 : ss << c;  // escaped char
                   state = 0;
                   break;

          case 2 :  ss << c; // within square brackets
                    if(c==']'){
                       state = 0;
                     }
      }
   }
    return ss.str();

}



template<class P, class E>
int like2regexVMT( Word* args, Word& result, int message, 
                   Word& local, Supplier s ) {

   result = qp->ResultStorage(s);
   RegExPattern2* res = (RegExPattern2*) result.addr;
   P* pattern = (P*) args[0].addr;
   if(!pattern->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string esc = "";
   if(qp->GetNoSons(s)==2){
     E* Esc = (E*) args[1].addr;
     if(!Esc->IsDefined()){
        res->SetDefined(false);
        return 0;
     }
     esc = Esc->GetValue();
   }
   string rep = rewrite_like(pattern->GetValue(), esc);
   res->constructFrom(rep);
   return 0;
}



ValueMapping like2regexVM[] = {
   like2regexVMT<CcString, CcString>,
   like2regexVMT<CcString, FText>,
   like2regexVMT<FText, CcString>,
   like2regexVMT<FText, FText>
};

int like2regexSelect(ListExpr args){
  int n1 = CcString::checkType(nl->First(args))?0:2;
  int n2 = 0;
  if(nl->HasLength(args,2)){
    n2 = CcString::checkType(nl->Second(args))?0:1;
  }
  return n1+n2;
}


OperatorSpec like2regexSpec(
  "{string,text} [x {string, text}] -> regex2",
  "like2regex(_,_)",
  "Converts a SQL LIKE pattern into an regular expression."
  "The first arguments is the pattern to converted, the second, optinal"
  "argument represents the ESCAPE part",
  "query like2regex('%erl') regexmatches 'Berlin'");

Operator like2regexOp(
  "like2regex",
  like2regexSpec.getStr(),
  4,
  like2regexVM,
  like2regexSelect,
  like2regexTM
);

/*

Operator dbobjects

*/
ListExpr dbobjectsTM(ListExpr args){
   ListExpr err = listutils::typeError();
   while(!nl->IsEmpty(args)){
     if(nl->Equal(err,nl->First(args))){
        return listutils::typeError("detected type error as an argument");
     }
     args = nl->Rest(args);
   }
   return nl->TwoElemList(
                listutils::basicSymbol<Stream<CcString> >(),
                listutils::basicSymbol<CcString>());
}

class dbobjectsInfo{
  public:
     dbobjectsInfo(Supplier s){
        qp->GetDBObjects((OpNode*)s, names);
        pos = 0;
     }
     CcString* next(){
        if(pos>=names.size()) return 0;
        pos++;
        return new CcString(true, names[pos-1]);
     }

  private:
     vector<string> names;
     size_t pos;

};


int dbobjectsVM( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {

  dbobjectsInfo* li = (dbobjectsInfo*) local.addr;
  switch(message){
     case OPEN: if(li){
                   delete li;
                }
                local.addr = new dbobjectsInfo(s);
                return 0;
     case REQUEST:
                result.addr = li?li->next():0;
                return result.addr?YIELD:CANCEL;
     case CLOSE :
                if(li){
                   delete li;
                   local.addr = 0;
                }
                return 0;
  }
  return -1;
}

OperatorSpec dbobjectsSpec(
   "ANY -> stream(string)",
   "dbobjects(_,_,...) ",
   "Returns the names of the subtree rootet by this operator.",
   "query dbobjects(1,ten) count"
);

Operator dbobjectsOp(
   "dbobjects",
   dbobjectsSpec.getStr(),
   dbobjectsVM,
   Operator::SimpleSelect,
   dbobjectsTM
);


/*
4.68 Operator ~configFile~

Returns the name of the configurationFile used by the smi environment

*/
ListExpr configFileTM(ListExpr args){
   if(!nl->IsEmpty(args)){
     return  listutils::typeError("no arguments expected");
   }
   return listutils::basicSymbol<FText>();
}

int configFileVM( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {

  string r = SmiEnvironment::ConfigFile();
  result = qp->ResultStorage(s);
  FText* res = (FText*)result.addr;
  res->Set(true,r);
  return 0;
}

OperatorSpec configFileSpec(
  "-> text",
  "configFile()",
  "Returns the configuration file used by the smi environment.",
  "query configFile()"
);

Operator configFileOp(
  "configFile",
   configFileSpec.getStr(),
   configFileVM,
   Operator::SimpleSelect,
   configFileTM
);


/*
Operator ~queryTime~

*/
ListExpr queryTimeTM(ListExpr args){
   if(!nl->IsEmpty(args)){
     return listutils::typeError("no arguments expected");
   }
   return listutils::basicSymbol<Instant>();
}

int queryTimeVM( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {

  result = qp->ResultStorage(s);
  datetime::DateTime* res = (datetime::DateTime*)result.addr;
  (*res) = qp->GetQueryTime(s);
  return 0;
}

OperatorSpec queryTimeSpec(
  "-> instant",
  "queryTime()",
  "Returns the time when constructing the operator tree.",
  "query queryTime()"
);

Operator queryTimeOp(
  "queryTime",
   queryTimeSpec.getStr(),
   queryTimeVM,
   Operator::SimpleSelect,
   queryTimeTM
);


/*
Operators finding positions within a string/text

*/
ListExpr findPosTM(ListExpr args){
   if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
     return listutils::typeError("two or three arguments expected");
   }
   if(   !CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
     return listutils::typeError("first argument not of type string or text");
   }
   if(   !CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
     return listutils::typeError("Second argument not of type string or text");
   }
   if(nl->HasLength(args,2)){
     return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                nl->OneElemList( nl->IntAtom(-1)),
                listutils::basicSymbol<CcInt>());
   } else {
      if(!CcInt::checkType(nl->Third(args))){
        return listutils::typeError("third argument not of type int");
      }
      return listutils::basicSymbol<CcInt>();
   }

   
   
}


template<class A1, class A2, class F>
int findPosVMT( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {
   A1* arg1 = (A1*) args[0].addr;
   A2* arg2 = (A2*) args[1].addr;
   CcInt* arg3 = (CcInt*) args[2].addr;
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   if(!arg1->IsDefined() || !arg2->IsDefined() || !arg3->IsDefined()){
     res->SetDefined(0);
     return 0;
   }
   size_t r = F::fun(arg1->GetValue(), arg2->GetValue(), arg3->GetValue());
   if(r==string::npos){
     res->SetDefined(0);
   } else {
     res->Set(true,r+1);
   }
   return 0;
}

int findPosSelect(ListExpr args){
  int n1 = CcString::checkType(nl->First(args))?0:2;
  int n2 = CcString::checkType(nl->Second(args))?0:1;
  return n1 + n2;
}


class FindFirst{
 public: 
  static size_t fun(const string& s1, const string& s2, int s){
     if(s<1) s = 1;
     return s1.find(s2,s-1);
  }
};

OperatorSpec findFirstSpec(
   "{string,text} x {string,text} [x int] -> int",
   "findFirst(_,_,_)",
   "Returns the positon of the first occurrence of the second argument within"
   " the first argument. If the optional third argument is present, the "
   "search starts at this position. " ,
   "query findFirst('abc','b')"
);

ValueMapping findFirstVM[] = {
   findPosVMT<CcString, CcString, FindFirst>, 
   findPosVMT<CcString, FText, FindFirst>, 
   findPosVMT<FText, CcString, FindFirst>, 
   findPosVMT<FText, FText, FindFirst> 
};

Operator findFirstOp(
  "findFirst",
  findFirstSpec.getStr(),
  4,
  findFirstVM,
  findPosSelect,
  findPosTM
);


class FindLast{
 public: 
  static size_t fun(const string& s1, const string& s2, size_t s){
     if(s<1) s = string::npos;
     return s1.rfind(s2,s-1);
  }
};

OperatorSpec findLastSpec(
   "{string,text} x {string,text} [x int] -> int",
   "findLast(_,_,_)",
   "Returns the positon of the last occurrence of the second argument within"
   " the first argument. If the optional third argument is present, the "
   "search starts at this position. " ,
   "query findLast('abcbcbc','b')"
);

ValueMapping findLastVM[] = {
   findPosVMT<CcString, CcString, FindLast>, 
   findPosVMT<CcString, FText, FindLast>, 
   findPosVMT<FText, CcString, FindLast>, 
   findPosVMT<FText, FText, FindLast> 
};

Operator findLastOp(
  "findLast",
  findLastSpec.getStr(),
  4,
  findLastVM,
  findPosSelect,
  findPosTM
);

class FindFirstOf{
 public: 
  static size_t fun(const string& s1, const string& s2, int s){
     if(s<1) s = 1;
     return s1.find_first_of(s2,s-1);
  }
};

OperatorSpec findFirstOfSpec(
   "{string,text} x {string,text} [x int] -> int",
   "findFirstOf(_,_,_)",
   "Returns the positon of the first occurrence some character in the second"
   " argument within the first argument. If the optional third argument is",
   " present, the search starts at this position. " ,
   "query findFirstOf('abc','bc')"
);

ValueMapping findFirstOfVM[] = {
   findPosVMT<CcString, CcString, FindFirstOf>, 
   findPosVMT<CcString, FText, FindFirstOf>, 
   findPosVMT<FText, CcString, FindFirstOf>, 
   findPosVMT<FText, FText, FindFirstOf> 
};

Operator findFirstOfOp(
  "findFirstOf",
  findFirstOfSpec.getStr(),
  4,
  findFirstOfVM,
  findPosSelect,
  findPosTM
);


class FindLastOf{
 public: 
  static size_t fun(const string& s1, const string& s2, size_t s){
     if(s<1) s = string::npos;
     return s1.find_last_of(s2,s-1);
  }
};

OperatorSpec findLastOfSpec(
   "{string,text} x {string,text} [x int] -> int",
   "findLastOf(_,_,_)",
   "Returns the positon of the last occurrence of some character "
   "in the second argument within"
   " the first argument. If the optional third argument is present, the "
   "search starts at this position. " ,
   "query findLastOf('abcbcbc','bc')"
);

ValueMapping findLastOfVM[] = {
   findPosVMT<CcString, CcString, FindLastOf>, 
   findPosVMT<CcString, FText, FindLastOf>, 
   findPosVMT<FText, CcString, FindLastOf>, 
   findPosVMT<FText, FText, FindLastOf> 
};

Operator findLastOfOp(
  "findLastOf",
  findLastOfSpec.getStr(),
  4,
  findLastOfVM,
  findPosSelect,
  findPosTM
);

class FindFirstNotOf{
 public: 
  static size_t fun(const string& s1, const string& s2, int s){
     if(s<1) s = 1;
     return s1.find_first_not_of(s2,s-1);
  }
};

OperatorSpec findFirstNotOfSpec(
   "{string,text} x {string,text} [x int] -> int",
   "findFirstNotOf(_,_,_)",
   "Returns the positon of the first occurrence of some character not "
   "in the second"
   " argument within the first argument. If the optional third argument is",
   " present, the search starts at this position. " ,
   "query findFirstNotOf('abc','bc')"
);

ValueMapping findFirstNotOfVM[] = {
   findPosVMT<CcString, CcString, FindFirstNotOf>, 
   findPosVMT<CcString, FText, FindFirstNotOf>, 
   findPosVMT<FText, CcString, FindFirstNotOf>, 
   findPosVMT<FText, FText, FindFirstNotOf> 
};

Operator findFirstNotOfOp(
  "findFirstNotOf",
  findFirstNotOfSpec.getStr(),
  4,
  findFirstNotOfVM,
  findPosSelect,
  findPosTM
);


class FindLastNotOf{
 public: 
  static size_t fun(const string& s1, const string& s2, size_t s){
     if(s<1) s = string::npos;
     return s1.find_last_not_of(s2,s-1);
  }
};

OperatorSpec findLastNotOfSpec(
   "{string,text} x {string,text} [x int] -> int",
   "findLastNotOf(_,_,_)",
   "Returns the positon of the last occurrence of some character "
   "not in the second argument within"
   " the first argument. If the optional third argument is present, the "
   "search starts at this position. " ,
   "query findLastNotOf('abcbcbc','bc')"
);

ValueMapping findLastNotOfVM[] = {
   findPosVMT<CcString, CcString, FindLastNotOf>, 
   findPosVMT<CcString, FText, FindLastNotOf>, 
   findPosVMT<FText, CcString, FindLastNotOf>, 
   findPosVMT<FText, FText, FindLastNotOf> 
};

Operator findLastNotOfOp(
  "findLastNotOf",
  findLastNotOfSpec.getStr(),
  4,
  findLastNotOfVM,
  findPosSelect,
  findPosTM
);


/*
Operator endl

*/
ListExpr endlTM(ListExpr args){
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no arguments expected");
  }
  return listutils::basicSymbol<CcString>();
}

int endlVM( Word* args, Word& result, int message, 
                 Word& local, Supplier s ) {

  result= qp->ResultStorage(s);
  CcString* res = (CcString*) result.addr;
  stringstream ss;
  ss << endl;
  res->Set(true, ss.str());
  return 0;
}

OperatorSpec endlSpec(
  " -> string",
  "endl() ",
  "returns the end of line symbol",
  "query endl()  "
);

Operator endlOp(
  "endl",
  endlSpec.getStr(),
  endlVM,
  Operator::SimpleSelect,
  endlTM
);



/*
5 Creating the algebra

*/

  class FTextAlgebra : public Algebra
  {
  public:
    FTextAlgebra() : Algebra()
    {
      if(traces)
        cout <<'\n'<<"Start FTextAlgebra() : Algebra()"<<'\n';
      AddTypeConstructor( &ftext );
      AddTypeConstructor( &svg );
      ftext.AssociateKind(Kind::DATA());
      svg.AssociateKind(Kind::DATA());
      ftext.AssociateKind(Kind::INDEXABLE());
      ftext.AssociateKind(Kind::CSVIMPORTABLE());
      ftext.AssociateKind(Kind::CSVEXPORTABLE());
      ftext.AssociateKind(Kind::DELIVERABLE());

      AddTypeConstructor(&regexPattern);
      regexPattern.AssociateKind(Kind::DATA());  
      
      AddTypeConstructor(&regexPattern2);
      regexPattern.AssociateKind(Kind::SIMPLE());  

      AddOperator( &contains );
      AddOperator( &length );
      AddOperator( &getkeywords );
      AddOperator( &getsentences );
      AddOperator( &diceCoeff);
      AddOperator( &ftextgetcatalog );
      AddOperator( &ftextsubstr );
      AddOperator( &ftextsubtext );
      AddOperator( &ftextfind );
      AddOperator( &ftextisempty );
      AddOperator( &ftexttrim );
      AddOperator( &ftextplus );
      AddOperator( &ftextless );
      AddOperator( &ftextlesseq );
      AddOperator( &ftexteq );
      AddOperator( &ftextbiggereq );
      AddOperator( &ftextbigger );
      AddOperator( &ftextneq );
      AddOperator( &ftextevaluate );
      AddOperator( &ftextreplace );
      AddOperator( &ftexttoupper );
      AddOperator( &ftexttolower );
      AddOperator( &ftexttostring );
      AddOperator( &ftexttotext );
      AddOperator( &isDBObject);
      AddOperator( &getTypeNL );
      getTypeNL.SetRequestsArguments();
      AddOperator( &getValueNL );
      AddOperator( &ftexttoobject );
      AddOperator( &chartext );
      AddOperator( &ftsendtextUDP );
      AddOperator( &ftreceivetextUDP );
      AddOperator( &ftreceivetextstreamUDP );
      AddOperator( &svg2text );
      AddOperator( &text2svg );
      AddOperator( &crypt);
      AddOperator( &checkpw);
      AddOperator( &md5);
      AddOperator( &blowfish_encode);
      AddOperator( &blowfish_decode);
      AddOperator( &ftextletObject);
      AddOperator( &ftextdeleteObject);
      AddOperator( &ftextcreateObject);
  //     AddOperator( &ftextderiveObject);
  //     AddOperator( &ftextupdateObject);
      AddOperator( &ftextgetObjectTypeNL);
      AddOperator( &ftextgetObjectValueNL);
      AddOperator( &getDatabaseName);
      AddOperator( &matchingOperatorNames);
      AddOperator( &matchingOperators);
      AddOperator( &sysgetMatchingOperators);
      AddOperator( &sysgetAlgebraName);
      AddOperator( &sysgetAlgebraId);
  //     AddOperator( &sysgetOperatorInfo);
  //     AddOperator( &sysgetTypeConstructorInfo);
  //     AddOperator( &sysgetNoAlgebras);
      AddOperator( &checkOperatorTypeMap);
      AddOperator( &checkOperatorTypeMap2);
      AddOperator( &strequal);
      AddOperator( &tokenize);
      AddOperator( &sendtextstreamTCP);
      AddOperator( &charToText);
      AddOperator( &attr2text);
      AddOperator( &isValidID);
      AddOperator( &trimAll);
      AddOperator( &trimAllUndef);
      AddOperator(&str2real);
      AddOperator(&cn2en);
      AddOperator(&str2int);
      AddOperator(&endsWith);
      AddOperator(&startsWith);
      AddOperator(&startsOp);
      AddOperator(&markText);
      AddOperator(&bashModifierOp);
      AddOperator(&getBashModifiersOp);
      AddOperator(&getQueryNL);
      AddOperator(&getOpTreeNL);
      AddOperator(&getOpName);
      AddOperator(&regexmatches);
      AddOperator(&startsReg);
      AddOperator(&findPattern);
      AddOperator(&createRegEx);
      AddOperator(&createRegEx2);
      AddOperator(&numOfFlobs);
      AddOperator(&flobSize);
      AddOperator(&flobMemSize);
      AddOperator(&sizeOf);
      AddOperator(&tmcheckOp);
      
      AddOperator(&pointerTest);
      
      AddOperator(&getObjectOP);
      AddOperator(&getObjectsOp);
      getObjectOP.SetUsesArgsInTypeMapping();     

      AddOperator(&flobInfoOP);
      AddOperator(&correctFileIdOP);
      AddOperator(&charCodesOP);
      AddOperator(&morseWavOP);
      AddOperator(&computeRegExOP);
      
      AddOperator(&letObject2OP);

      AddOperator(&globalMemoryOP);
      AddOperator(&fileExtensionOP);
      AddOperator(&maskBackslashOP);

      AddOperator(&messageTestOP);
      AddOperator(&errorMessageOP);
      AddOperator(&simpleMessageOP);

      AddOperator(&filepath2textOP);
      AddOperator(&text2filepathOP);
      
      AddOperator(&secondoHomeOP);

      AddOperator(&query2ListOP);
      AddOperator(&substrwOP);

      AddOperator(&getConfigOP);
      AddOperator(&getParamOP);
      AddOperator(&savetoOp);
      
      AddOperator(&executeScriptOP);
      AddOperator(&like2regexOp);

      AddOperator(&dbobjectsOp);

      AddOperator(&configFileOp);
      AddOperator(&queryTimeOp);

      AddOperator(&findFirstOp);
      AddOperator(&findLastOp);
      AddOperator(&findFirstOfOp);
      AddOperator(&findLastOfOp);
      AddOperator(&findFirstNotOfOp);
      AddOperator(&findLastNotOfOp);
      AddOperator(&endlOp);

#ifdef RECODE
      AddOperator(&recode);
#endif


      LOGMSG( "FText:Trace",
        cout <<"End FTextAlgebra() : Algebra()"<<'\n';
      )
    }

    ~FTextAlgebra() {};
  };

  } // end namespace ftext

  /*
  Type name for data type svg in Secondo

  */
  const string SVG::BasicType() {
    return "svg";
  }

  /*
  6 Initialization

  Each algebra module needs an initialization function. The algebra manager
  has a reference to this function if this algebra is included in the list
  of required algebras, thus forcing the linker to include this module.

  The algebra manager invokes this function to get a reference to the instance
  of the algebra class and to provide references to the global nested list
  container (used to store constructor, type, operator and object information)
  and to the query processor.

  */

  extern "C"
  Algebra*
  InitializeFTextAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef,
                          AlgebraManager* amRef )
  {
    if(traces)
      cout << '\n' <<"InitializeFTextAlgebra"<<'\n';
    ftext::FTextAlgebra* ptr = new ftext::FTextAlgebra();
    srand( (unsigned) time(0) ) ;
    ptr->Init(nl, qp, am);
    return ptr;
  }

