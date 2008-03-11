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

[1] FText Algebra

March - April 2003 Lothar Sowada

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

April 2006, M. Spiekermann. Output format of type text changed! 
Now it will be only a text atom instead of a one element list containing a text atom.  

The algebra ~FText~ provides the type constructor ~text~ and two operators:

(i) ~contains~, which search text or string in a text.

(ii) ~length~ which give back the length of a text.


March 2008, Christian D[ue]ntgen added operators ~getcatalog~, $+$, ~substr~,
~subtext~, ~isempty~, $<$, $<=$, $=$, $>=$, $>$, \#, ~find~, ~evaluate~.

1 Preliminaries

1.1 Includes

*/


#include <cstring>
#include <iostream>


#include "FTextAlgebra.h"
#include "StandardAttribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "NList.h"
#include "LogMsg.h"
#include "DiceCoeff.h"
#include "SecParser.h"
#include "StopWatch.h"

using namespace std;

//extern NestedList* nl;
//extern QueryProcessor* qp;
//extern AlgebraManager* am;

/*
2 Type Constructor ~text~

2.1 Data Structure - Class ~FText~

*/
size_t FText::HashValue() const
{
  if(traces)
    cout << '\n' << "Start HashValue" << '\n';

  if(!defined)
    return 0;

  unsigned long h = 0;
  const char* s = 0;
  theText.Get(0, &s);
  while(*s != 0)
  {
    h = 5 * h + *s;
    s++;
  }
  return size_t(h);
}

void FText::CopyFrom( const StandardAttribute* right ) 
{
  if(traces)
    cout << '\n' << "Start CopyFrom" << '\n';

  const FText* r = (const FText*)right;
  const char *s = 0;
  r->theText.Get(0, &s);
  Set( r->defined, s );
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

  const char *s1 = 0, *s2 = 0;
  f->theText.Get(0, &s1);
  this->theText.Get(0, &s2);

  return strcmp( s2, s1 );
}

ostream& FText::Print(ostream &os) const
{
  if(!IsDefined())
  {
    return (os << "TEXT: UNDEFINED");
  }
  const char* t = 0;
  theText.Get(0, &t);
  string s(t);
  size_t len = theText.Size();
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

  const char *a = 0, *b = 0;
  theText.Get(0, &a);
  ((const FText *)arg)->theText.Get(0, &b);

  if( strcmp( a, b ) == 0 )
    return 1;

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

  return 0;
}

/*

2.3 Functions for using ~text~ in tuple definitions

The following Functions must be defined if we want to use ~text~ as an attribute type in tuple definitions.

*/

Word
CreateFText( const ListExpr typeInfo )
{
  if(traces)
    cout << '\n' << "Start CreateFText" << '\n';
  return (SetWord(new FText( false )));
}

void
DeleteFText( const ListExpr typeInfo, Word& w )
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
bool
OpenFText( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
  FText *ft =
    (FText*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( ft );
  return true;
}

/*
2.9 ~Save~-function

*/
bool
SaveFText( SmiRecord& valueRecord,
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


void
CloseFText( const ListExpr typeInfo, Word& w )
{
  if(traces)
    cout << '\n' << "Start CloseFText" << '\n';
  delete (FText*) w.addr;
  w.addr = 0;
}

Word
CloneFText( const ListExpr typeInfo, const Word& w )
{
  if(traces)
    cout << '\n' << "Start CloneFText" << '\n';
  return SetWord( ((FText *)w.addr)->Clone() );
}

int
SizeOfFText()
{
  return sizeof( FText );
}

void*
CastFText( void* addr )
{
  if(traces)
    cout << '\n' << "Start CastFText" << '\n';
  return (new (addr) FText);
}


/*

2.4 ~In~ and ~Out~ Functions

*/

ListExpr
OutFText( ListExpr typeInfo, Word value )
{
  if(traces)
    cout << '\n' << "Start OutFText" << '\n';
  FText* pftext;
  pftext = (FText*)(value.addr);

  if(traces)
    cout <<"pftext->Get()='"<< pftext->Get() <<"'\n";
 
  ListExpr res;
  if(pftext->IsDefined()){ 
     res=nl->TextAtom(pftext->Get());
  } else {
     res = nl->SymbolAtom("undef");
  }
  //nl->AppendText( TextAtomVar, pftext->Get() );

  if(traces)
    cout <<"End OutFText" << '\n';
  return res;
}


Word
InFText( const ListExpr typeInfo, const ListExpr instance,
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
       && nl->SymbolValue( First ) == "undef" )
  {
    string buffer = "";
    FText* newftext = new FText( false, buffer.c_str() );
    correct = true;

    if(traces)
      cout << "End InFText with undef Text '"<<buffer<<"'\n";
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

ListExpr
FTextProperty()
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
        nl->StringAtom("text"),
        nl->StringAtom("<text>writtentext</text--->"),
        nl->StringAtom("<text>This is an example.</text--->")
      )
    )
  );
}


/*
2.6 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/

bool
CheckFText( ListExpr type, ListExpr& errorInfo )
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




/*

2.7 Creation of the type constructor instance

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


/*

3 Creating Operators

3.1 Type Mapping Functions

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol ~typeerror~.

*/

ListExpr
TypeMapTextTextBool( ListExpr args )
{
  if(traces)
  {
    cout <<'\n'<<"Start TypeMapTextTextBool"<<'\n';
    nl->WriteToFile("/dev/tty",args);
  }
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, typeName) && nl->IsEqual(arg2, typeName) )
    {
      return nl->SymbolAtom("bool");
    }
  }

  if(traces)
    cout <<"End TypeMapTextTextBool with typeerror"<<'\n';
  return nl->SymbolAtom("typeerror");
}


ListExpr
TypeMapTextStringBool( ListExpr args )
{
  if(traces)
    {
    cout <<'\n'<<"Start TypeMapTextStringBool"<<'\n';
    nl->WriteToFile("/dev/tty",args);
    }
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, typeName) && nl->IsEqual(arg2, "string") )
    {
      return nl->SymbolAtom("bool");
    }
  }

  if(traces)
    cout <<"End TypeMapTextStringBool with typeerror"<<'\n';
  return nl->SymbolAtom("typeerror");
}


ListExpr
TypeMapTextInt( ListExpr args )
{
  if(traces)
    cout << '\n' << "Start TypeMapTextInt" << '\n';
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1= nl->First(args);
    if ( nl->IsEqual(arg1, typeName))
    {
      if(traces)
        cout << '\n' << "Start TypeMapTextInt" << '\n';
      return nl->SymbolAtom("int");
    }
  }

  if(traces)
    cout <<"End TypeMapTextInt with typeerror"<<'\n';
  return nl->SymbolAtom("typeerror");
}

ListExpr
TypeMapkeywords( ListExpr args ){
  ListExpr arg;
  string nlchrs;

  if ( nl->ListLength(args) == 1 )
  {
    arg = nl->First(args);
    if ( nl->IsEqual(arg, typeName) )
      return nl->TwoElemList( nl->SymbolAtom("stream"), 
                              nl->SymbolAtom("string"));
  }
  return nl->SymbolAtom("typeerror");
}

ListExpr
TypeMapsentences( ListExpr args ){
  ListExpr arg;

  if ( nl->ListLength(args) == 1 )
  {
    arg = nl->First(args);
    if ( nl->IsEqual(arg, typeName) )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("text"));
  }
  return nl->SymbolAtom("typeerror");
}

ListExpr TypeMapDice(ListExpr args){
  if(nl->ListLength(args)!=3){
       ErrorReporter::ReportError("three arguments required");
       return nl->SymbolAtom("typeerror");
  }
  if(!nl->IsEqual(nl->First(args),"int")){
     ErrorReporter::ReportError("first argument must be an integer");
     return nl->SymbolAtom("typeerror");
  }
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  if(  (nl->AtomType(arg2)!=SymbolType ) ||
       (nl->AtomType(arg3)!=SymbolType)){
     ErrorReporter::ReportError("only simple types allowed");
     return nl->SymbolAtom("typeerror");
  }

  string t2 = nl->SymbolValue(arg2);
  string t3 = nl->SymbolValue(arg3);

  // first version, only texts, later extend to string
  if(t2!="text" || t3!="text"){
     ErrorReporter::ReportError("text as second and third argument expected");
     return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("real");
}


/*

Type Mapping for operator ~getCatalog~

*/
ListExpr TypeGetCatalog( ListExpr args )
{
  NList type(args);

  if ( type.hasLength(0) ){
    NList resTupleType = NList(NList("ObjectName"),NList(STRING)).enclose();
    resTupleType.append(NList(NList("Type"),NList(TEXT)));
    resTupleType.append(NList(NList("TypeExpr"),NList(TEXT)));
    NList resType =
        NList(NList(NList(STREAM),NList(NList(TUPLE),resTupleType)));
    return resType.listExpr();
  }
  return NList::typeError( "No argument expected!");
}

/*

Type Mapping for operators ~substr~

*/
ListExpr TypeFTextSubstr( ListExpr args )
{
  NList type(args);

  if ( !type.hasLength(3) ){
    return NList::typeError( "Three arguments expected");
  }
  if (    type.second() != NList(INT)
       || type.third()  != NList(INT)
     )
  {
    return NList::typeError( "Boundary arguments must be of type int.");
  }
  if ( type.first() == NList(TEXT) )
  {
    return NList(STRING).listExpr();
  }
  return NList::typeError( "Expected text as first argument type.");
}

/*

Type Mapping for operators ~subtext~

*/
ListExpr TypeFTextSubtext( ListExpr args )
{
  NList type(args);

  if ( !type.hasLength(3) ){
    return NList::typeError( "Three arguments expected");
  }
  if (    type.second() != NList(INT)
          || type.third()  != NList(INT)
     )
  {
    return NList::typeError( "Boundary arguments must be of type int.");
  }
  if ( type.first() == NList(TEXT) )
  {
    return NList(TEXT).listExpr();
  }
  return NList::typeError( "Expected text as first argument type.");
}

/*

Type Mapping for operator ~find~

*/
ListExpr FTextTypeMapFind( ListExpr args )
{
  NList type(args);

  if ( type.hasLength(2) &&
       (
          (type == NList(STRING, STRING))
       || (type == NList(TEXT,   TEXT  ))
       || (type == NList(STRING, TEXT  ))
       || (type == NList(TEXT,   STRING))
       )
     )
  {
    return NList(STREAM, INT).listExpr();
  }
  return NList::typeError("Expected {text|string} x {text|string}.");
}


/*

Type Mapping for signature ~text [->] bool~

*/
ListExpr FTextTypeMapTextBool( ListExpr args )
{
  NList type(args);

  if ( type.hasLength(1)
       &&( (type.first() == NList(TEXT)) )
     )
  {
    return NList(BOOL).listExpr();
  }
  return NList::typeError("Expected single text argument.");
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
  if(    (type == NList(STRING, TEXT  ))
      || (type == NList(TEXT,   TEXT  ))
      || (type == NList(TEXT,   STRING))
    )
  {
    return NList(TEXT).listExpr();
  }
  return NList::typeError("Expected (text x {text|string}) "
      "or ({text|string} x text).");
}



/*
Type Mapping Function for comparison predicates ~$<$, $<=$, $=$, $>=$, $>$, $\neq$~

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
  if(    (type == NList(STRING, TEXT  ))
          || (type == NList(TEXT,   TEXT  ))
          || (type == NList(TEXT,   STRING))
    )
  {
    return NList(BOOL).listExpr();
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
  if( !type.hasLength(1) || (type.first() != TEXT) )
  {
    return NList::typeError("Expected single 'text' argument.");
  }
  NList resTupleType = NList(NList("CmdStr"),NList(TEXT)).enclose();
  resTupleType.append(NList(NList("Success"),NList(BOOL)));
  resTupleType.append(NList(NList("Correct"),NList(BOOL)));
  resTupleType.append(NList(NList("Evaluable"),NList(BOOL)));
  resTupleType.append(NList(NList("Defined"),NList(BOOL)));
  resTupleType.append(NList(NList("IsFunction"),NList(BOOL)));
  resTupleType.append(NList(NList("ResultType"),NList(TEXT)));
  resTupleType.append(NList(NList("Result"),NList(TEXT)));
  resTupleType.append(NList(NList("ErrorMessage"),NList(TEXT)));
  resTupleType.append(NList(NList("ElapsedTimeReal"),NList(REAL)));
  resTupleType.append(NList(NList("ElapsedTimeCPU"),NList(REAL)));
  NList resulttype(NList(STREAM),NList(NList(TUPLE),resTupleType));
  return resulttype.listExpr();
}

/*
3.3 Value Mapping Functions

*/

int
ValMapTextStringBool ( Word* args, Word& result, 
                       int message, Word& local, Supplier s)

/*
Value Mapping for the ~contains~ operator with the operands text and string.

*/

{
  if(traces)
    cout <<'\n'<<"Start ValMapTextStringBool"<<'\n';
  FText* ftext1= ((FText*)args[0].addr);
  CcString* string1= ((CcString*)args[1].addr);

  result = qp->ResultStorage(s); //query processor has provided
          //a CcBool instance to take the result
  ((CcBool*)
    result.addr)->Set( true, 
                       ftext1->SearchString( *string1->GetStringval() ));
          //the first argument says the boolean
          //value is defined, the second is the
          //real boolean value)

  if(traces)
    cout <<"End ValMapTextStringBool"<<'\n';
  return 0;
}


int
ValMapTextTextBool ( Word* args, Word& result, 
                     int message, Word& local, Supplier s)

/*
Value Mapping for the ~contains~ operator with two text operands.

*/

{
  if(traces)
    cout <<'\n'<<"Start ValMapTextTextBool"<<'\n';
  FText* ftext1= ((FText*)args[0].addr);
  FText* ftext2= ((FText*)args[1].addr);

  result = qp->ResultStorage(s); //query processor has provided
          //a CcBool instance to take the result
  ((CcBool*)result.addr)->Set(true, ftext1->SearchString( ftext2->Get() ));
          //the first argument says the boolean
          //value is defined, the second is the
          //real boolean value)

  if(traces)
    cout <<"End ValMapTextTextBool"<<'\n';
  return 0;
}


int
ValMapTextInt (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Value Mapping for the ~length~ operator with a text and a string operator .

*/

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
  return ( isalnum(c)
      || c == '-' || c == 'ß' || c =='ü' || c =='Ü'
      || c == 'ö' || c == 'Ö' || c == 'ä' || c == 'Ä');
}

int
ValMapkeywords (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Creates a stream of strings of a text given as input parameter,
Recognized strings contains letters, digits and the '-' character.
The length of a string is three characters or more.

*/
{
  struct TheText {int start, nochr, strlength; const char* subw;}* thetext;

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
                 if ( IsNonStopCharacter( c ) && (stringcursor == 48) ) {
                  state = 5;
                  stringcursor = 0;
                 }         
                 else if ( IsNonStopCharacter( c ) && (stringcursor < 48) ) {
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
                   mystring = new CcString();
                   mystring->Set(true, &outstr);
                   result = SetWord(mystring);  
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
      thetext = ((TheText*) local.addr);
      delete thetext;
      return 0;
  }
  /* should not happen */
  return -1;
}

int
ValMapsentences (Word* args, Word& result, int message, Word& local, Supplier s)
{
  struct TheText {int start, strlength; const char* subw;}* thetext;
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
                   result = SetWord(returnsentence);  
                   thetext->start = textcursor;
                   local.addr = thetext;
                   return YIELD;
        }
      }
    case CLOSE:
      //cout << "close ValMapsentences" << endl;
      thetext = ((TheText*) local.addr);
      delete thetext;
      return 0;
  }
  /* should not happen */
  return -1;
}



int
ValMapDice_t_t(Word* args, Word& result, int message, 
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
  dt->appendText(arg2->Get(),true);
  dt->appendText(arg3->Get(),false);
  double res = dt->getCoeff();
  delete dt;
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
      local = SetWord( li );
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
          newTuple->PutAttribute(  0,(StandardAttribute*)objectNameValue);
          newTuple->PutAttribute(  1,(StandardAttribute*)typeValue);
          newTuple->PutAttribute(  2,(StandardAttribute*)typeExprValue);
          result = SetWord(newTuple);
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
  string mytxt = static_cast<const char*>( Ctxt->Get() );
  cout << "mytxt=\"" << mytxt << "\"" << endl;
  string mysubstring = mytxt.substr(begin-1, n+1);
  cout << "mysubstring=\"" << mysubstring << "\"" << endl;
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
4.29 Operator ~find~


*/

struct ValMapFindLocalInfo{
  string text;
  string pattern;
  unsigned textlen;
  unsigned patternlen;
  unsigned int lastPosFound;
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
      local = SetWord(li);
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

int FTextSelectFunFind( ListExpr args )
{
  NList type(args);
  if( (type.first() == NList(STRING)) && (type.second() == NList(TEXT)) )
    return 0;
  if( (type.first() == NList(TEXT)) && (type.second() == NList(STRING)) )
    return 1;
  if( (type.first() == NList(TEXT)) && (type.second() == NList(TEXT)) )
    return 2;
  if( (type.first() == NList(STRING)) && (type.second() == NList(STRING)) )
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
  if( (type.first() == NList(STRING)) && (type.second() == NList(TEXT)) )
    return 0;
  if( (type.first() == NList(TEXT)) && (type.second() == NList(STRING)) )
    return 1;
  if( (type.first() == NList(TEXT)) && (type.second() == NList(TEXT)) )
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
  if( (type.first() == NList(STRING)) && (type.second() == NList(TEXT)) )
    return 0;
  else if( (type.first() == NList(TEXT)) && (type.second() == NList(STRING)) )
    return 1;
  else if( (type.first() == NList(TEXT)) && (type.second() == NList(TEXT)) )
    return 2;
  return -1; // error
}

int FTextValueMapEvaluate( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  FText* CCommand = (FText*)(args[0].addr);
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
      *finished = (!CCommand->IsDefined());
      return 0;
    case REQUEST:
      if(local.addr == 0)
      {
        result = SetWord(0);
        return CANCEL;
      }
      finished = (bool*)(local.addr);
      if(*finished)
      {
        result = SetWord(0);
        return CANCEL;
      }
      if(!CCommand->IsDefined())
      {
        *finished = true;
        result = SetWord(0);
        return CANCEL;
      }

      // call Parser: transform expression to nested-list-string
      correct = true;
      querystring = CCommand->GetValue();
      if(mySecParser.Text2List( "query " + querystring,querystringParsed ) != 0)
      {
        errorstring = "ERROR: Text does not contain a "
            "parsable query expression.";
        correct = false;
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
      if ( correct )
      {  // remove the "query" from the list
        if ( (nl->ListLength(parsedCommand) == 2) )
        {
          parsedCommand = nl->Second(parsedCommand);
          string parsedCommandstr;
          nl->WriteToString(parsedCommandstr,parsedCommand);
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
        success =
            QueryProcessor::ExecuteQuery(parsedCommand,
                                         queryresultword,
                                         typestring,
                                         errorstring,
                                         correct,
                                         evaluable,
                                         defined,
                                         isFunction);
        myTimeReal = myTimer.diffSecondsReal();
        myTimeCPU  = myTimer.diffSecondsCPU();
        cout << "TimeReal = " << myTimeReal << endl;
        cout << "TimeCPU  = " << myTimeCPU  << endl;

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
                && ( typestring != "typeerror"  )
            )
          { // yielded a typeerror
            ListExpr valueList = SecondoSystem::GetCatalog()
                ->OutObject(queryResType,queryresultword);
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

      newTuple->PutAttribute(  0,(StandardAttribute*)CcCmdStr);
      newTuple->PutAttribute(  1,(StandardAttribute*)CcSuccess);
      newTuple->PutAttribute(  2,(StandardAttribute*)CcCorrect);
      newTuple->PutAttribute(  3,(StandardAttribute*)CcEvaluable);
      newTuple->PutAttribute(  4,(StandardAttribute*)CcDefined);
      newTuple->PutAttribute(  5,(StandardAttribute*)CcIsFunction);
      newTuple->PutAttribute(  6,(StandardAttribute*)CcResultType);
      newTuple->PutAttribute(  7,(StandardAttribute*)CcResult);
      newTuple->PutAttribute(  8,(StandardAttribute*)CcErrorMessage);
      newTuple->PutAttribute(  9,(StandardAttribute*)CcElapsedTimeReal);
      newTuple->PutAttribute( 10,(StandardAttribute*)CcElapsedTimeCPU);

      result = SetWord(newTuple);
      resultTupleType->DeleteIfAllowed();
      *finished = true;
      return YIELD;

    case CLOSE:
      if(local.addr != 0)
      {
        finished = (bool*)(local.addr);
        delete finished;
      }
      return 0;
  }
  return 0;
}

/*
3.4 Definition of Operators

*/

/*
Used to explain signature, syntax and meaning of the operators of the type ~text~.

*/

const string containsStringSpec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" )"
    "("
    "<text>(" +typeName+" string) -> bool</text--->"
    "<text>_ contains _</text--->"
    "<text>Search string in "+typeName+".</text--->"
    ")"
  ")";

const string containsTextSpec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" )"
    "("
    "<text>("+typeName+" "+typeName+") -> bool</text--->"
    "<text>_ contains _</text--->"
    "<text>Search second "+typeName+" in first "+typeName+".</text--->"
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

const string FTextplusSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> {text | string} x text -> text \n"
    " text x {text | string -> text}</text--->"
    "<text>t1 + t2</text--->"
    "<text>Returns the concatenation of a combination of text with another "
    "text or string value.</text--->"
    "<text>query ('Hello' + \" world\" + '!')</text--->"
    ") )";

// Compare predicates
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

const string FTextEvaluateSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text -> stream(tuple((CmdStr text) (Success bool) "
    "(Evaluable bool) (Defined bool) (IsFunction bool)"
    "(ResultType text) (Result text) (ErrorMessage text) "
    "(ElapsedTimeReal real) (ElapsedTimeCPU real)))</text--->"
    "<text>evaluate( _ )</text--->"
    "<text>Interprets the text argument as a Secondo Executable Language "
    "query expression and evaluates it. The calculated result returned as a "
    "nested list expression. Operator's result is a stream containing at most "
    "1 tuple with a copy of the command, the result, errormessage, runtimes, "
    "and some more status information.</text--->"
    "<text>query evaluate('ten feed filter[.No > 5] count') "
    "filter[.Success] count > 1</text--->"
    ") )";

/*
The Definition of the operators of the type ~text~.

*/

Operator containsString
(
  "contains",           //name
  containsStringSpec,   //specification
  ValMapTextStringBool, //value mapping
  Operator::SimpleSelect,         //trivial selection function
  TypeMapTextStringBool //type mapping
);


Operator containsText
(
  "contains",           //name
  containsTextSpec,     //specification
  ValMapTextTextBool,   //value mapping
  Operator::SimpleSelect,         //trivial selection function
  TypeMapTextTextBool   //type mapping
);

Operator length
(
  "length",             //name
  lengthSpec,           //specification
  ValMapTextInt,        //value mapping
  Operator::SimpleSelect,         //trivial selection function
  TypeMapTextInt        //type mapping
);

Operator getkeywords
(
  "keywords",            //name
  keywordsSpec,          //specification
  ValMapkeywords,        //value mapping
  Operator::SimpleSelect,          //trivial selection function
  TypeMapkeywords        //type mapping
);

Operator getsentences
(
  "sentences",            //name
  sentencesSpec,          //specification
  ValMapsentences,        //value mapping
  Operator::SimpleSelect,           //trivial selection function
  TypeMapsentences        //type mapping
);

Operator diceCoeff(
   "dice",
   diceSpec,
   ValMapDice_t_t,
   Operator::SimpleSelect,
   TypeMapDice
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
    FTextSelectFunFind,
    FTextTypeMapFind
    );

Operator ftextisempty
    (
    "isempty",
    FTextisemptySpec,
    FTextValMapIsEmpty,
    Operator::SimpleSelect,
    FTextTypeMapTextBool
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

Operator ftextevaluate
    (
    "evaluate",
    FTextEvaluateSpec,
    FTextValueMapEvaluate,
    Operator::SimpleSelect,
    FTextTypeMapEvaluate
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
    ftext.AssociateKind("DATA");
    AddOperator( &containsString );
    AddOperator( &containsText );
    AddOperator( &length );
    AddOperator( &getkeywords );
    AddOperator( &getsentences );
    AddOperator( &diceCoeff);
    AddOperator( &ftextgetcatalog );
    AddOperator( &ftextsubstr );
    AddOperator( &ftextsubtext );
    AddOperator( &ftextfind );
    AddOperator( &ftextisempty );
    AddOperator( &ftextplus );
    AddOperator( &ftextless );
    AddOperator( &ftextlesseq );
    AddOperator( &ftexteq );
    AddOperator( &ftextbiggereq );
    AddOperator( &ftextbigger );
    AddOperator( &ftextneq );
    AddOperator( &ftextevaluate );
//     AddOperator( &ftextreplace );
//     AddOperator( &ftexttoupper );
//     AddOperator( &ftexttostring );
//     AddOperator( &ftexttotext );

    
    LOGMSG( "FText:Trace",
      cout <<"End FTextAlgebra() : Algebra()"<<'\n';
    )
  }

  ~FTextAlgebra() {};
};


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
  FTextAlgebra* ptr = new FTextAlgebra;
  ptr->Init(nl, qp, am); 
  return ptr;
}

