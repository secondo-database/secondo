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

The algebra ~FText~ provides the type constructor ~text~ and two operators:

(i) ~contains~, which search text or string in a text.

(ii) ~length~ which give back the length of a text.

1 Preliminaries

1.1 Includes

*/


#include <cstring>
#include <iostream>

#include "StandardAttribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h" //needed because we return a CcBool in an op.
#include "LogMsg.h"

using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Type Constructor ~text~

2.1 Data Structure - Class ~FText~

*/

const string typeName="text";
const bool traces=false;


class FText: public StandardAttribute
{
public:

  FText();
  FText(bool newDefined, const char *newText = NULL);
  FText(FText&);
  ~FText();
  void Destroy();

  bool  SearchString( const char* subString );
  void  Set( const char *newString );
  void  Set( bool newDefined, const char *newString );
  int TextLength() const;
  const char *Get();

/*************************************************************************

  The following virtual functions:
  IsDefined, SetDefined, HashValue, CopyFrom, Compare, Sizeof, Clone, Print, Adjacent
  need to be defined if we want to use ~text~ as an attribute type in tuple definitions.

*************************************************************************/

  bool     IsDefined() const;
  void     SetDefined(bool newDefined);
  size_t   HashValue();
  void     CopyFrom(StandardAttribute* right);
  int      Compare(Attribute * arg);
  int      Sizeof() const;
  FText*   Clone();
  ostream& Print(ostream &os);
  bool     Adjacent(Attribute * arg);

  int NumOfFLOBs();
  FLOB* GetFLOB( const int );

private:
  FLOB theText;
  bool defined;
};


FText::FText()
{
  LOGMSG( "FText:Trace", cout << '\n' <<"Start FText()"<<'\n'; )
  LOGMSG( "FText:Trace",  cout <<"End FText()"<<'\n'; )
}

FText::FText( bool newDefined, const char *newText ) :
theText( 0 )
{
  LOGMSG( "FText:Trace", cout << '\n' <<"Start FText(bool newDefined, textType newText)"<<'\n'; )
  Set( newDefined, newText );
  LOGMSG( "FText:Trace",  cout <<"End FText(bool newDefined, textType newText)"<<'\n'; )
}

FText::FText( FText& f ) :
theText( 0 )
{
  LOGMSG( "FText:Trace", cout << '\n' <<"Start FText(FText& f)"<<'\n'; )
  Set( f.defined, f.theText.BringToMemory() );
  LOGMSG( "FText:Trace",  cout <<"End FText(FText& f)"<<'\n'; )
}

FText::~FText()
{
  LOGMSG( "FText:Trace",  cout << '\n' <<"Start ~FText()"<<'\n'; )
  LOGMSG( "FText:Trace",  cout <<"End ~FText()"<<'\n'; )
}

void FText::Destroy()
{
  theText.Destroy();
  defined = false;
}

bool FText::SearchString( const char* subString )
{
  const char *text = theText.BringToMemory();
  return strstr( text, subString ) != NULL;
}

void FText::Set( const char *newString )
{
  LOGMSG( "FText:Trace", cout << '\n' << "Start Set with *newString='"<< newString << endl; )

  Set( true, newString );

  LOGMSG( "FText:Trace", cout <<"End Set"<<'\n'; )
}

void FText::Set( bool newDefined, const char *newString )
{
  LOGMSG( "FText:Trace", cout << '\n' << "Start Set with *newString='"<< newString << endl; )

  if( newString != NULL )
  {
    theText.Resize( strlen( newString ) + 1 );
    theText.Put( 0, strlen( newString ) + 1, newString );
  }
  defined = newDefined;

  LOGMSG( "FText:Trace", cout <<"End Set"<<'\n'; )
}

int FText::TextLength() const
{
  return theText.Size() - 1;
}

const char *FText::Get()
{
  return theText.BringToMemory();
}

/*

2.2 Class methods for using ~text~ in tuple definitions

In the following, we give the definitions of the virtual functions which are needed
if we want to use ~text~ as an attribute type in tuple definitions.

*/

bool FText::IsDefined() const
{
  return defined;
}

void FText::SetDefined( bool newDefined )
{
  if(traces)
    cout << '\n' << "Start SetDefined" << '\n';
  defined = newDefined;
}

size_t FText::HashValue()
{
  if(traces)
    cout << '\n' << "Start HashValue" << '\n';

  if(!defined)
    return 0;

  unsigned long h = 0;
  const char* s = theText.BringToMemory();
  while(*s != 0)
  {
    h = 5 * h + *s;
    s++;
  }
  return size_t(h);
}

void FText::CopyFrom( StandardAttribute* right )
{
  if(traces)
    cout << '\n' << "Start CopyFrom" << '\n';

  FText* r = (FText*)right;
  const char *s = r->theText.BringToMemory();
  Set( r->defined, s );
}

int FText::Compare( Attribute *arg )
{
  if(traces)
    cout << '\n' << "Start Compare" << '\n';

  if(!IsDefined() && !arg->IsDefined())
    return 0;

  if(!IsDefined())
    return -1;

  if(!arg->IsDefined())
    return 1;

  FText* f = (FText* )(arg);

  if ( !f )
    return -2;

  const char *s1 = f->theText.BringToMemory(),
             *s2 = this->theText.BringToMemory();

  return strcmp( s2, s1 );
}


int FText::Sizeof() const
{
  if(traces)
    cout << '\n' << "Start Sizeof" << '\n';

  return this->TextLength();
}


FText* FText::Clone()
{
  return new FText( *this );
}


ostream& FText::Print(ostream &os)
{
  return (os << theText.BringToMemory());
}


bool FText::Adjacent(Attribute *arg)
{
  if(traces)
    cout << '\n' << "Start Adjacent" << '\n';

  const char *a = theText.BringToMemory(),
             *b = ((FText *)arg)->theText.BringToMemory();


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

int FText::NumOfFLOBs()
{
  return 1;
}

FLOB* FText::GetFLOB( const int i )
{
  assert( i == 0 );
  return &theText;
}


/*

2.3 Functions for using ~text~ in tuple definitions

The following Functions must be defined if we want to use ~text~ as an attribute type in tuple definitions.

*/

static Word
CreateFText( const ListExpr typeInfo )
{
  if(traces)
    cout << '\n' << "Start CreateFText" << '\n';
  return (SetWord(new FText( false )));
}

static void
DeleteFText( Word& w )
{
  if(traces)
    cout << '\n' << "Start DeleteFText" << '\n';
  FText *f = (FText *)w.addr;

  f->Destroy();
  delete f;
  w.addr = 0;
}

static void
CloseFText( Word& w )
{
  if(traces)
    cout << '\n' << "Start CloseFText" << '\n';
  delete (FText*) w.addr;
  w.addr = 0;
}

static Word
CloneFText( const Word& w )
{
  if(traces)
    cout << '\n' << "Start CloneFText" << '\n';
  return SetWord( ((FText *)w.addr)->Clone() );
}

static int
SizeOfFText()
{
  return sizeof( FText );
}

static void*
CastFText( void* addr )
{
  if(traces)
    cout << '\n' << "Start CastFText" << '\n';
  return (new (addr) FText);
}


/*

2.4 ~In~ and ~Out~ Functions

*/

static ListExpr
OutFText( ListExpr typeInfo, Word value )
{
  if(traces)
    cout << '\n' << "Start OutFText" << '\n';
  FText* pftext;
  pftext = (FText*)(value.addr);
  ListExpr TextAtomVar=nl->TextAtom();

  if(traces)
    cout <<"pftext->Get()='"<< pftext->Get() <<"'\n";
  nl->AppendText( TextAtomVar, pftext->Get() );

  if(traces)
    cout <<"End OutFText" << '\n';
  return nl->OneElemList(TextAtomVar);
}


static Word
InFText( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(traces)
    cout << '\n' << "Start InFText with ListLength "<<nl->ListLength( instance );
  ListExpr First;
  if (nl->ListLength( instance ) == 1 || nl->ListLength( instance ) == 4 )
    First = nl->First(instance);
  else
    First =instance;
  if ( nl->IsAtom(First) && nl->AtomType(First) == TextType)
  {
    string buffer;
    TextScan textScan = nl->CreateTextScan (First);
    nl->GetText (textScan, nl->TextLength( First ), buffer);
    nl->DestroyTextScan (textScan);

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

static ListExpr
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

static bool
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
  0, 0, CloseFText, CloneFText, //object open, save, close, and clone
  CastFText,                    //cast function
  SizeOfFText,					//sizeof function
  CheckFText,                   //kind checking function
  0,                            //predef. pers. function for model
  TypeConstructor::DummyInModel,
  TypeConstructor::DummyOutModel,
  TypeConstructor::DummyValueToModel,
  TypeConstructor::DummyValueListToModel );


/*

3 Creating Operators

3.1 Type Mapping Functions

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol ~typeerror~.

*/

static ListExpr
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


static ListExpr
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


static ListExpr
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
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("string"));
  }
  return nl->SymbolAtom("typeerror");
}

/*

3.2 Selection Function

Is used to select one of several evaluation functions for an overloaded
operator, based on the types of the arguments. In case of a non-overloaded
operator, we just have to return 0.

*/

static int
simpleSelect (ListExpr args )
{
  return 0;
}


/*
3.3 Value Mapping Functions

*/

static int
ValMapTextStringBool (Word* args, Word& result, int message, Word& local, Supplier s)
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
  ((CcBool*)result.addr)->Set(true, ftext1->SearchString( *string1->GetStringval() ));
          //the first argument says the boolean
          //value is defined, the second is the
          //real boolean value)

  if(traces)
    cout <<"End ValMapTextStringBool"<<'\n';
  return 0;
}


static int
ValMapTextTextBool (Word* args, Word& result, int message, Word& local, Supplier s)
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


static int
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
  STRING outstr;
  Word arg0;
  char c;
  CcString* mystring;

  switch( message )
  {
    case OPEN:
      //cout << "open ValMapkeywords" << endl;     
      qp->Request(args[0].addr, arg0);     
      thetext = new TheText;
      thetext->start = 0;
      thetext->nochr = 0;
      thetext->subw = ((FText*)arg0.addr)->Get();
      thetext->strlength = ((FText*)arg0.addr)->TextLength();
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
	           if ( isalnum(c) || c == '-' || c == 'ä' || c =='ö' || c =='ü'
		                   || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
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
	           if ( isalnum(c) || c == '-' || c == 'ä' || c =='ö' || c =='ü'
		                   || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
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
	           if ( isalnum(c) || c == '-' || c == 'ä' || c =='ö' || c =='ü'
		                   || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
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
	           if ( isalnum(c) || c == '-' || c == 'ä' || c =='ö' || c =='ü'
		                   || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
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
	         if ( (isalnum(c) || c == '-'|| c == 'ä' || c =='ö' || c =='ü'
		                 || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
		                 && (stringcursor == 48) ) {
	         state = 5;
		 stringcursor = 0;
		 }	         
	         else if ( (isalnum(c) || c == '-'|| c == 'ä' || c =='ö' || c =='ü'
		                 || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
				 && (stringcursor < 48) ) {
		   outstr[stringcursor] = c;
		   stringcursor++;
		   state = 4;
		 }
		 else {
		   //if ( c == '\0' ) { outstr[stringcursor] = c; stringcursor ++; }
		   if ( textcursor == thetext->strlength ) { outstr[stringcursor] = c; stringcursor++; };
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
             "<text>let Keyword = documents feed extendstream[kword: .title keywords] consume</text--->"
             ") )"; 

/*
The Definition of the operators of the type ~text~.

*/

Operator containsString
(
  "contains",           //name
  containsStringSpec,   //specification
  ValMapTextStringBool, //value mapping
  Operator::DummyModel, //dummy model mapping, defined in Algebra.h
  simpleSelect,         //trivial selection function
  TypeMapTextStringBool //type mapping
);


Operator containsText
(
  "contains",           //name
  containsTextSpec,     //specification
  ValMapTextTextBool,   //value mapping
  Operator::DummyModel, //dummy model mapping, defined in Algebra.h
  simpleSelect,         //trivial selection function
  TypeMapTextTextBool   //type mapping
);

Operator length
(
  "length",             //name
  lengthSpec,           //specification
  ValMapTextInt,        //value mapping
  Operator::DummyModel, //dummy model mapping, defined in Algebra.h
  simpleSelect,         //trivial selection function
  TypeMapTextInt        //type mapping
);

Operator getkeywords
(
  "keywords",            //name
  keywordsSpec,          //specification
  ValMapkeywords,        //value mapping
  Operator::DummyModel,  //dummy model mapping, defined in Algebra.h
  simpleSelect,          //trivial selection function
  TypeMapkeywords        //type mapping
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
    LOGMSG( "FText:Trace",
      cout <<"End FTextAlgebra() : Algebra()"<<'\n';
    )
  }

  ~FTextAlgebra() {};
};

FTextAlgebra FTextAlgebra;

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
InitializeFTextAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  if(traces)
    cout << '\n' <<"InitializeFTextAlgebra"<<'\n';
  nl = nlRef;
  qp = qpRef;
  return (&FTextAlgebra);
}

