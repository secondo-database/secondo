/*
\def\CC{C\raise.22ex\hbox{{\footnotesize +}}\raise.22ex\hbox{\footnotesize +}\xs
pace}
\centerline{\LARGE \bf  DisplayTTY}
 
\centerline{Friedhelm Becker , Mai1998}
 
\begin{center}
\footnotesize
\tableofcontents
\end{center}
 
1 Overview

There must be exactly one TTY display function of type DisplayFunction
for every type constructor provided by any of the algebra modules which
are loaded by *AlgebraManager2*. The first parameter is the type
expression in numeric nested list format describing the value which is
going to be displayed by the display function. The second parameter is
this value in nested list format.
 
1.2 Includes and defines

maxLineLength gives the maximum length of an input line (an command)
which will be read.

*/

using namespace std;

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "DisplayTTY.h"
#include "NestedList.h"
#include "SecondoInterface.h"
#include "AlgebraTypes.h"

#define LINELENGTH 80

/*
1.3 Managing display functions
 
The map *displayFunctions* holds all existing display functions. It
is indexed by a string created from the algebraId and the typeId of
the corresponding type constructor.

*/

SecondoInterface* DisplayTTY::si = 0;
NestedList*       DisplayTTY::nl = 0;
map<string,DisplayFunction> DisplayTTY::displayFunctions;

/*
The function *CallDisplayFunction* uses its first argument *idPair*
--- consisting of the two-elem-list $<$algebraId, typeId$>$ --- to
find the right display function in the array *displayFunctions*. The
arguments *typeArg* and *valueArg* are simply passed to this display
function.

*/

void
DisplayTTY::CallDisplayFunction( const ListExpr idPair,
                                 ListExpr type,
                                 ListExpr numType,
                                 ListExpr value )
{
  ostringstream osId;
  osId << "[" << nl->IntValue( nl->First( idPair ) )
       << "|" << nl->IntValue( nl->Second( idPair ) ) << "]";
  map<string,DisplayFunction>::iterator dfPos =
    displayFunctions.find( osId.str() );
  if ( dfPos != displayFunctions.end() )
  {
    (*(dfPos->second))( type, numType, value );
  }
  else
  {
    DisplayGeneric( type, numType, value );
  }
}

/*
The procedure *InsertDisplayFunction* inserts the procedure given as
second argument into the array displayFunctions at the index which is
determined by the type constructor name given as first argument.

*/

void
DisplayTTY::InsertDisplayFunction( const string& name,
                                   DisplayFunction df )
{
  int algebraId, typeId;
  si->GetTypeId( ExecutableLevel, name, algebraId, typeId );
  ostringstream osId;
  osId << "[" << algebraId << "|" << typeId << "]";
  displayFunctions[osId.str()] = df;
}

/*
1.4 Display functions
 
Display functions of the DisplayTTY module are used to  transform a
nested list value into a pretty printed output in text format. Display
functions which are called with a value of compound type usually call
recursively the display functions of the subtypes, passing the subtype
and subvalue, respectively.
 
*/

void
DisplayTTY::DisplayGeneric( ListExpr type, ListExpr numType, ListExpr value )
{
  cout << "No specific display function defined. Generic function used." << endl;
  cout << "Type: ";
  nl->WriteListExpr( type, cout );
  cout << endl << "Value: ";
  nl->WriteListExpr( value, cout );
}

void
DisplayTTY::DisplayRelation( ListExpr type, ListExpr numType, ListExpr value )
{
  type = nl->Second( type );
  numType = nl->Second( numType );
  CallDisplayFunction( nl->First( numType ), type, numType, value );
}

int
DisplayTTY::MaxHeaderLength( ListExpr type )
{
  int max, len;
  string s;
  max = 0;
  while (!nl->IsEmpty( type ))
  {
    s = nl->StringValue( nl->First( type ) );
    len = s.length();
    if ( len > max )
    {
      max = len;
    }
    type = nl->Rest( type );
  }
  return (max);
}


int
DisplayTTY::MaxAttributLength( ListExpr type )
{
  int max, len;
  string s;
  max = 0;
  while (!nl->IsEmpty( type ))
  {
    s = nl->SymbolValue( nl->First( nl->First( type ) ) );
    len = s.length();
    if ( len > max )
    {
      max = len;
    }
    type = nl->Rest( type );
  }
  return (max);
}

void
DisplayTTY::DisplayTuple( ListExpr type, ListExpr numType,
                          ListExpr value, const int maxNameLen )
{
  string s, blanks;
  while (!nl->IsEmpty( value ))
  {
    cout << endl;
    s = nl->SymbolValue( nl->First( nl->First( numType ) ) );
    blanks.assign( maxNameLen-s.length() , ' ' );
    cout << blanks << s << ": ";
    CallDisplayFunction( nl->Second( nl->First( numType ) ),
                         nl->Second( nl->First( type ) ),
                         nl->Second( nl->First( numType ) ),
                         nl->First( value ) );
    type    = nl->Rest( type );
    numType = nl->Rest( numType );
    value   = nl->Rest( value );
  }
}

void
DisplayTTY::DisplayTuples( ListExpr type, ListExpr numType, ListExpr value )
{
  int maxAttribNameLen = MaxAttributLength( nl->Second( numType ) );
  while (!nl->IsEmpty( value ))
  {  
    DisplayTuple( nl->Second( type ), nl->Second( numType ),
                  nl->First( value ), maxAttribNameLen );
    value = nl->Rest( value );
    cout << endl;
  }
}

void
DisplayTTY::DisplayInt( ListExpr type, ListExpr numType, ListExpr value )
{
  if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    cout << "UNDEFINED";
  } 
  else
  {
    cout << nl->IntValue( value );
  }
}

void
DisplayTTY::DisplayReal( ListExpr type, ListExpr numType, ListExpr value )
{
  if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    cout << "UNDEFINED";
  } 
  else
  {
    cout << nl->RealValue( value );
  }
}
 
void
DisplayTTY::DisplayBoolean( ListExpr list, ListExpr numType, ListExpr value )
{
  if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    cout << "UNDEFINED";
  } 
  else
  {
    if ( nl->BoolValue( value ) )
    {
      cout << "TRUE";
    }
    else
    {
      cout << "FALSE";
    }
  }
}

void
DisplayTTY::DisplayString( ListExpr type, ListExpr numType, ListExpr value )
{
  if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    cout << "UNDEFINED";
  } 
  else
  { 
    cout << nl->StringValue( value );
  }
}

void
DisplayTTY::DisplayFun( ListExpr type, ListExpr numType, ListExpr value )
{
  cout << "Function type: ";
  nl->WriteListExpr( type, cout );
  cout << endl << "Function value: ";
  nl->WriteListExpr( value, cout );
  cout << endl;
}

void
DisplayTTY::DisplayDate( ListExpr type, ListExpr numType, ListExpr value)
{
   /*   ListExpr d, m, y;
   if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
    {
      cout << "UNDEFINED";
    }
   else
   {
      d =  nl->Second( value ) ;
      m =  nl->Third( value ) ;
      y = nl->Fourth( value );  
      nl->WriteListExpr( d, cout );
      cout << ",";
      nl->WriteListExpr( m, cout );
      cout << ",";
      nl->WriteListExpr( y, cout );
   }
   */
  if (nl->IsAtom(value) && nl->AtomType(value)==StringType)
      cout <<nl->StringValue(value);
  else
      cout <<"Incorrect Data Format!";
}

void
DisplayTTY::DisplayResult( ListExpr type, ListExpr value )
{
  int algebraId, typeId;
  string  name;
  si->LookUpTypeExpr( ExecutableLevel, type, name, algebraId, typeId );
  ListExpr numType = si->NumericTypeExpr( ExecutableLevel, type );
  if ( !nl->IsAtom( type ) )
  {
    CallDisplayFunction( nl->First( numType ), type, numType, value );
  }
  else
  {
    CallDisplayFunction( numType, type, numType, value );
  }
  nl->Destroy( numType );
  cout << endl;         
}

void
DisplayTTY::DisplayDescriptionLines( ListExpr value, int  maxNameLen)
{
  string s, blanks, printstr, line, restline, descrstr;
  int position, lastblank;
  ListExpr valueheader, valuedescr;
  bool firstline, lastline;
  
  valueheader = nl->Second(value);
  valuedescr  = nl->Third(value);
  
  cout << endl;
  
  blanks.assign( maxNameLen-4 , ' ' );
  cout << blanks << "Name: " << nl->SymbolValue(nl->First(value)) << endl;
    
  while (!nl->IsEmpty( valueheader ))
  {  
    s = nl->StringValue( nl->First( valueheader ));
    blanks.assign( maxNameLen-s.length() , ' ' );
    //cout << blanks << s << ": ";
    printstr = blanks + s + ": ";
    
    if( nl->IsAtom(nl->First( valuedescr ))) //&& 
      //nl->AtomType(nl->First(valuedescr))==StringType)
    {
      if ( nl->AtomType(nl->First(valuedescr))==StringType )
      //DisplayString(nl->TheEmptyList(), nl->TheEmptyList(), 
        //nl->First(valuedescr));
      printstr += nl->StringValue( nl->First(valuedescr) );
      else
      {
        if ( nl->AtomType(nl->First(valuedescr))==TextType )
	{
	  TextScan txtscan = nl->CreateTextScan(nl->First(valuedescr));
	  descrstr = "";
	  nl->GetText(txtscan, nl->TextLength(nl->First(valuedescr)),descrstr);
	  printstr += descrstr;
	  //cout << printstr << endl;
	  nl->DestroyTextScan(txtscan);
	}
      }
      //check whether line break is necessary
      if (printstr.length() <= LINELENGTH) cout << printstr << endl; 
      //cout << endl;
      else
      {
        firstline = true;
        position = 0;
	lastblank = -1;
	line = "";
        for (unsigned i = 1; i <= printstr.length(); i++)
        {
	  line += printstr[i-1];
	  //cout << line << endl;
	  if (printstr[i-1] == ' ') lastblank = position;
	  position++;
	  lastline = (i == printstr.length());
	  if ( (firstline && (position == LINELENGTH)) || (!firstline &&
	  (position == (LINELENGTH-maxNameLen-2))) || lastline )
	  //if ((position == LINELENGTH) || lastline)
	  {
	    if (lastblank > 0)
	    {
	      if (firstline)
	      {
	        if (lastline && (line.length() <= LINELENGTH))
		{
		  cout << line << endl;
		}
	        else cout << line.substr(0, lastblank) << endl;
		firstline = false;
	      }
	      else
	      {
	        blanks.assign( maxNameLen+2 , ' ' );
		if (lastline && (line.length() <= LINELENGTH))
		{
		  cout << blanks << line << endl;
		}
	        else cout << blanks << line.substr(0, lastblank) << endl;
	      }
	      restline = line.substr(lastblank+1, position);
	      line = "";
	      line += restline;
	      lastblank = -1;
	      position = line.length();
	    }
	    else 
	    {
	      if (firstline)
	      {
	        cout << line << endl;
		firstline = false;
	      }
	      else
	      {
	        blanks.assign( maxNameLen+2 , ' ' );
	        cout << blanks << line << endl;
	      }
	      line = "";
	      lastblank = -1;
	      position = 0;
	    }	      
	  }
	}
      }
    }	  
    valueheader   = nl->Rest( valueheader ); 
    valuedescr    = nl->Rest( valuedescr ); 
  }
  nl->Destroy( valueheader );
  nl->Destroy( valuedescr );  
}

ListExpr
DisplayTTY::ConcatLists( ListExpr list1, ListExpr list2)
{
  if (nl->IsEmpty(list1))
  {
    return list2;
  }
  else
  {
    return nl->Cons(nl->First(list1), ConcatLists(nl->Rest(list1), list2));
  }
}

void
DisplayTTY::DisplayResult2( ListExpr value )
{
  ListExpr headerlist, concatenatedlist;
  
  headerlist = value;
  concatenatedlist = nl->TheEmptyList();
  concatenatedlist = nl->Second(nl->First(headerlist));
  headerlist = nl->Rest(headerlist);
  while (!nl->IsEmpty( headerlist ))
  {
    concatenatedlist = 
      ConcatLists( concatenatedlist, nl->Second(nl->First(headerlist)) );
    headerlist = nl->Rest(headerlist);
  }
  
  int maxHeadNameLen = MaxHeaderLength( concatenatedlist ); 
  //int  maxHeadNameLen = 17; 
  //int  maxHeadNameLen = 9; 
  while (!nl->IsEmpty( value ))
  {   
    DisplayDescriptionLines( nl->First(value), maxHeadNameLen );
    value   = nl->Rest( value );
  }
  nl->Destroy( headerlist );
  nl->Destroy( concatenatedlist );  
}

void
DisplayTTY::Initialize( SecondoInterface* secondoInterface )
{
  si = secondoInterface;
  nl = si->GetNestedList();
  InsertDisplayFunction( "int",    &DisplayInt );
  InsertDisplayFunction( "real",   &DisplayReal );
  InsertDisplayFunction( "bool",   &DisplayBoolean );
  InsertDisplayFunction( "string", &DisplayString );
  InsertDisplayFunction( "rel",    &DisplayRelation );
  InsertDisplayFunction( "tuple",  &DisplayTuples );
  InsertDisplayFunction( "map",    &DisplayFun );
  InsertDisplayFunction( "date",    &DisplayDate );
}

