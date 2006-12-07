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
#include "StandardTypes.h" //needed because we return a CcBool in an op.
#include "LogMsg.h"
#include "DiceCoeff.h"

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
  const char* t = 0;
  theText.Get(0, &t);
  string s(t);
  return (os << "'" << s.substr(0,20) << " ... '" );
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
  0, 0, CloseFText, CloneFText, //object open, save, close, and clone
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
                   if ( isalnum(c) 
                        || c == '-' || c == 'ä' || c =='ö' || c =='ü'
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
                   if ( isalnum(c) 
                        || c == '-' || c == 'ä' || c =='ö' || c =='ü'
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
                   if ( isalnum(c) 
                        || c == '-' || c == 'ä' || c =='ö' || c =='ü'
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
                   if ( isalnum(c) 
                        || c == '-' || c == 'ä' || c =='ö' || c =='ü'
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
                 if ( (isalnum(c) 
                       || c == '-'|| c == 'ä' || c =='ö' || c =='ü'
                       || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
                       && (stringcursor == 48) ) {
                 state = 5;
                 stringcursor = 0;
                 }         
                 else if ( (isalnum(c) 
                           || c == '-'|| c == 'ä' || c =='ö' || c =='ü'
                           || c == 'Ä' || c == 'Ö' || c == 'Ü' || c == 'ß') 
                           && (stringcursor < 48) ) {
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
/*

*/
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

