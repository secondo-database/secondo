/*
//paragraph [1] Title: [{\Large \bf ] [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[->] [$\rightarrow $]

1 Implementation of Stable Nested Lists

Copyright (C) 1995 Gral Support Team

November 1995 Ralf Hartmut G[ue]ting

March 8,  1996 Holger Schenk

May 13, 1996 Carsten Mund

June 10, 1996 RHG Changed result type of procedure RealValue back to REAL.

September 24, 1996 RHG Cleaned up PD representation.

October 22, 1996 RHG Corrected EndOfList, Nth-Element, and Second ... Sixth. Made operations ~ListLength~ and ~WriteListExpr~ available. Changed internal procedure ~WriteList~ so that atoms following a list are written indented to the same level as a preceding list. This affects all output of lists. 

November 14, 1996 RHG Removed ~SetValid~ commands.

November 18, 1996 RHG Changed Import from ~CatalogManager~ to ~Ctable~ in component ~ListsAndTables~.

November 22, 1996 RHG Added a final ~WriteLn~ in ~WriteToFile~.

January 20, 1997 RHG Corrected procedure ~AtomType~. Removed error message from ~ReadFromString~.

September 26, 1997 Stefan Dieker Corrected wrong order of CTable calls in 
~Append~, ~AppendText~, and all procedures creating atoms.

May 8, 1998 RHG Changed the way how text atoms are written to the screen. Affects procedures ~WriteAtom~, ~WriteLists~.

October 12, 1998 Stefan Dieker. The name of the temporary file created by ~ReadFromString~ is now unique for each call of ~ReadFromString~.

December 1, 2001 Ulrich Telle Port to C++

November 28, 2002 M. Spiekermann coding of reportVectorSizes(). 

December 05, 2002 M. Spiekermann methods InitializeListMemory() and CopyList/CopyRecursive implemented.

1 Introduction

A nested list is represented by four stable tables called ~Nodes~, ~Ints~, 
~Strings~, and ~Texts~.

2 Imports

*/

using namespace std;

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "CharTransform.h"
#include "NestedList.h"
#include "NLParser.h"

/*
3 Preliminaries

3.1 Constants, Types \& Variables

Definitions implied by convention:

*/

bool NestedList::doDestroy = false;
/*
This constant defines whether the ~Destroy~ method really destroys a
nested list. Only if ~doDestroy~ is ~true~, nested lists are destroyed.

*/

NestedList::NestedList( Cardinal NodeEntries, Cardinal ConstEntries,
		        Cardinal StringEntries, Cardinal TextEntries )
{
   intTable = 0;
   stringTable = 0;
   nodeTable = 0;
   textTable = 0;
   initializeListMemory(NodeEntries, ConstEntries, StringEntries, TextEntries);
}


NestedList::~NestedList()
{
   deleteListMemory();
}

void
NestedList::deleteListMemory()
{
   if(intTable && stringTable && nodeTable && textTable)
   {
     delete intTable; intTable = 0;
     delete stringTable; stringTable = 0;
     delete nodeTable; nodeTable = 0;
     delete textTable; textTable = 0;
   }
}


void
NestedList::initializeListMemory( Cardinal NodeEntries, Cardinal ConstEntries,
		                  Cardinal StringEntries, Cardinal TextEntries ) 
{   
   deleteListMemory();
   
   nodeTable   = new CTable<NodeRecord>(NodeEntries);
   intTable    = new CTable<Constant>(ConstEntries);
   stringTable = new CTable<StringRecord>(StringEntries);
   textTable   = new CTable<TextRecord>(TextEntries);
}

/*
3.1 PrintTableTexts

PrintTableTexts displays the contents of the Table 'Texts' on the screen. 
The procedure was very helpful during the test phase of the module, and is 
not used anywhere in the current version of the module. 

*/

void
NestedList::PrintTableTexts()
{
  Cardinal i, numEntries;

  numEntries = textTable->NoEntries();
  cout << "/////////////////////////////////////////////////////////////" << endl;
  for ( i = 1; i <= numEntries; i++ )
  {
    if ( textTable->IsValid( i ) )
    {
      TextRecord& entry = (*textTable)[i];
      cout << endl << "Index: " << i;
      cout << endl << "Field: " << entry.field;
      cout << endl << "Next:  " << entry.next << endl;
    }
  }
  cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" << endl;
}

/*
3.2 NodeType2Text

Converts an instance of NODETYPE into the corresponding textual representation.

*/

string
NestedList::NodeType2Text( NodeType type )
{
  switch (type)
  {
    case NoAtom:     return ("NoAtom");
    case IntType:    return ("IntType");
    case RealType:   return ("RealType");
    case BoolType:   return ("BoolType");
    case StringType: return ("StringType");
    case SymbolType: return ("SymbolType");
    case TextType:   return ("TextType");
    default:         return ("Type unknown");
  }
}

/*
4 Construction

4.1 TheEmptyList

*/

ListExpr
NestedList::TheEmptyList()
{
  return (0);
}

/*
4.2 Cons

*/

ListExpr
NestedList::Cons( const ListExpr left, const ListExpr right )
{
  assert( !IsAtom( right ) );

  Cardinal newNode = nodeTable->EmptySlot();
  NodeRecord& newNodeRef = (*nodeTable)[newNode];
  newNodeRef.nodeType = NoAtom;
  newNodeRef.n.left     = left;
  newNodeRef.n.right    = right;
  newNodeRef.n.isRoot   = true;
  if ( !(IsAtom( left ) || IsEmpty( left )) )
  {
    (*nodeTable)[left].n.isRoot = false;
  }
  if ( !IsEmpty( right ) )
  {
    (*nodeTable)[right].n.isRoot = false;
  }
  return (newNode);
}

/*
4.3 Append

*/

ListExpr
NestedList::Append ( const ListExpr lastElem,
                     const ListExpr newSon )
{
  assert( !IsAtom( lastElem ) && IsEmpty( (*nodeTable)[lastElem].n.right ) );

  Cardinal newNode = nodeTable->EmptySlot();
  NodeRecord& newNodeRef = (*nodeTable)[newNode];

  (*nodeTable)[lastElem].n.right = newNode;
  newNodeRef.nodeType = NoAtom;
  newNodeRef.n.left     = newSon;
  newNodeRef.n.right    = 0;
  newNodeRef.n.isRoot   = false;

  if ( !(IsAtom( newSon ) || IsEmpty( newSon )) )
  {
    (*nodeTable)[newSon].n.isRoot = false;
  }
  return (newNode);
}

/* 
4.4 Destroy

*/

void
NestedList::Destroy ( const ListExpr list )
{
  if ( !IsEmpty( list ) && !IsAtom( list ) && (*nodeTable)[list].n.isRoot )
  {
    if ( doDestroy )
    {
      DestroyRecursive( list );
    }
  }
}

/* 
Internal procedure *DestroyRecursive* 

*/

void
NestedList::DestroyRecursive ( const ListExpr list )
{
  Cardinal elem, nextElem;

  if ( !IsEmpty( list ) )
  {
    if ( nodeTable->IsValid( list ) )
    {       /* node not yet destroyed */
      NodeRecord& listRef = (*nodeTable)[list];
      switch (listRef.nodeType)
      {
        case NoAtom:
          DestroyRecursive( listRef.n.left );
          DestroyRecursive( listRef.n.right );
          nodeTable->Remove( list );
          break;
        case IntType:
        case RealType:
        case BoolType:
          intTable->Remove( listRef.a.index);
          nodeTable->Remove( list );
          break;
        case StringType:
        case SymbolType:
          if ( listRef.s.first  != 0 )
          {
            stringTable->Remove( listRef.s.first );
          }
          if ( listRef.s.second != 0 )
          {
            stringTable->Remove( listRef.s.second );
          }
          if ( listRef.s.third  != 0 )
          {
            stringTable->Remove( listRef.s.third );
          }
          nodeTable->Remove( list );
          break;
        case TextType:
          elem = listRef.t.start;
          while ( elem != 0 )
          {
            nextElem = (*textTable)[elem].next;
            textTable->Remove( elem );
            elem = nextElem;
          }
          nodeTable->Remove( list );
          break;
      }
    }
  }
}

/*
4.5 OneElemList, .., SixElemList

*/

ListExpr
NestedList::OneElemList( const ListExpr elem1 )
{
  return (Cons( elem1, TheEmptyList() ));
}

ListExpr
NestedList::TwoElemList( const ListExpr elem1, const ListExpr elem2 )
{
  return (Cons( elem1,
                Cons( elem2, TheEmptyList () ) ));
}

ListExpr
NestedList::ThreeElemList( const ListExpr elem1, const ListExpr elem2,
                           const ListExpr elem3 )
{
  return (Cons( elem1,
                Cons( elem2,
                      Cons( elem3, TheEmptyList () ) ) ));
}

ListExpr
NestedList::FourElemList( const ListExpr elem1, const ListExpr elem2,
                          const ListExpr elem3, const ListExpr elem4 )
{
  return (Cons( elem1,
                Cons( elem2,
                      Cons( elem3,
                            Cons( elem4, TheEmptyList () ) ) ) ));
}

ListExpr
NestedList::FiveElemList( const ListExpr elem1, const ListExpr elem2,
                          const ListExpr elem3, const ListExpr elem4,
                          const ListExpr elem5 )
{
  return (Cons( elem1,
                Cons( elem2,
                      Cons( elem3,
                            Cons( elem4,
                                  Cons( elem5, TheEmptyList () ) ) ) ) ));
}

ListExpr
NestedList::SixElemList( const ListExpr elem1, const ListExpr elem2,
                         const ListExpr elem3, const ListExpr elem4,
                         const ListExpr elem5, const ListExpr elem6 )
{
  return (Cons( elem1,
                Cons( elem2,
                      Cons( elem3,
                            Cons( elem4,
                                  Cons( elem5,
                                        Cons( elem6, TheEmptyList () ) ) ) ) ) ));
}

/* 
5 Simple Tests

5.1 IsEmpty, IsAtom, EndOfList, ListLength

*/

bool
NestedList::IsEmpty( const ListExpr list )
{
  return (list == 0);
}


bool
NestedList::IsAtom( const ListExpr list )
{
  if ( IsEmpty( list ) )
  {
     return (false);
  }
  else
  { 
    return ((*nodeTable)[list].nodeType != NoAtom);   
  }
}

bool
NestedList::EndOfList( ListExpr list )
{
  if ( IsEmpty( list ) )
  { 
    return (false);
  }
  else if ( IsAtom( list ) )
  {
    return (false);
  } 
  else
  {
    return (Rest( list ) == 0);
  }
}

int
NestedList::ListLength( ListExpr list )
{
/*
~list~ may be any list expression. Returns the number of elements, if it is 
a list, and -1, if it is an atom.

*/
  int result = 0;

  if ( IsAtom( list ) )
  { 
    result = -1;
  }
  else
  {
    while ( !IsEmpty( list ) )
    {
      result++;
      list = Rest( list );
    }
  }
  return (result);
}

int
NestedList::ExprLength( ListExpr expr )
{
/* 
Reads a list expression ~expr~ and counts the number ~length~ of
subexpressions.

*/
  int length = 0;
  while (!IsAtom( expr ) && !IsEmpty( expr ))
  {
    length++;
    expr = Rest( expr );
  }
  if ( IsAtom( expr ) )
  {
    length++;
  }
  return (length);
}

bool
NestedList::Equal( const ListExpr list1, const ListExpr list2 )
{
/*
Test for deep equality of two nested lists

*/
  if ( IsEmpty( list1 ) && IsEmpty( list2 ) )
  {
    return (true);
  }
  else if ( IsEmpty( list1 ) || IsEmpty( list2 ) )
  {
    return (false);
  }
  else if ( IsAtom( list1 ) && IsAtom( list2 ) )
  {
    if ( AtomType( list1 ) == AtomType( list2 ) )
    {
      if ( AtomType( list1 ) == IntType )
      {
        return (IntValue( list1 ) == IntValue( list2 ));
      }
      else if ( AtomType( list1 ) == BoolType )
      {
        return (BoolValue( list1 ) == BoolValue( list2 ));
      }
      else if ( AtomType( list1 ) == RealType )
      {
        return (RealValue( list1 ) == RealValue( list2 ));
      }
      else if ( AtomType( list1 ) == SymbolType )
      {
        return (SymbolValue( list1 ) == SymbolValue( list2 ));
      }
      else if ( AtomType( list1 ) == StringType )
      {
        return (StringValue( list1 ) == StringValue( list2 ));
      }
      else
      {
        return (false);
      }
    }
    else
    {
      return (false);
    }
  }  
  else if ( !IsAtom( list1 ) && !IsAtom( list2 ) )
  {
    return (Equal( First( list1 ), First( list2 ) ) &&
            Equal( Rest( list1 ), Rest( list2 ) ));
  }
  else
  {
    return (false);
  }
}


const ListExpr
NestedList::CopyList(const ListExpr list, const NestedList* target)
{
   return CopyRecursive(list, target);
}


const ListExpr
NestedList::CopyRecursive(const ListExpr list, const NestedList* target)
{ 
   ListExpr newnode = 0; 
   CTable<NodeRecord> *pnT_target = target->nodeTable; 

   if (list == 0) {
     return 0;
   }
   
   NodeRecord& listRef = (*nodeTable)[list];

      switch (listRef.nodeType)
      {
        case NoAtom: {
	  ListExpr left = 0, right = 0; 
			     
	  if (listRef.n.left) {
          left = CopyRecursive( listRef.n.left, target );
	  }
	  if (listRef.n.right) {
          right = CopyRecursive( listRef.n.right, target );
	  }
	  
	  // create a new node record in the target list and link it
	  // with the left and right subtrees.
	  newnode = pnT_target->Add( listRef );
	  (*pnT_target)[newnode].n.left = left;
	  (*pnT_target)[newnode].n.right = right;
          
	  break; }
        case IntType:
        case RealType:
        case BoolType: {
	  ListExpr newconst = 0;
			       
	  // create a new constant and node records in the target list. 
	  newconst = target->intTable->Add( (*intTable)[listRef.a.index] );
	  newnode = pnT_target->Add( listRef );
	  (*pnT_target)[newnode].a.index = newconst;
	  }
          break;
        case StringType:
        case SymbolType: {
          CTable<StringRecord>  *psT_target = target->stringTable;
	  ListExpr newstr = 0;
	  StringRecord sRec;

	  newnode = pnT_target->Add(listRef);
	  if (listRef.s.third) {
	    sRec = (*stringTable)[listRef.s.third];
	    newstr = psT_target->Add(sRec);
	    (*pnT_target)[newnode].s.third = newstr;
	  }
	  if (listRef.s.second) {
	    sRec = (*stringTable)[listRef.s.second];
	    newstr = psT_target->Add(sRec);
	    (*pnT_target)[newnode].s.second = newstr;
	  }
	  if (listRef.s.first) {
	    sRec = (*stringTable)[listRef.s.first];
	    newstr = psT_target->Add(sRec);
	    (*pnT_target)[newnode].s.first = newstr;
	  }

          break; }
        case TextType: {

	  ListExpr newtext=0, newstart=0, tnext=0, tlast=0;
	  TextRecord tRec;

	  newnode = pnT_target->Add(listRef);
	  tRec = (*textTable)[listRef.t.start];
	  newstart = target->textTable->Add(tRec);
	  (*pnT_target)[newnode].t.start = newstart;
	  (*pnT_target)[newnode].t.last = newstart;

	  ListExpr tCurrent = listRef.t.start;
          while ( (tnext = (*textTable)[tCurrent].next) != 0 )
	  {
	    tRec = (*textTable)[tnext];
	    newtext = target->textTable->Add(tRec);
	    (*target->textTable)[newtext].next = 0;
	    tlast = (*pnT_target)[newnode].t.last;
	    (*target->textTable)[tlast].next = newtext;
	    (*pnT_target)[newnode].t.last = newtext;
	    tCurrent = tnext;
          }

          break; }
	default:
	  break;
      }
   return newnode;
}

bool
NestedList::IsEqual( const ListExpr atom, const string& str,
                     const bool caseSensitive )
{
/*
returns TRUE if ~atom~ is a symbol atom and has the same value as ~str~.

*/
  if ( IsAtom( atom ) && (AtomType( atom ) == SymbolType) )
  {
    if ( caseSensitive )
    {
      return (SymbolValue( atom ) == str);
    }
    else
    {
      string aStr = SymbolValue( atom );
      string bStr = str;
      transform( aStr.begin(), aStr.end(), aStr.begin(), ToUpperProperFunction );
      transform( bStr.begin(), bStr.end(), bStr.begin(), ToUpperProperFunction );
      return (aStr == bStr);
    }
  }
  else
  {
    return (false);
  }
}

/*
6 Scanning and Parsing

6.1 ReadFromFile

*/

bool
NestedList::ReadFromFile ( const string& fileName, ListExpr& list )
{
  bool success = false;
  list = 0;
  ifstream ifile( fileName.c_str() );
  if ( ifile )
  {
    NLParser* nlParser = new NLParser( this, &ifile );
    if ( nlParser->yyparse() == 0 )
    {
      list = nlParser->GetNestedList();
      success = true;
    }
    delete nlParser;
    ifile.close();
  }
  return (success);
}

/*
Internal procedure *BoolToStr*

*/

string
NestedList::BoolToStr( const bool boolValue )
{
  return (boolValue ? "TRUE" : "FALSE");
}

/*
Internal procedure *WriteAtom*

*/

void
NestedList::WriteAtom( const ListExpr atom, bool toScreen )
{
  NodeRecord& nodeRef = (*nodeTable)[atom];
  switch (nodeRef.nodeType)
  {
    case IntType:
      *outStream << IntValue( atom );
      break;
    case RealType:
      {
        ostringstream os;
        os << setprecision(16) << RealValue( atom );
        if ( os.str().find( '.' ) == string::npos )
        {
          os << ".0";
        }
        *outStream << os.str();
      }
      break;
    case BoolType:
      *outStream << BoolToStr( BoolValue( atom ) );
      break;
    case StringType:
      *outStream << "\"" << StringValue( atom ) << "\"";
      break;
    case SymbolType:
      *outStream << SymbolValue( atom );
      break;
    case TextType:
      {
        if ( !toScreen )
        {
          *outStream << "<text>";
        }
        const int textFragmentLength = 80;
        string textFragment;
        int textLength = TextLength( atom );
        TextScan textScan   = CreateTextScan( atom );
        while (textFragmentLength < textLength)
        {
          textFragment = "";
          GetText( textScan, textFragmentLength, textFragment );
          *outStream << textFragment;
          textLength -= textFragmentLength;
        }
        /* The remaining text fits into array text_fragment); */
        textFragment = "";
        GetText ( textScan, textLength, textFragment );
        *outStream << textFragment;
        DestroyTextScan ( textScan );
        if ( !toScreen )
        {
          *outStream << "</text--->";
        }
      }
      break;
    default:  /* Error */
      *outStream << "WriteAtom: NodeType out of range!" << endl;
      break;
  }
}

/*
Internal procedure *WriteList*

*/
bool
NestedList::WriteList( ListExpr list, const int level,
                       const bool afterList, const bool toScreen )
{
/*
Write a list ~List~ indented by 4 blanks for each ~Level~ of nesting. Atoms are written sequentially into a line as long as they do not follow a nonempty list (~AfterList~ = FALSE). Otherwise (~AfterList~ = TRUE) they are also indented to the current level. Text atoms are written indented into a new line like lists. If ~ToScreen~ is true, then text atoms are written without their brackets.

*/
  int i;
  bool after;

  if ( IsEmpty( list ) )
  { 
    *outStream << endl;
    for ( i = 1; i <= level; i++ )
    {
      *outStream << "    ";
    }
    *outStream << "()"; 
    return (afterList);
  }
  else if ( IsAtom( list ))
  {
    if ( afterList || (AtomType( list) == TextType ) )
    { 
      *outStream << endl;
      for ( i = 1; i <= level; i++ )
      {
        *outStream << "    ";
      }
    }
    WriteAtom( list, toScreen );
    return (afterList);
  }
  else
  {
    *outStream << endl;
    for ( i = 1; i <= level; i++ )
    {
      *outStream << "    ";
    }
    *outStream << "(";
    after = WriteList( First( list ), level+1, false, toScreen );
    while (!IsEmpty( Rest( list ) ))
    {
      list = Rest( list );
      *outStream << " ";
      after = WriteList( First( list ), level+1, after, toScreen );
    }
    *outStream << ")"; 
    return (true);
  }
}

/*
6.2 WriteToFile

*/

bool
NestedList::WriteToFile( const string& fileName, const ListExpr list )
{
  assert( !IsAtom( list ) );
  bool ok = false;
  ofstream outFile( fileName.c_str() );
  if ( outFile )
  {
    outStream = &outFile;
//    outStream.rdbuf( outFile.rdbuf() );
    /* bool afterList = */ WriteList( list, 0, false, false );
    *outStream << endl;
    outFile.close();
    outStream = 0;
    ok = true;
  }
  return (ok);
}

/*
6.3 ReadFromString

*/

bool
NestedList::ReadFromString( const string& nlChars, ListExpr& list )
{
/* 
   The job of this procedure is to read a string and to convert it to a nested
   list.

   Short: String [->] NestedList 

*/

  bool success = false;
  list = 0;
  istringstream inString( nlChars );
  if ( inString )
  {
    NLParser* nlParser = new NLParser( this, &inString );
    if ( nlParser->yyparse() == 0 )
    {
      list = nlParser->GetNestedList();
      success = true;
    }
    delete nlParser;
  }
  return (success);
}

/*
6.4 WriteToString

*/

bool
NestedList::WriteToString( string& nlChars, const ListExpr list )
{
//  assert( !IsAtom( list ) );
  nlChars = "";
  return (WriteToStringLocal( nlChars, list ));
//  bool ok = WriteToStringLocal( nlChars, list );
//  return (ok);
}

/*
Internal procedure *WriteToStringLocal* 

*/

bool
NestedList::WriteToStringLocal( string& nlChars, ListExpr list )
{
/* 
Error Handling in this procedure: If anything goes wrong, the execution of the 
function is finished immediately, and the function result is ~false~, if the
string nlChars could not be written properly, or if there was something wrong
within the structure of the list, otherwise, the function result is ~true~.

*/
  ostringstream os;
  if ( IsEmpty( list ) )
  {
    nlChars += "()";
  }
  else if ( IsAtom ( list ) )
  {
    NodeRecord& nodeRef = (*nodeTable)[list];
    switch ( nodeRef.nodeType )
    {
      case IntType:
        os << IntValue( list );
        nlChars += os.str();
        break;
      case RealType:
        os << setprecision(16) << RealValue( list );
        if ( os.str().find( '.' ) == string::npos )
        {
          os << ".0";
        }
        nlChars += os.str();
        break;
      case BoolType:
        nlChars += BoolToStr( BoolValue( list ) );
        break;
      case StringType:
        nlChars += "\"" + StringValue( list ) + "\"";
        break;
      case SymbolType:
        nlChars += SymbolValue( list );
        break;
      case TextType:
        {
          TextScan textScan = CreateTextScan( list );
          string tempText = "";
          GetText( textScan, TextLength( list ), tempText ); 
          DestroyTextScan( textScan );
          nlChars += "<text>" + tempText + "</text--->";
        }
        break;
      default:
        return (2);
    }
  }
  else
  { /* List is neither empty nor an atom */
    nlChars += "(";
    bool ok = WriteToStringLocal( nlChars, First( list ) );
    if ( !ok )
    {
      return (ok);
    }
    while ( !IsEmpty( Rest( list ) ) )
    {
      nlChars += " ";
      list = Rest( list );
      ok = WriteToStringLocal( nlChars, First( list ) );
      if ( !ok )
      {
        return (ok);
      }
    }
    nlChars += ")";
  }

  /* This point in the function is only reached if no error occurred before. */
  return (true);
}

/*
6.5 WriteListExpr

*/

void
NestedList::WriteListExpr( const ListExpr list, ostream& ostr )
/*
Write ~list~ indented by level to standard output.

*/
{
  outStream = &ostr;
  /* bool after = */ WriteList( list, 0, false, true );
  outStream = 0;
}

void
NestedList::WriteListExpr( const ListExpr list )
/*
Write ~list~ indented by level to standard output.

*/
{
  outStream = &cout;
  /* bool after = */ WriteList( list, 0, false, true );
  outStream = 0;
}

/* 
7 Traversal

7.1 First

*/
ListExpr
NestedList::First( const ListExpr list )
{
  assert( !IsEmpty( list ) && !IsAtom( list ) );
  return ((*nodeTable)[list].n.left);    
}

/*
7.2 Rest

*/

ListExpr
NestedList::Rest( const ListExpr list )
{
  assert( !IsEmpty( list ) && !IsAtom( list ) );
  return ((*nodeTable)[list].n.right);
}

/*
7.3 NthElement

*/

ListExpr
NestedList::NthElement( const Cardinal n,
                        const Cardinal initialN,
                        const ListExpr list )
{
/*
Return the ~n~-th element of ~List~. Since this is used recursively,
~initialN~ keeps the argument of the first call to be able to give intelligent
error messages. Must hence be called externally always with ~n~ = ~initialN~.

*Precondition* (for initial call): ~List~ is not empty and no atom and has at
least ~N~ elements.

*/
  if ( IsEmpty( list ) )
  {
    cerr << endl << "NestedList-ERROR. *********" << endl
         << "Element " << initialN << " selected from a list with "
         << initialN - n << " elements!" << endl;
  }
  else if ( IsAtom( list ) )
  {
    cerr << endl << "NestedList-ERROR. *********" << endl
         << "Element " << initialN << " selected from an atom!" << endl;
  }
  else if ( n == 0 )
  {
    cerr << endl << "NestedList-ERROR. *********" << endl
         << "Function 'NthElement' called with Zero!" << endl;
  }
  else if ( n == 1)
  {
    return (First( list ));
  }
  else
  {
    return (NthElement( n-1, initialN, Rest( list ) ));
  }
  return (0);
}

/*
7.4 Second

*/

ListExpr
NestedList::Second( const ListExpr list)
{
  return (NthElement( 2, 2, list ));
}

/*
7.5 Third

*/

ListExpr
NestedList::Third( const ListExpr list )
{
  return (NthElement( 3, 3, list ));
}

/*
7.6 Fourth

*/

ListExpr
NestedList::Fourth( const ListExpr list )
{
  return (NthElement( 4, 4, list ));
}

/*
7.7 Fifth

*/

ListExpr
NestedList::Fifth( const ListExpr list )
{
  return (NthElement( 5, 5, list ));
}

/*
7.8 Sixth

*/

ListExpr
NestedList::Sixth( const ListExpr list )
{
  return (NthElement( 6, 6, list ));
}

/*
8 Construction of Atoms

8.1 IntAtom

*/

ListExpr
NestedList::IntAtom( const long  value )
{
  Cardinal newNode    = nodeTable->EmptySlot();
  Cardinal newIntNode = intTable->EmptySlot();

  NodeRecord& newNodeRef = (*nodeTable)[newNode];
  newNodeRef.nodeType = IntType;
  newNodeRef.a.index  = newIntNode;

  (*intTable)[newIntNode].intValue = value;

  return (newNode);
}

/*
8.2 RealAtom

*/

ListExpr
NestedList::RealAtom( const double value )
{
  Cardinal newNode    = nodeTable->EmptySlot();
  Cardinal newIntNode = intTable->EmptySlot();

  NodeRecord& newNodeRef = (*nodeTable)[newNode];
  newNodeRef.nodeType = RealType;
  newNodeRef.a.index  = newIntNode;

  (*intTable)[newIntNode].realValue = value;

  return (newNode);
}

/*
8.3 BoolAtom

*/

ListExpr
NestedList::BoolAtom( const bool value )
{
  Cardinal newNode    = nodeTable->EmptySlot();
  Cardinal newIntNode = intTable->EmptySlot();

  NodeRecord& newNodeRef = (*nodeTable)[newNode];
  newNodeRef.nodeType = BoolType;
  newNodeRef.a.index  = newIntNode;

  (*intTable)[newIntNode].boolValue = value;

  return (newNode);
}

/*
8.4 StringAtom

*/

ListExpr
NestedList::StringAtom( const string& value )
{
  assert( value.length() <= 3*STRINGSIZE );
  Cardinal newNode       = nodeTable->EmptySlot();
  NodeRecord& newNodeRef = (*nodeTable)[newNode];
  newNodeRef.nodeType    = StringType;
  newNodeRef.s.strLength = ((value.length() <= 3*STRINGSIZE) ? value.length() : 3*STRINGSIZE);

  if ( value.length() <= STRINGSIZE )
  {
    newNodeRef.s.first  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.first].field, STRINGSIZE );
    newNodeRef.s.second = TheEmptyList();
    newNodeRef.s.third  = TheEmptyList();
  }
  else if ( value.length() <= 2*STRINGSIZE )
  {
    newNodeRef.s.first  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.first].field, STRINGSIZE );
    newNodeRef.s.second = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.second].field, STRINGSIZE, STRINGSIZE );
    newNodeRef.s.third  = TheEmptyList();
  }
  else
  {
    newNodeRef.s.first  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.first].field, STRINGSIZE );
    newNodeRef.s.second = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.second].field, STRINGSIZE, STRINGSIZE );
    newNodeRef.s.third  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.third].field, STRINGSIZE, 2*STRINGSIZE );
  }
  return (newNode);
}

/*
8.5 SymbolAtom

*/

ListExpr
NestedList::SymbolAtom( const string& value )
{
//  assert( value.length() <= 3*STRINGSIZE );
  Cardinal newNode       = nodeTable->EmptySlot();
  NodeRecord& newNodeRef = (*nodeTable)[newNode];
  newNodeRef.nodeType    = SymbolType;
  newNodeRef.s.strLength = ((value.length() <= 3*STRINGSIZE) ? value.length() : 3*STRINGSIZE);

  if ( value.length() <= STRINGSIZE )
  {
    newNodeRef.s.first  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.first].field, STRINGSIZE );
    newNodeRef.s.second = TheEmptyList();
    newNodeRef.s.third  = TheEmptyList();
  }
  else if ( value.length() <= 2*STRINGSIZE )
  {
    newNodeRef.s.first  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.first].field, STRINGSIZE );
    newNodeRef.s.second = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.second].field, STRINGSIZE, STRINGSIZE );
    newNodeRef.s.third  = TheEmptyList();
  }
  else
  {
    newNodeRef.s.first  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.first].field, STRINGSIZE );
    newNodeRef.s.second = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.second].field, STRINGSIZE, STRINGSIZE );
    newNodeRef.s.third  = stringTable->EmptySlot();
    value.copy( (*stringTable)[newNodeRef.s.third].field, STRINGSIZE, 2*STRINGSIZE );
  }
  return (newNode);
}

/*
8.6 TextAtom

*/

ListExpr
NestedList::TextAtom()
{
  Cardinal newNode     = nodeTable->EmptySlot();

  NodeRecord& newNodeRef = (*nodeTable)[newNode];
  newNodeRef.nodeType = TextType;
  newNodeRef.t.start  = textTable->EmptySlot();
  newNodeRef.t.last   = newNodeRef.t.start;
  newNodeRef.t.length = 0;

  TextRecord& newTextNodeRef = (*textTable)[newNodeRef.t.start];
  newTextNodeRef.next  = TheEmptyList();
  memset( newTextNodeRef.field, 0, MaxFragmentLength );

  return (newNode);
}

/* 
8.7 AppendText

*/

void
NestedList::AppendText( const ListExpr atom,
                        const string& textBuffer )
{
  assert( AtomType( atom ) == TextType );

  NodeRecord& atomContent  = (*nodeTable)[atom];
  TextRecord* lastFragment = &(*textTable)[atomContent.t.last];

  Cardinal lastFragmentLength, emptyFragmentLength; 
  for ( lastFragmentLength = 0; 
        lastFragmentLength < MaxFragmentLength &&
        lastFragment->field[lastFragmentLength];
        lastFragmentLength++ );
  emptyFragmentLength = MaxFragmentLength - lastFragmentLength;

/* 
There are two cases: Either there is enough space in the current fragment
for NoChars, or there is not enough space. The last fragment of a text atom
is never filled completely (with MaxFragmentLength characters), but it is
empty or it is filled with up to MaxFragmentLength-1 characters.

*/
  
  Cardinal textLength = textBuffer.length();
  Cardinal textStart  = 0;
  if ( (lastFragmentLength + textLength) <= MaxFragmentLength )
  {
    /* There is enough space in the last fragment. (Case 1) */
    /* --> Append new text.                                 */
    textBuffer.copy( lastFragment->field + lastFragmentLength,
                     emptyFragmentLength );
    atomContent.t.length += textLength;
  }
  else
  {
    ListExpr newFragmentID;
    TextRecord* newFragment;

    /* There is not enough space in the last fragment. (Case 2) */
    /* Steps 1/2: Fill current fragment completely.             */

    textBuffer.copy( lastFragment->field + lastFragmentLength,
                     emptyFragmentLength );
    atomContent.t.length += emptyFragmentLength;
    textLength         -= emptyFragmentLength;
    textStart          += emptyFragmentLength;

    /* Step 2/2: Create new (empty) fragment and append it, then recursive   */
    /*           procedure call.                                             */

    while ( textLength > 0 )
    {
      newFragmentID = textTable->EmptySlot ();
      newFragment   = &(*textTable)[newFragmentID];
      memset( newFragment->field, 0, MaxFragmentLength );
      newFragment->next  = TheEmptyList();

      lastFragment = &(*textTable)[atomContent.t.last];
      lastFragment->next  = newFragmentID;

      emptyFragmentLength = (textLength <= MaxFragmentLength) ? textLength : MaxFragmentLength;
      textBuffer.copy( newFragment->field, MaxFragmentLength, textStart );
      textLength -= emptyFragmentLength;
      textStart  += emptyFragmentLength;
      atomContent.t.last    = newFragmentID;
      atomContent.t.length += emptyFragmentLength;
    }
  }
}

/*
9 Reading Atoms

9.1 IntValue

*/

long
NestedList::IntValue( const ListExpr atom )
{
  assert( AtomType( atom ) == IntType );
  return ((*intTable)[(*nodeTable)[atom].a.index].intValue);
}

/*
9.2 RealValue

*/

double
NestedList::RealValue( const ListExpr atom )
{
  assert( AtomType( atom ) == RealType );
  return ((*intTable)[(*nodeTable)[atom].a.index].realValue);
}

/*
9.3 BoolValue

*/

bool
NestedList::BoolValue( const ListExpr atom )
{
  assert( AtomType( atom ) == BoolType );
  return ((*intTable)[(*nodeTable)[atom].a.index].boolValue);
}

/*
9.4 StringValue

*/

string
NestedList::StringValue( const ListExpr atom )
{
  assert( AtomType( atom ) == StringType );
  NodeRecord& atomRef = (*nodeTable)[atom];
  string outString( "" );
  if ( atomRef.s.strLength <= STRINGSIZE )
  {
    outString.append( (*stringTable)[atomRef.s.first].field, atomRef.s.strLength );
  }
  else if ( atomRef.s.strLength <= 2*STRINGSIZE )
  {
    outString.append( (*stringTable)[atomRef.s.first].field,  STRINGSIZE );
    outString.append( (*stringTable)[atomRef.s.second].field, atomRef.s.strLength - STRINGSIZE );
  }
  else
  {
    outString.append( (*stringTable)[atomRef.s.first].field,  STRINGSIZE );
    outString.append( (*stringTable)[atomRef.s.second].field, STRINGSIZE );
    outString.append( (*stringTable)[atomRef.s.third].field,  atomRef.s.strLength - 2*STRINGSIZE );
  }
  return (outString);
}

/*
9.5 SymbolValue

*/

string
NestedList::SymbolValue( const ListExpr atom )
{
  assert( AtomType( atom ) == SymbolType );
  NodeRecord& atomRef = (*nodeTable)[atom];
  string outString( "" );
  if ( atomRef.s.strLength <= STRINGSIZE )
  {
    outString.append( (*stringTable)[atomRef.s.first].field, atomRef.s.strLength );
  }
  else if ( atomRef.s.strLength <= 2*STRINGSIZE )
  {
    outString.append( (*stringTable)[atomRef.s.first].field,  STRINGSIZE );
    outString.append( (*stringTable)[atomRef.s.second].field, atomRef.s.strLength - STRINGSIZE );
  }
  else
  {
    outString.append( (*stringTable)[atomRef.s.first].field,  STRINGSIZE );
    outString.append( (*stringTable)[atomRef.s.second].field, STRINGSIZE );
    outString.append( (*stringTable)[atomRef.s.third].field,  atomRef.s.strLength - 2*STRINGSIZE );
  }
  return (outString);
}

/*
9.6 Treatment of Text Atoms

9.6.1 CreateTextScan

*/

TextScan
NestedList::CreateTextScan (const ListExpr atom )
{
  assert( AtomType( atom ) == TextType );

  TextScan textScan = new TextScanRecord;
  textScan->currentFragment = (*nodeTable)[atom].t.start;
  textScan->currentPosition = 0;

  return (textScan);
}

/*
9.6.2 GetText

*/

void
NestedList::GetText ( TextScan       textScan,
                      const Cardinal noChars,
                      string&        textBuffer )
{
  TextRecord& fragment = (*textTable)[textScan->currentFragment];
  Cardinal fragmentLength, fragmentRestLength;

  if ( fragment.next == 0 )
  {
    for ( fragmentLength = 0; 
          fragmentLength < MaxFragmentLength &&
          fragment.field[fragmentLength];
          fragmentLength++ );
  }
  else
  {
    fragmentLength = MaxFragmentLength;
  }
  fragmentRestLength = fragmentLength - textScan->currentPosition;
  
  if ( noChars <= fragmentRestLength )
  {
    /* Enough text in the current fragment */
    textBuffer.append( &fragment.field[textScan->currentPosition], noChars );

    /* Increase values of Scan and Pos */
    textScan->currentPosition += noChars;

    /* Treat a special case: */
    if ( textScan->currentPosition >= MaxFragmentLength )
    {
      textScan->currentFragment = fragment.next;  
      textScan->currentPosition = 0;
    }
  }
  else
  {
    // Not enough text in the current fragment
    // Return as much text of the current fragment as possible
    textBuffer.append( &fragment.field[textScan->currentPosition], fragmentRestLength );

    /* Increase values of Scan and Pos */
    textScan->currentPosition += fragmentRestLength;

    if ( textScan->currentPosition >= MaxFragmentLength )
    {
      textScan->currentFragment = fragment.next;  
      textScan->currentPosition = 0;
    }
    
    /* Get more text if there are more fragments */
    if ( fragment.next != 0 )
    {
      /* Recursive call (with scan pointing to next fragment) */
      GetText( textScan, noChars - fragmentRestLength, textBuffer );
    }
  }
}

/*
9.6.3 TextLength

*/

Cardinal
NestedList::TextLength ( const ListExpr textAtom )
{
  assert( AtomType( textAtom ) == TextType );
  return ((*nodeTable)[textAtom].t.length);
}

/*
9.6.4 EndOfText

*/

bool
NestedList::EndOfText( const TextScan textScan )
{
  if ( textScan->currentFragment == 0 )
  {
    return (true);
  }
  else
  {
    TextRecord& fragment = (*textTable)[textScan->currentFragment];
    Cardinal fragmentLength;
    for ( fragmentLength = 0; 
          fragmentLength < MaxFragmentLength &&
          fragment.field[fragmentLength];
          fragmentLength++ );
    return ((fragment.next == 0) && 
            (textScan->currentPosition >= fragmentLength));
  }
}

/*
9.6.5 DestroyTextScan

*/

void
NestedList::DestroyTextScan( TextScan& textScan )
{
  if ( textScan != 0 )
  {
    delete textScan;
    textScan = 0;
  }
}

/* 
10 AtomType

*/

NodeType
NestedList::AtomType (const ListExpr atom )
{
  if ( !IsAtom( atom ) )
  {
    return (NoAtom);
  }
  else
  {
    return ((*nodeTable)[atom].nodeType);
  } 
}

/*
11 ReportVectorSizes
*/

const string
NestedList::reportVectorSizes() {

  ostringstream report;
 
  report << "List memory (slots/used): " 
	 << "nodes " << nodeTable->Size() << "/" << nodeTable->NoEntries() << ", " 
 	 << "int " << intTable->Size() << "/" << intTable->NoEntries() << ", " 
	 << "str " << stringTable->Size() << "/" <<  stringTable->NoEntries() << ". "
         << endl << "Total " 
	 << nodeTable->totalMemory() + intTable->totalMemory() + stringTable->totalMemory()
         << " Bytes." << endl;
	 
  return report.str();

}


