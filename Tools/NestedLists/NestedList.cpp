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

October 22, 1996 RHG Corrected EndOfList, Nth-Element, and Second ... Sixth.
Made operations ~ListLength~ and ~WriteListExpr~ available. Changed internal
procedure ~WriteList~ so that atoms following a list are written indented to
the same level as a preceding list. This affects all output of lists.

November 14, 1996 RHG Removed ~SetValid~ commands.

November 18, 1996 RHG Changed Import from ~CatalogManager~ to ~Ctable~ 
in component ~ListsAndTables~.

November 22, 1996 RHG Added a final ~WriteLn~ in ~WriteToFile~.

January 20, 1997 RHG Corrected procedure ~AtomType~. Removed error message 
from ~ReadFromString~.

September 26, 1997 Stefan Dieker Corrected wrong order of CTable calls in
~Append~, ~AppendText~, and all procedures creating atoms.

May 8, 1998 RHG Changed the way how text atoms are written to the screen. 
Affects procedures ~WriteAtom~, ~WriteLists~.

October 12, 1998 Stefan Dieker. The name of the temporary file created by 
~ReadFromString~ is now unique for each call of ~ReadFromString~.

December 1, 2001 Ulrich Telle Port to C++

November 28, 2002 M. Spiekermann coding of reportVectorSizes().

December 05, 2002 M. Spiekermann methods InitializeListMemory() and 
CopyList/CopyRecursive implemented.

April 22, 2003 V. Almeida changed the methods CopyList and Destroy to use 
iteration instead of recursion.

Jan - May 2003, M. Spiekermann. Get() and Put() Methods of CTable.h were 
used to allow switching between persistent and in memory implementations of 
NL without writing special code for both implementations.
Uncomment the precompiler directive \#define CTABLE\_PERSISTENT for support 
of big nested lists.
Currently it is not possible to mix both alternatives.

December 2003, M. Spiekermann. A new method GetNextText has been introduced 
and and the implementation of Text2String was changed in order to use 
stringstreams.

February 2004, M. Spiekermann. Reading of binary encoded lists was implemented.
 WriteAtom changed;
only 48 bytes of text atoms are displayed on screen now.

July 2004, M. Spiekermann. A big constant was replaced by UINT\_MAX and PD 
syntax corrected.

August 2004, M. Spiekermann. A char pointer in ~StringSymbolValue~ has been 
changed to an array of char.

1 Introduction

A nested list is represented by four stable tables called ~Nodes~, ~Ints~,
~Strings~, and ~Texts~.

2 Imports

*/


#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits.h>

#include "CharTransform.h"
#include "NLParser.h"
#include "WinUnix.h"
#include "LogMsg.h"
#include "Counter.h"
#include "StringUtils.h"


using namespace std;


// used in PagedArray.h
unsigned int FileCtr = 0;

/*
3 Preliminaries

3.1 Constants, Types \& Variables

Definitions implied by convention:

*/


size_t NestedList::NLinstance = 0;

NodeRecord::NodeRecord() :
    nodeType(BoolType),
    isRoot(false),
    strLength(0),
    inLine(0),
    references(1){

 }


ostream& operator<<(ostream& out, const NodeRecord& nr){
    out << "nr: " << NestedList::NodeType2Text(nr.nodeType) << "  " ;
    if(nr.nodeType==NoAtom){
      out << (nr.isRoot?"root":"non-root");
    } 
    if(nr.nodeType==StringType || nr.nodeType==SymbolType){
      out << "  strlength("<< (int)nr.strLength<<")  ";
      out << (nr.inLine ?"inline" : "non-inline") ;
    }
    out << " refs = " << nr.references << "  ";
    if(nr.nodeType == NoAtom){
       out << "left(" << nr.n.left << ") "
           << ", right(" << nr.n.right << ")";
    } else if(nr.nodeType==IntType){
      out << "value = " <<nr.a.value.intValue; 
    } else if (nr.nodeType==RealType){
      out << "value = " << nr.a.value.realValue;
    } else if(nr.nodeType==BoolType){
      out << "value = " << (nr.a.value.boolValue?"true":"false");
    } else if(  (nr.nodeType==StringType)
              ||(nr.nodeType==SymbolType)){
      if(!nr.inLine){
        out << "first("<<nr.s.first<<")";
        out << "field -> stringTable";
      } else {
         out << "field =" << string(nr.s.field,nr.strLength);
      }
    } else if( nr.nodeType==TextType){
      out << "start(" << nr.t.start<<")";
      out << "last(" << nr.t.last<<")";
    } else {
      out << "unknown nodetype" ; 
    }
    return out;
}



template<class Array>
Cardinal emptySlot(Array& a, set<Cardinal>& freeSlots){
   if(freeSlots.empty()){
     return a.EmptySlot();
   } 
   set<Cardinal>::iterator it = freeSlots.begin();
   Cardinal res = *it;
   freeSlots.erase(it);
   return res;
}




/*
This constant defines whether the ~Destroy~ method really destroys a
nested list. Only if ~doDestroy~ is ~true~, nested lists are destroyed.

The ~Destroy~ method was never used. The concept for freeing list memory has
changed and therefore an implementation of this method is no longer useful.
However, the code for destroying in this file will not be compiled but unless
you comment out the line below.

*/



#ifdef THREAD_SAFE
   boost::mutex smtx;
#endif

NestedList::NestedList( )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::mutex> guard1(smtx);
#endif
  assert( sizeof(float) == 4);
  // How can we convert a N byte floating point representation
  // to a M byte representation?

  stringTable = 0;
  nodeTable = 0;
  textTable = 0;

  instanceNo = NLinstance++;
  stringstream ss;
  ss << "TMP_NESTED_LIST_" << WinUnix::getpid() << "_" << instanceNo;
  basename = ss.str();
  setMem(1024, 512, 512);
  initializeListMemory();

}


NestedList::~NestedList()
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
   DeleteListMemory();
}

void
NestedList::DeleteListMemory()
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
   //cerr << "DeleteListMem this = " << (void*)this << endl;
   //cerr << "stringTable = " << (void*) stringTable << endl;
   //cerr << "nodeTable = " << (void*) nodeTable << endl;
   //cerr << "textTable = " << (void*) textTable << endl;
   
   if(stringTable){
     delete stringTable;
     stringTable = 0;
   }
   if(nodeTable){
      delete nodeTable;
      nodeTable = 0;
   }
   if(textTable){
     delete textTable;
     textTable = 0;
   }
   freeNodeSlots.clear();
   freeStringSlots.clear();
   freeTextSlots.clear();
}


void
NestedList::setMem( Cardinal nodeMem, Cardinal strMem, Cardinal textMem)
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  nodeEntries = nodeMem * 1024 / sizeof(NodeRecord);
  stringEntries = nodeMem * 1024 / sizeof(StringRecord);
  textEntries = nodeMem * 1024 / sizeof(TextRecord);
}


void
NestedList::initializeListMemory()
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
   //cout << endl << "### NestedList::initializeListMemory" << endl;
   DeleteListMemory(); //cleans also free sets

   nodeTable   = BigArray<NodeRecord>::newInstance(basename+"_nodes",
                                                   nodeEntries,true);
   stringTable = BigArray<StringRecord>::newInstance(basename+"_strings",
                                                     nodeEntries,true);
   textTable   = BigArray<TextRecord>::newInstance(basename+"_texts",
                                                   nodeEntries,true);

   typeError = SymbolAtom("typeerror");
   errorList = OneElemList( SymbolAtom("ERRORS") );
}



string
NestedList::SizeOfStructs() {
  stringstream sizes;
  sizes << "NodeRecord: " << sizeof(NodeRecord) << endl;
  sizes << "TextRecord: " << sizeof(TextRecord) << endl;
  sizes << "StringRec.: " << sizeof(StringRecord) << endl;

  return sizes.str();
}


/*

3.1 PrintTableTexts

PrintTableTexts displays the contents of the Table 'Texts' on the screen.
The procedure was very helpful during the test phase of the module, and is
not used anywhere in the current version of the module.

*/

void
NestedList::PrintTableTexts() const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  Cardinal i, numEntries;

  numEntries = textTable->NoEntries();
  cout << "/////////////////////////////////////////////////////////////"
       << endl;
  for ( i = 1; i <= numEntries; i++ )
  {
    if ( textTable->IsValid( i ) )
    {
      const TextRecord& entry = (*textTable)[i];
      cout << endl << "Index: " << i;
      cout << endl << "Field: " << entry.field;
      cout << endl << "Next:  " << entry.next << endl;
    }
  }
  cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
       << endl;
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



ListExpr
NestedList::First( const ListExpr list ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( !IsEmpty( list ) && !IsAtom( list ) );
  return ((*nodeTable)[list].n.left);
};


ListExpr
NestedList::Rest( const ListExpr list ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( !IsEmpty( list ) && !IsAtom( list ) );
  return ((*nodeTable)[list].n.right);
};

ListExpr
NestedList::End( ListExpr list ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  ListExpr last = Empty();
  while ( !IsEmpty(list) ) {
     last = list;
     list = Rest(list);
  }
  return last;
}

/*
4.2 Cons

*/

ListExpr
NestedList::Cons( const ListExpr left, const ListExpr right, 
                  bool incRefs)
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( !IsAtom( right ) );

  Cardinal newNode = emptySlot(*nodeTable, freeNodeSlots);

  // concatenate nodes
  NodeRecord tmpNodeVal;
  nodeTable->Get(newNode, tmpNodeVal);
  tmpNodeVal.nodeType = NoAtom;
  tmpNodeVal.n.left     = left;
  tmpNodeVal.n.right    = right;
  tmpNodeVal.isRoot = 1;
  tmpNodeVal.references = 1;
  (*nodeTable).Put(newNode, tmpNodeVal);

  if ( ! IsEmpty( left ) )
  {
    nodeTable->Get(left, tmpNodeVal);
    tmpNodeVal.isRoot = 0;
    if(incRefs){
       tmpNodeVal.references++;
    }
    (*nodeTable).Put(left, tmpNodeVal);
  }
  if ( !IsEmpty( right ) )
  {
    (*nodeTable).Get(right, tmpNodeVal);
    if(incRefs){
      tmpNodeVal.references++;
    }
    tmpNodeVal.isRoot = 0;
    (*nodeTable).Put(right, tmpNodeVal);
  }

  return (newNode);
}

/*

4.3 Append

*/

ListExpr
NestedList::Append ( const ListExpr lastElem,
                     const ListExpr newSon,
                     bool incRef )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
        assert( EndOfList(lastElem) );

  NodeRecord lastElemNodeRec;
  (*nodeTable).Get(lastElem, lastElemNodeRec);

  Cardinal newNode = emptySlot(*nodeTable, freeNodeSlots);

  NodeRecord newNodeRec;
  (*nodeTable).Get(newNode, newNodeRec);

  lastElemNodeRec.n.right = newNode;
  newNodeRec.nodeType = NoAtom;
  newNodeRec.n.left = newSon;
  newNodeRec.n.right = 0;
  newNodeRec.isRoot = 0;
  newNodeRec.references = 1;

  (*nodeTable).Put(lastElem, lastElemNodeRec);
  (*nodeTable).Put(newNode, newNodeRec);

  if ( !IsEmpty( newSon ) ) {

    NodeRecord newSonRec;
    (*nodeTable).Get(newSon, newSonRec);
    newSonRec.isRoot = 0;
    if(incRef){
      newSonRec.references++;
    }
    (*nodeTable).Put(newSon, newSonRec);
  }
  return (newNode);
}

/*
4.4 Destroy


The ~Destroy~ method was never used. The concept for freeing list memory has
changed and therefore an implementation of this method is no longer useful.
The code in this file will not compile but may be used as start point for
an implementation if ever needed. Look at the Copy List method which works
similarly.

*/



void NestedList::DestroyRec(ListExpr& list){

  if(IsEmpty(list)){
    return;
  }

  switch( AtomType(list) ) {

  case BoolType     : 
  case IntType      : 
  case RealType     : {
                      NodeRecord root; 
                      nodeTable->Get(list, root);
                      assert(root.references>0);
                      root.references--;
                      if(root.references>0){
                         nodeTable->Put(list,root);
                         return;
                      }
                      freeNodeSlots.insert(list);
                      }
                      break;

  case SymbolType   : 
  case StringType   : { 
                      NodeRecord root; 
                      nodeTable->Get(list, root);
                      assert(root.references>0);
                      root.references--;
                      if(root.references>0){
                         nodeTable->Put(list,root);
                         return;
                      }
                      freeNodeSlots.insert(list);
                      if(!root.inLine){
                        Cardinal next = root.s.first;
                        root.s.first = 0;
                        while(next){
                           StringRecord sr;
                           stringTable->Get(next,sr);
                           freeStringSlots.insert(next);
                           Cardinal last = next;
                           next = sr.next;
                           sr.next = 0;
                           stringTable->Put(last, sr);
                        }
                      }  
                      }
                      break;


  case TextType     : {
                      NodeRecord root; 
                      nodeTable->Get(list, root);
                      assert(root.references>0);
                      root.references--;
                      if(root.references>0){
                         nodeTable->Put(list,root);
                         return;
                      }
                      Cardinal next = root.t.start;
                      root.t.start = 0;
                      freeNodeSlots.insert(list);
                      while(next){
                         TextRecord tr;
                         textTable->Get(next,tr);
                         freeTextSlots.insert(next);
                         Cardinal last = next;
                         next = tr.next;
                         tr.next = 0;
                         textTable->Put(last, tr);
                      }
                      }
                      break;

  case NoAtom        : {
                         NodeRecord root; 
                         nodeTable->Get(list, root);
                         assert(root.references>0);
                         root.references--;
                         if(root.references>0){
                            nodeTable->Put(list,root);
                            return;
                         }
                         freeNodeSlots.insert(list);
                         // destroy first
                         DestroyRec(root.n.left);
                         // destroy rest
                         Cardinal scan = root.n.right;
                         while(scan){
                            nodeTable->Get(scan, root);
                            root.references--;
                            if(root.references==0){
                                freeNodeSlots.insert(scan);
                            } 
                            DestroyRec(root.n.left);
                            scan = root.n.right; 
                         }
                       }
                       break;
   default: assert(false); // invalid atom type

  }
}



void
NestedList::Destroy(ListExpr& list )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  
  DestroyRec(list); 
  list = 0;
}

void NestedList::IncReferences(ListExpr& list){
  if(list){
     NodeRecord node;
     nodeTable->Get(list, node);
     assert(node.references>0); 
     node.references++;
     nodeTable->Put(list,node);
  }
}



/*
5 Simple Tests

5.1 ListLength

*/

int
NestedList::ListLength( ListExpr list ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
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

bool
NestedList::HasLength( ListExpr list, const int n ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  if ( IsAtom(list) ){
    return false;
  }

  
  int result = 0;
  while ( !IsEmpty( list ) && (result < n) )
  {
    result++;
    list = Rest( list );
  }
  return (result == n) && (IsEmpty(list));
}

/*
~HasMinLength~

Returns true iff the Given Listexpr contains at least n elements.

*/
bool
NestedList::HasMinLength( ListExpr list, const int n ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  if ( IsAtom(list) ){
    return false;
  }

  
  int result = 0;
  while ( !IsEmpty( list ) && (result < n) )
  {
    result++;
    list = Rest( list );
  }
  return (result == n) ;
}


int
NestedList::ExprLength( ListExpr expr ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
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

/*
enum NodesStacked { None, Left, Right, Both };
struct CopyStackRecord
{
  CopyStackRecord( NodeRecord nr, NodesStacked ns = None, ListExpr le = 0 ):
   nr( nr ),
   ns( ns ),
   le( le )
   {}

  NodeRecord nr;
  NodesStacked ns;
  ListExpr le;
};

*/


ListExpr
NestedList::SimpleCopy(const ListExpr list, NestedList* target) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  //cout << "NestedList::SimpleCopy" << endl;
  stringstream ss;
  ListExpr temp = list;
  WriteBinaryTo(temp, ss);
  ListExpr result = TheEmptyList();
  target->ReadBinaryFrom(ss, result);

  return result;
}


bool
NestedList::IsEqual( const ListExpr atom, const string& str,
                     const bool caseSensitive                ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
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
      transform( aStr.begin(), aStr.end(),
                 aStr.begin(), ToUpperProperFunction );
      transform( bStr.begin(), bStr.end(),
                 bStr.begin(), ToUpperProperFunction );
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
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  bool success = false;
  list = 0;
  ifstream ifile( fileName.c_str() );

  if ( ifile )
  {
    NLParser* nlParser = new NLParser( this, &ifile );
    if ( nlParser->parse() == 0 )
    {
      list = nlParser->GetNestedList();
      success = true;
    }
    delete nlParser;
    ifile.close();
  }
  else
  {
    cmsg.error() << "Could not access file '" << fileName << "'" << endl;
    cmsg.send();
  }
  return (success);
}


/*
Internal procedure *WriteAtom*

*/

void
NestedList::WriteAtom( const ListExpr atom,
                       bool toScreen, ostream& ostr ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  switch ((*nodeTable)[atom].nodeType)
  {
    case IntType:
      ostr << IntValue( atom );
      break;
    case RealType:
      {
        ostringstream os;
        //os << showpoint ;
        //os << scientific ;
        os << setprecision(16) << RealValue( atom );
        if ( (os.str().find( '.' ) == string::npos)  &&
             (os.str().find('e') == string::npos))
        {
          os << ".0";
        }
        ostr << os.str();
      }
      break;
    case BoolType:
      ostr << BoolToStr( BoolValue( atom ) );
      break;
    case StringType:{
       string s(StringValue(atom));
      s = stringutils::replaceAll(
              stringutils::replaceAll(s,"\\","\\\\"),"\"","\\\"");
      ostr << "\"" << s << "\"";
      break;
    }
    case SymbolType:
      ostr << SymbolValue( atom );
      break;
    case TextType:
      {
        if ( !toScreen )
        {
          ostr << "'";
          string textFragment = "";
          TextScanInfo info;
          while ( GetNextText(atom, textFragment, 1024, info) ) {
           ostr << transformText2Outtext(textFragment);
          }
          ostr << "'";

        } else {

          static const size_t len=48;
          string textFragment = "";
          TextScan textScan = CreateTextScan( atom );
          GetText( textScan, len, textFragment );
          ostr << "'" << textFragment << "'";
          if ( textFragment.length() > len ) {
            ostr << " ... (text atom truncated after "
                       << len << " bytes)";
          }
          DestroyTextScan ( textScan );

        }
      }
      break;
    default:  /* Error */
      ostr << "WriteAtom: NodeType out of range!" << endl;
      break;
  }
}

/*
Internal procedure *WriteList*

*/
bool
NestedList::WriteList( ListExpr list,
                       const int level,
                       const bool afterList,
                       const bool toScreen,
                       ostream& os,
                       const int offset /*=4*/ ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
/*
Write a list ~List~ indented by 4 blanks for each ~Level~ of nesting. Atoms
are written sequentially into a line as long as they do not follow a
nonempty list (~AfterList~ = FALSE). Otherwise (~AfterList~ = TRUE) they are
also indented to the current level. Text atoms are written indented into a
new line like lists. If ~ToScreen~ is true, then text atoms are written
without their brackets.

*/
  const int indent = level*offset;
  bool after = false;


  if ( IsEmpty( list ) )
  {
    os << endl << tab(indent) << "()";
    return (afterList);
  }
  else if ( IsAtom( list ))
  {
    if ( afterList || (AtomType( list) == TextType ) )
    {
      os << endl << tab(indent);
    }
    WriteAtom( list, toScreen, os );
    return (afterList);
  }
  else
  {
    os << endl << tab(indent) << "(";
    after = WriteList( First( list ), level+1, false, toScreen, os, offset );
    while (!IsEmpty( Rest( list ) ))
    {
      list = Rest( list );
      os << " ";
      after = WriteList( First( list ), level+1, after, toScreen, os, offset );
    }
    os << ")";
    return (true);
  }
}




/*
6.2 WriteToFile

*/

bool
NestedList::WriteToFile( const string& fileName, const ListExpr list ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( !IsAtom( list ) );
  bool ok = false;
  ofstream outFile( fileName.c_str() );
  if ( outFile )
  {
    WriteList( list, 0, false, false, outFile );
    outFile << endl;
    outFile.close();
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
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
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
    if ( nlParser->parse() == 0 )
    {
      list = nlParser->GetNestedList();
      success = true;
    }
    delete nlParser;
  }
  return (success);
}

/*
6.4 String Conversion

6.4.1 ToString

*/

string
NestedList::ToString( const ListExpr list ) const
{
   string listStr;
   int Success = WriteToString(listStr, list);
   assert( Success );
   return listStr;
}

/*
6.4.2 WriteToString

*/

bool
NestedList::WriteToString( string& nlChars, const ListExpr list ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  bool ok =false;
  ostringstream nlos;

  if ( (ok=WriteToStringLocal( nlos, list  ))) {
    nlChars = nlos.str();
  } else {
    nlChars = "";
  }
  return ok;
}


/*
6.4.2 WriteStringTo

Write a list in its textual representation into an ostream object

*/

bool
NestedList::WriteStringTo(  const ListExpr list, ostream& os ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  bool ok =false;
  ok=WriteToStringLocal( os, list  );
  return ok;
}

/*
Internal procedure *WriteToStringLocal*

*/

bool
NestedList::WriteToStringLocal( ostream& nlChars, ListExpr list ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
/*
Error Handling in this procedure: If anything goes wrong, the execution of the
function is finished immediately, and the function result is ~false~, if the
string nlChars could not be written properly, or if there was something wrong
within the structure of the list, otherwise, the function result is ~true~.

*/
  ostringstream os;
  if ( IsEmpty( list ) )
  {
    nlChars << "()";
  }
  else if ( IsAtom ( list ) )
  {
    switch ( (*nodeTable)[list].nodeType )
    {
      case IntType:
        os << IntValue( list );
        nlChars << os.str();
        break;
      case RealType:
        os << setprecision(16) << RealValue( list );
        if ( (os.str().find( '.' ) == string::npos )&&
             (os.str().find('e') == string::npos))
        {
          os << ".0";
        }
        nlChars << os.str();
        break;
      case BoolType:
        nlChars << BoolToStr( BoolValue( list ) );
        break;
      case StringType:{
        string s = StringValue(list);
        s = stringutils::replaceAll(
               stringutils::replaceAll(s,"\\","\\\\"),"\"","\\\"");
        nlChars << ("\"" + s + "\"");
        break;
      }
      case SymbolType:
        nlChars << SymbolValue( list );
        break;
      case TextType:
          // ToDo replace tempText with a loop which
          // writes small fragments into nlChars
        {
          /*
          TextScan textScan = CreateTextScan( list );
          string tempText = "";

          GetText( textScan, TextLength( list ), tempText );
          DestroyTextScan( textScan );
          nlChars << ("<text>" + tempText + "</text--->");
          */

          if ( RTFlag::isActive("NL:TextLength") ) {
             cerr << "list, TextLength( list ): "
                  << list << "," << TextLength( list ) << endl;
          }

          nlChars << "'";

          string textFragment = "";
          TextScanInfo info;
          while ( GetNextText(list, textFragment, 1024, info) ) {
            textFragment = transformText2Outtext(textFragment);
             nlChars << textFragment;
          }
          nlChars << "'";

        }
        break;
      default:
        return (2);
    }
  }
  else
  { /* List is neither empty nor an atom */
    nlChars << "(";
    bool ok = WriteToStringLocal( nlChars, First( list ) );

    if ( !ok ) {
      return (ok);
    }

    while ( !IsEmpty( Rest( list ) ) )
    {
      nlChars << " ";
      list = Rest( list );
      ok = WriteToStringLocal( nlChars, First( list ) );

      if ( !ok ) {
        return (ok);
      }
    }
    nlChars << ")";
  }

  /* This point in the function is only reached if no error occurred before. */
  return (true);
}


/*
6.4 WriteBinaryTo: Write a list in binary encoded format into
    an output stream

*/


bool
NestedList::WriteBinaryTo(const ListExpr list, ostream& os) const {

#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( os.good() );

  const byte v[7] = {'b','n','l',0,1,0,2};
  os.write((char*)v,7);
  bool ok = WriteBinaryRec(list, os);
  os.flush();
  return ok;

}

/*
6.4 ReadBinaryFrom: Reconstruct a list from
    an input stream containing a binary encoded list

*/

bool
NestedList::ReadBinaryFrom(istream& in, ListExpr& list) {

#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( in.good() );

  char version[8] = {0,0,0,0,0,0,0,0};
  in.read(version,7);

  string vStr = string(version);
  if ( vStr.substr(0,3) != "bnl" ) {
    cerr << "Error: Input stream is not a binary encoded nested list." << endl;
    list = 0;
    return false;
  }
  // version number check ommitted
  unsigned long pos = 0;
  bool ok = ReadBinaryRec(list, in, pos);
  return ok;
}


/*

6.4 hton (host to network) converts a 'long' value with
    LSB (little endian) byte order into the network representation
    MSB (big endian). This computation should be independent of the
    hosts internal representation of a long value.

*/

void 
NestedList::hton(long value, char* buffer) const {
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
   static const int n = sizeof(long);
   for (int i=0; i<n; i++) {
     buffer[n-1-i] = (byte) (value & 255);
     value = value >> 8;
   }
}



// Type IDs for binary encoded lists
static const byte BIN_LONGLIST = 0;
static const byte BIN_INTEGER  = 1;
static const byte BIN_REAL = 2;
static const byte BIN_BOOLEAN = 3;
static const byte BIN_LONGSTRING = 4;
static const byte BIN_LONGSYMBOL = 5;
static const byte BIN_LONGTEXT = 6;
static const byte BIN_LIST = 10;
static const byte BIN_SHORTLIST = 11;
static const byte BIN_SHORTINT  = 12;
static const byte BIN_BYTE = 13;
static const byte BIN_STRING = 14;
static const byte BIN_SHORTSTRING = 15;
static const byte BIN_SYMBOL= 16;
static const byte BIN_SHORTSYMBOL = 17;
static const byte BIN_TEXT = 18;
static const byte BIN_SHORTTEXT = 19;
static const byte BIN_DOUBLE = 20;


/*
6.4 GetBinaryType: Determine the binary type ID of a list value
    depending on the size of the list atom.

*/

byte
NestedList::GetBinaryType(const ListExpr list) const {
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  switch( AtomType(list) ) {

  case BoolType     : return  BIN_BOOLEAN;
  case IntType      : { long v = IntValue(list);
                        if(v>=-128 && v<=127)
                           return BIN_BYTE;
                        if(v>=-32768 && v<=32767)
                           return BIN_SHORTINT;
                        return BIN_INTEGER;
                      }
  case RealType     : return BIN_DOUBLE;
  case SymbolType   : { int len = SymbolValue(list).length();
                        if(len<256)
                           return BIN_SHORTSYMBOL;
                        if(len<65536)
                           return BIN_SYMBOL;
                        return BIN_LONGSYMBOL;
                      }
  case StringType   : { int len = StringValue(list).length();
                        if(len<256)
                           return BIN_SHORTSTRING;
                        if(len<65536)
                           return BIN_STRING;
                        return BIN_LONGSTRING;
                      }
  case TextType     : { int len = TextLength(list);
                        if(len<256)
                           return BIN_SHORTTEXT;
                        if(len<65536)
                           return BIN_TEXT;
                        return BIN_LONGTEXT;
                       }
  case NoAtom        : { if(HasMinLength(list,256)){
                            return BIN_LONGLIST;
                         } else {
                            return BIN_SHORTLIST;
                         }
                       }
  default : return (byte) 255;

  }
}

/*
6.4 ReadInt: This is the counterpart to the function hton.
    A 4 byte (signed) integer value is created.

*/


int32_t
NestedList::ReadInt(istream& in, const int len /*= 4*/) const{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif

  static const bool debug = RTFlag::isActive("NL:BinaryListDebug");
  char buffer[4] = { 0, 0, 0, 0 };
  int32_t result = 0;

  in.read(buffer,len);

  if( debug ) {
    cerr << "Hex-Value: ";
    for (int i=0; i<len; i++) {
      cerr << setiosflags(ios::showbase | ios::hex)
           << (unsigned char) buffer[i] << " ";
    }
  }

  for (int i=0; i<len; i++) {
    result = result << 8;
    result += (unsigned char) 255 & buffer[i];
  }

  if( debug ) {
    cerr << "   =>  Int-Value: " << setiosflags(ios::dec)
         << result << endl;
  }

  return result;
}


/*
6.4 ReadShort: This is the counterpart to the function hton.
    A 2 byte (signed) integer value is created.

*/


int32_t
NestedList::ReadShort(istream& in) const {
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  int32_t result = ReadInt(in, 2);
  if((result & 0x8000)  == 0x8000) {
     //result is negativ, adjust first bytes
     result |= ((~0l) ^ 0xffff) ;
  }
  return result;
}


/*
6.4 swap: Convert a ~N~ byte buffer into reverse order. This is needed to
    convert float/double values between little endian an big endian 
    representation.

*/

inline void
NestedList::swap(char* buffer, const int n) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  for (int i = 0; i < n/2; i++)
  {
    char c = buffer[n-1-i];
    buffer[n-1-i] = buffer[i];
    buffer[i] = c;
  }
}


/*

6.4 ReadString: This function allocates temporarily
    a character buffer, reads a given number of characters into it and
    converts the buffer into a string object.

*/

void
NestedList::ReadString( istream& in,
                        string& outStr, unsigned long length) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  char* strBuf = new char[length+1];
  in.read(strBuf, length);
  strBuf[length]=0;
  outStr = string(strBuf);
  delete [] strBuf;
}


bool
NestedList::ReadBinarySubLists( ListExpr& LE, istream& in,
                                unsigned long length, 
                                unsigned long& pos   )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  if(length==0) {
     LE = TheEmptyList();
     return true;
  }

  ListExpr subList = 0;
  bool ok = ReadBinaryRec(subList, in, pos);
  if(!ok) // error in reading sublist
     return false;
  LE = OneElemList(subList,false);

  ListExpr Last = LE;
  ListExpr Next = 0;
  for(unsigned int i=1; i<length; i++){
      bool ok = ReadBinaryRec(Next, in, pos);
      if(!ok) // error in reading sublist
          return false;
      Last = Append(Last,Next,false);
  }
  return true;

}


/*
6.4 ReadBinaryRec: This recursive function reconstructs a list from
    a stream containing binary encoded list data.

*/
bool
NestedList::ReadBinaryRec(ListExpr& result, istream& in, unsigned long& pos) {
  static const bool debug = RTFlag::isActive("NL:BinaryListDebug");
  unsigned long len = 0;
  string str = "";

  byte typeId = 255 & in.get();
  pos++;

  if( debug ) {
    cerr << "TypeId: " << (unsigned int) (255 & typeId) << endl;
  }

  switch( typeId ){

      case BIN_BOOLEAN        : { result =  BoolAtom( in.get() ? true : false );
                                  pos++;
                                  return true;
                                }
      case BIN_BYTE           : { char c = 0;
                                  in.read(&c,1);
                                  result = IntAtom( c );
                                  pos++;
                                  return true;
                                }
      case BIN_SHORTINT       : { result =  IntAtom( ReadShort(in) );
                                  pos += 2;
                                  return true;
                                }
      case BIN_INTEGER        : { result =  IntAtom( ReadInt(in) );
                                  pos +=4;
                                  return true;
                                }
      case BIN_REAL           : {
                                 float fval = 0;
                                 char* fp = (char*) &fval;
                                 in.read(fp, 4);
                                 pos +=4;
                                 if ( WinUnix::isLittleEndian() ) {
                                    swap(fp,sizeof(float));
                                 }
                                 result = RealAtom( fval );
                                 return true;
                                }
      case BIN_DOUBLE           : {
                                 double dval = 0;
                                 char* dp = (char*) &dval;
                                 in.read(dp, 8);
                                 pos += 8;
                                 if ( WinUnix::isLittleEndian() ) {
                                    swap(dp,sizeof(double));
                                 }
                                 result = RealAtom( dval );
                                 return true;
                                }

      case BIN_SHORTSTRING    : { len = 255 & in.get();
                                  ReadString(in, str, len);
                                  pos = pos + 1 + len;
                                  result = StringAtom( str );
                                  return true;
                                }
      case BIN_STRING         : { len = 65535 & ReadShort(in);
                                  ReadString(in, str, len);
                                  pos = pos + 2 + len;
                                  result = StringAtom( str );
                                  return true;
                                }
      case BIN_LONGSTRING     : { len = UINT_MAX & ReadInt(in);
                                  ReadString(in, str, len);
                                  pos = pos + 4 + len;
                                  result = StringAtom( str );
                                  return true;
                                }
      case BIN_SHORTSYMBOL    : { len = 255 & in.get();
                                  ReadString(in, str, len);
                                  pos = pos + 1 + len;
                                  result =  SymbolAtom( str );
                                  return true;
                                }
      case BIN_SYMBOL         : { len = 65535 & ReadShort(in);
                                  ReadString(in, str, len);
                                  pos = pos + 2 + len;
                                  result =  SymbolAtom( str );
                                  return true;
                                }
      case BIN_LONGSYMBOL     : { len = UINT_MAX & ReadInt(in);
                                  ReadString(in, str, len);
                                  pos = pos + 4 + len;
                                  result =  SymbolAtom( str );
                                  return true;
                                }
      case BIN_SHORTTEXT      : { len = 255 & in.get();
                                  ReadString(in, str, len);
                                  pos = pos + 1 + len;
                                  ListExpr text = TextAtom();
                                  AppendText(text, str );
                                  result = text;
                                  return true;
                                }
      case BIN_TEXT           : { len = 65535 & ReadShort(in);
                                  ReadString(in, str, len);
                                  pos = pos + 2 + len;
                                  ListExpr text = TextAtom();
                                  AppendText(text, str );
                                  result = text;
                                  return true;
                                }
      case BIN_LONGTEXT       : { len = UINT_MAX & ReadInt(in);
                                  ReadString(in, str, len);
                                  pos = pos + 4 + len;
                                  ListExpr text = TextAtom();
                                  AppendText(text, str );
                                  result =  text;
                                  return true;
                                }

      case BIN_SHORTLIST      : { len = 255 & in.get();
                                  pos++;
                                  ListExpr LE = 0;
  if( debug ) {
    cerr << "len: " << len << endl;
  }
                                  bool ok = ReadBinarySubLists(LE, in, len,pos);
                                  if (!ok) {
                                    result = 0;
                                    return false;
                                  } else {
                                    result = LE;
                                    return true;
                                  }
                                 }

      case BIN_LIST           : { len = 65535 & ReadShort(in);
                                  pos += 2;
                                  ListExpr LE = 0;
                                  bool ok = ReadBinarySubLists(LE, in,len,pos);
                                  if (!ok) {
                                    result = 0;
                                    return false;
                                  } else {
                                    result = LE;
                                    return true;
                                  }
                                 }

      case BIN_LONGLIST      : {  len = 4294967295u & ReadInt(in); // 2^32-1
                                  pos += 4;
                                  ListExpr LE = 0;
                                  bool ok = ReadBinarySubLists(LE,in,len,pos);
                                  if (!ok) {
                                    cout << "Error in ReadBinarySubLists(...)!"
                                         << endl;
                                    result = 0;
                                    return false;
                                  } else {
                                    result = LE;
                                    return true;
                                  }
                                }


      default      : {
                        cerr << "Error: Unknown binary list type ID: "
                             << (unsigned int) typeId
                             << " at position " << pos << endl;
                        //cerr << "Last read string: " << str << endl;
                        return false;
                     }
  }


}


/*

6.4 WriteBinaryRec: This recursive function  writes
    lists in binary format to the output stream.

*/

bool
NestedList::WriteBinaryRec(ListExpr list, ostream& os) const {
  static const bool debug = RTFlag::isActive("NL:BinaryListDebug");
  static const int floatLen=sizeof(float);
  static const int doubleLen=sizeof(double);
  static const int longLen=sizeof(long);


  unsigned long strlen = 0;
  unsigned int len = 0;

  os.flush();
  assert( os.good() );

  byte typeId = GetBinaryType(list);
  os << typeId;

      switch( typeId ) {

          case BIN_BOOLEAN:   {
                           bool b = BoolValue(list);
                           byte value = (byte) (b?1:0);
                           os << (byte) value;
                           return true;
          }
          case BIN_BYTE:
          case BIN_SHORTINT:
          case BIN_INTEGER:
          {
                           long value = IntValue(list);
                           char pv[sizeof(long)];
                           hton(value,pv);
                           if (typeId == BIN_BYTE) {
                             len = 1;
                           } else {
                             len = (typeId == BIN_SHORTINT) ? 2 : 4;
                           }
                           os.write(pv+longLen-len,len);
                           return true;
          }
          case BIN_REAL:   {
                           float value = RealValue(list);
                           char val[floatLen];
                           memcpy( (void*) val, (void*) &value, floatLen );

                           if ( WinUnix::isLittleEndian() ) {
                             swap(val,sizeof(float));
                           }
                           os.write(val,floatLen);
                           return true;
          }
          case BIN_DOUBLE:   {
                double value = RealValue(list);
          char val[doubleLen];
          memcpy( (void*) val, (void*) &value, doubleLen );

                           if ( WinUnix::isLittleEndian() ) {
                             swap(val,sizeof(double));
                           }
           os.write(val,doubleLen);
                           return true;
          }
          case BIN_SHORTSTRING:
          case BIN_STRING:
          case BIN_LONGSTRING: {
                           string value = StringValue(list);
                           strlen = value.length();
                           char pv[sizeof(long)];
                           hton(strlen,pv);
                           if (typeId == BIN_SHORTSTRING) {
                             len = 1;
                           } else {
                             len = (typeId == BIN_STRING) ? 2 : 4;
                           }
                           os.write(pv+longLen-len,len);
                           os << value;
                           return true;
          }
          case BIN_SHORTSYMBOL:
          case BIN_SYMBOL:
          case BIN_LONGSYMBOL: {
                           string value = SymbolValue(list);
                           strlen = value.length();
                           char pv[sizeof(long)];
                           hton(strlen,pv);
                           if (typeId == BIN_SHORTSYMBOL) {
                             len = 1;
                           } else {
                             len = (typeId == BIN_SYMBOL) ? 2 : 4;
                           }
                           os.write(pv+longLen-len,len);
                           os << value;
                           return true;
          }
          case BIN_SHORTTEXT:
          case BIN_TEXT:
          case BIN_LONGTEXT:   {
                           strlen = TextLength(list);
                           char pv[sizeof(long)];
                           hton(strlen,pv);
                           if (typeId == BIN_SHORTTEXT) {
                             len = 1;
                           } else {
                             len = (typeId == BIN_TEXT) ? 2 : 4;
                           }
                           os.write(pv+longLen-len,len);
                           string value="";
                           TextScanInfo info;
                           while ( GetNextText(list, value, 1024, info) ) {
                              os << value;
                           }
                           return true;
          }

          case BIN_SHORTLIST:
          case BIN_LIST:
          case BIN_LONGLIST: {

  if (debug) {
    cerr << "TypeId: " << (unsigned int) (255 & typeId) << endl;
    cerr << "ListLength: " << ListLength(list) << endl;
    cerr << "sizeof(long): " << sizeof(long) << endl;
   }                       
                           char pv[sizeof(long)];
                           hton(ListLength(list),pv);
                           if (typeId == BIN_SHORTLIST) {
                             len = 1;
                           } else {
                             len = (typeId == BIN_LIST) ? 2 : 4;
                           }

  if (debug) {
    cerr << "len: " << len << endl;
  }

                           int offset=longLen-len;
  if (debug) {
    cerr << "offset: " << offset << endl;
  }
                           os.write(pv+offset,len);

                           while( !IsEmpty(list) ){

                             if( !WriteBinaryRec( First(list), os ) ) {
                               // error in writing sublist
                               return false;
                             }
                             list=Rest(list);
                           }
                           return true;
          }
          default: return false;
      }

}



/*
6.5 WriteListExpr

Write ~list~ indented by level to standard output.

*/

void
NestedList::WriteListExpr( const ListExpr list,
                           ostream& ostr  /*= cout*/,
                           const int offset /*= 4*/   )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  WriteList( list, 0, false, true, ostr, offset );
}

/*
7 Traversal

7.3 NthElement

*/

ListExpr
NestedList::NthElement( const Cardinal n,
                        const Cardinal initialN,
                        const ListExpr list      ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
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
    abort();
  }
  else if ( IsAtom( list ) )
  {
    cerr << endl << "NestedList-ERROR. *********" << endl
         << "Element " << initialN << " selected from an atom!" << endl;
    abort();
  }
  else if ( n == 0 )
  {
    cerr << endl << "NestedList-ERROR. *********" << endl
         << "Function 'NthElement' called with Zero!" << endl;
    abort();
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
8 Construction of Atoms

8.1 IntAtom

*/

ListExpr
NestedList::IntAtom( const long  value )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  Cardinal newNode = emptySlot(*nodeTable, freeNodeSlots);
  NodeRecord newNodeRec;
  nodeTable->Get(newNode, newNodeRec);
  newNodeRec.nodeType = IntType;
  newNodeRec.a.value.intValue = value;
  newNodeRec.references = 1;
  nodeTable->Put(newNode, newNodeRec);

  return (newNode);
}

/*
8.2 RealAtom

*/

ListExpr
NestedList::RealAtom( const double value )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  Cardinal newNode = emptySlot(*nodeTable, freeNodeSlots);

  NodeRecord newNodeRec;
  nodeTable->Get(newNode, newNodeRec);
  newNodeRec.nodeType = RealType;
  newNodeRec.a.value.realValue = value;
  newNodeRec.references = 1;
  nodeTable->Put(newNode, newNodeRec);

  return (newNode);
}

/*
8.3 BoolAtom

*/

ListExpr
NestedList::BoolAtom( const bool value )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  Cardinal newNode = emptySlot(*nodeTable, freeNodeSlots);

  NodeRecord newNodeRec;
  nodeTable->Get(newNode, newNodeRec);
  newNodeRec.nodeType = BoolType;
  newNodeRec.a.value.boolValue = value;
  newNodeRec.references = 1;
  nodeTable->Put(newNode, newNodeRec);

  return (newNode);
}

/*
8.4 StringAtom

*/

ListExpr
NestedList::StringAtom( const string& value, bool isString /*=true*/ )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  int strLen = value.length();
  assert( strLen <= MAX_STRINGSIZE );

  NodeRecord newNodeRec;

  Cardinal newNode = emptySlot(*nodeTable, freeNodeSlots);

  nodeTable->Get(newNode, newNodeRec);

  // store length and type information
  newNodeRec.strLength = strLen;
  if ( isString ) {
    newNodeRec.nodeType = StringType;
  } else {
    newNodeRec.nodeType = SymbolType;
  }
  newNodeRec.references = 1;

  if ( strLen <= STRING_INTERNAL_SIZE ) {

    // store string directly in the node record
    newNodeRec.inLine = 1;
    value.copy( newNodeRec.s.field, strLen );
    nodeTable->Put(newNode, newNodeRec);
    return newNode;
  }

  newNodeRec.inLine = 0;   // create records in the string table

  StringRecord strRec;
  Cardinal index  = emptySlot(*stringTable, freeStringSlots);
  stringTable->Get(index, strRec);
  newNodeRec.s.first = index;
  nodeTable->Put(newNode, newNodeRec);

  unsigned char appendedChars = 0;
  while ( strLen > appendedChars ) {

    unsigned char n = strLen - appendedChars;

    if ( n >  StringFragmentSize ) {
      n = StringFragmentSize;
                }

    value.copy( strRec.field, n, appendedChars );
    appendedChars += n;

    if ( appendedChars < strLen ) { // another fragment is needed

      Cardinal pred = index;  // save reference

      index = emptySlot(*stringTable, freeStringSlots);
      strRec.next = index;
      stringTable->Put(pred, strRec);
      stringTable->Get(index, strRec);

    } else { // last fragment

      strRec.next = 0;
      stringTable->Put(index, strRec);
    }
  }
  return newNode;
}

/*
8.5 SymbolAtom

*/

ListExpr
NestedList::SymbolAtom( const string& value )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  ListExpr newNode = StringAtom( value, false );
  return (newNode);
}


/*
8.6 TextAtom

*/

ListExpr
NestedList::TextAtom()
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  Cardinal newNode = emptySlot(*nodeTable, freeNodeSlots);

  NodeRecord newNodeRec;
  (*nodeTable).Get(newNode, newNodeRec);
  newNodeRec.nodeType = TextType;
  newNodeRec.references = 1;
  newNodeRec.t.start  = emptySlot(*textTable, freeTextSlots);
  newNodeRec.t.last   = newNodeRec.t.start;
  //newNodeRec.t.length = 0;
  (*nodeTable).Put(newNode, newNodeRec);

  TextRecord newTextRec;
  (*textTable).Get(newNodeRec.t.start, newTextRec);
  newTextRec.next = TheEmptyList();
  memset( newTextRec.field, 0, TextFragmentSize );
  (*textTable).Put(newNodeRec.t.start, newTextRec);

  return (newNode);
}

/*
8.7 AppendText

*/

void
NestedList::AppendText( const ListExpr atom,
                        const string& textBuffer )
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( AtomType( atom ) == TextType );

  NodeRecord atomContentRec;
  (*nodeTable).Get(atom, atomContentRec);

  TextRecord lastTextRec;
  (*textTable).Get(atomContentRec.t.last, lastTextRec);

  Cardinal lastFragmentLength = lastTextRec.used();
  Cardinal emptyFragmentLength = TextFragmentSize - lastFragmentLength;

  /*
  cerr << "(emptyFragmentLength, TextFragmentSize, "
       << "lastFragmentLength, textBuffer.length() ): "
       << emptyFragmentLength << ","
       << TextFragmentSize << ","
       << lastFragmentLength << ","
       << textBuffer.length()
       << endl;
  */

/*
There are two cases: Either there is enough space in the current fragment
for NoChars, or there is not enough space. The last fragment of a text atom
is never filled completely (with TextFragmentSize characters), but it is
empty or it is filled with up to TextFragmentSize-1 characters.

*/

  Cardinal textLength = textBuffer.length();
  Cardinal textStart  = 0;
  if ( (lastFragmentLength + textLength) <= TextFragmentSize )
  {
    /* There is enough space in the last fragment. (Case 1) */
    /* --> Append new text.                                 */
    textBuffer.copy( lastTextRec.field + lastFragmentLength,
                     emptyFragmentLength );

    (*textTable).Put(atomContentRec.t.last, lastTextRec);
    (*nodeTable).Put(atom, atomContentRec);
  }
  else
  {
    ListExpr newFragmentID;

    /* There is not enough space in the last fragment. (Case 2) */
    /* Steps 1/2: Fill current fragment completely.             */

    textBuffer.copy( lastTextRec.field + lastFragmentLength,
                     emptyFragmentLength );
    textLength         -= emptyFragmentLength;
    textStart          += emptyFragmentLength;

    /* Step 2/2: Create new (empty) fragments and append them */

    TextRecord newTextRec;
    //TextRecord* plastRec = &lastTextRec;
    while ( textLength > 0 )
    {
      // create a new text record
      newFragmentID = emptySlot(*textTable, freeTextSlots);
      (*textTable).Get(newFragmentID, newTextRec);

      memset( newTextRec.field, 0, TextFragmentSize );
      newTextRec.next  = TheEmptyList();

      // let the last one point to it and save it
      lastTextRec.next = newFragmentID;
      (*textTable).Put(atomContentRec.t.last, lastTextRec);

      // compute how much bytes must be copied
      emptyFragmentLength = (textLength <= TextFragmentSize)
                          ? textLength
                          : TextFragmentSize;

      textBuffer.copy( newTextRec.field, TextFragmentSize, textStart );
      //textBuffer.copy( test1, TextFragmentSize, textStart );
      //cerr << textLength << ", " << textStart << ":" << test1 << endl;
      //cerr << "substr: "
      //     << textBuffer.substr(textStart, TextFragmentSize) << endl;

      textLength -= emptyFragmentLength;
      textStart  += emptyFragmentLength;

      atomContentRec.t.last = newFragmentID;
      lastTextRec = newTextRec;
    }
      //cout << "textLength: " << textLength << endl;
      //cout << "textStart: " << textStart << endl;
      //cout << "field: " << string(newTextRec.field) << endl;
    (*textTable).Put(atomContentRec.t.last, lastTextRec);
    (*nodeTable).Put(atom, atomContentRec);
  }
}

/*
9 Reading Atoms

9.1 IntValue

*/

long
NestedList::IntValue( const ListExpr atom ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( AtomType( atom ) == IntType );
  return ((*nodeTable)[atom].a.value.intValue);
}

/*
9.2 RealValue

*/

double
NestedList::RealValue( const ListExpr atom ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( AtomType( atom ) == RealType );
  return ((*nodeTable)[atom].a.value.realValue);
}

/*
9.3 BoolValue

*/

bool
NestedList::BoolValue( const ListExpr atom ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( AtomType( atom ) == BoolType );
  return ((*nodeTable)[atom].a.value.boolValue);
}

/*
9.4 StringSymbolValue

*/

string
NestedList::StringSymbolValue( const ListExpr atom ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif

  const NodeRecord& atomRef = (*nodeTable)[atom];
  char buffer[MAX_STRINGSIZE + 1];

  const unsigned char strLen = atomRef.strLength;
  assert( strLen <= MAX_STRINGSIZE );

  if ( atomRef.inLine == 1 ) { // copy chars out of node record

    assert(strLen <= STRING_INTERNAL_SIZE);
    memcpy( buffer, atomRef.s.field, strLen );

  } else { // copy chars from string table

    Cardinal index = atomRef.s.first;
    unsigned char appendedChars = 0;
    while ( index ) {

      unsigned char n = strLen - appendedChars;
      if (n >  StringFragmentSize)
      n = StringFragmentSize;

      memcpy( &(buffer[appendedChars]), (*stringTable)[index].field, n );
      appendedChars += n;
      index = (*stringTable)[index].next;
    }
    if( strLen != appendedChars )
    {
      cerr << "strlen: " << (unsigned int) strLen
           << ", appendedChars: " << (unsigned int) appendedChars << endl;
      assert( false );
    };
  }

  buffer[strLen]='\0';
  return string(buffer);
}

/*
9.5 StringValue and SymbolValue

*/

string
NestedList::StringValue( const ListExpr atom ) const
{
  assert( AtomType( atom ) == StringType );
  return StringSymbolValue(atom);
}

string
NestedList::SymbolValue( const ListExpr atom ) const
{
  assert( AtomType( atom ) == SymbolType );
  return StringSymbolValue(atom);
}

/*
9.6 Treatment of Text Atoms

9.6.1 CreateTextScan

*/

TextScan
NestedList::CreateTextScan (const ListExpr atom ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
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
                      string&        textBuffer ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  // load the current fragment
  TextRecord fragment;
  (*textTable).Get(textScan->currentFragment, fragment);

  Cardinal sum=0; // already retrieved bytes in this call
  Cardinal curPos = textScan->currentPosition;

  while( sum < noChars) {

      Cardinal n = min( fragment.used() - curPos, noChars );
      textBuffer.append( &fragment.field[curPos], n);
      sum += n;
      textScan->currentPosition += n;

      if (sum >= noChars) // all bytes retrieved
        return;

      if( fragment.next != 0 )
      {
    // get the next Fragment
        textScan->currentFragment = fragment.next;
        textScan->currentPosition = 0;
    curPos = 0;
        (*textTable).Get(textScan->currentFragment,fragment);
      }
      else
      {
    // more characters requested than stored
        return;
      }
  }
}

/*
9.6.3 TextLength

Currently the length is aggregated by visiting all text record. This is
expensive and was only implemented as a temporary solution. It was done
to save memory in the NodeRecord representation.

*/


Cardinal
TextRecord::used() const {

  Cardinal usedLength = 0;
  for ( usedLength = 0;
        usedLength < TextFragmentSize &&
        field[usedLength];
        usedLength++ );

  return usedLength;
}


Cardinal
NestedList::TextLength ( const ListExpr textAtom ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  assert( AtomType( textAtom ) == TextType );

  TextRecord fragment;
  Cardinal textLength = 0;
  TextsEntry tnext = (*nodeTable)[textAtom].t.start;
  textTable->Get(tnext, fragment);

  textLength += TextFragmentSize;
  while ( (tnext = fragment.next) != 0 ) {

    textTable->Get(tnext, fragment);
    textLength += TextFragmentSize;
  }
  textLength = textLength - TextFragmentSize + fragment.used();
  return (textLength);
}

/*
9.6.4 EndOfText

*/

bool
NestedList::EndOfText( const TextScan textScan ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  if ( textScan->currentFragment == 0 )
  {
    cerr << "textScan->currentFragment == 0: "
         << textScan->currentFragment << endl;
    return (true);
  }
  else
  {
    TextRecord fragment = (*textTable)[textScan->currentFragment];
    Cardinal used = fragment.used();

    cerr << "fragment.next == 0, textScan->currentPosition"
         << " >= usedFragmentLength: "
         << fragment.next << ","
         << textScan->currentPosition << ","
         << used
         << endl;


    return ((fragment.next == 0) &&
            (textScan->currentPosition >= used));
  }
}


/*
9.6.5 Alternative function for iteration over text atoms. This
was implemented, since EndOfText has not been used in the complete
SECONDO code.

*/
 bool
 NestedList::GetNextText( const ListExpr textAtom,
                          string& textFragment,
                          Cardinal size, 
                          TextScanInfo& info           ) const
 {
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
 
   if (info.last) { // end of text reached ?
     textFragment = "";
     DestroyTextScan ( info.textScan );
     info.first = true;
     info.last = false;
     return false;
   }
 
   if (info.first) { // initialize status variables
     info.atom = textAtom;
     info.textFragmentLength = size;
     info.textLength = TextLength( info.atom );
     info.textScan = CreateTextScan( info.atom );
     textFragment.resize(size);
     info.first = false;
   }
 
   assert( (size == info.textFragmentLength)); 
   assert(info.atom == textAtom);
 
   textFragment="";
   /*  Write the text atom to the output stream in chunks
    *  of size textFragmentLength
    */
   if (info.textFragmentLength < info.textLength)
   {
     GetText( info.textScan, info.textFragmentLength, textFragment );
     info.textLength -= info.textFragmentLength;
     assert(info.textLength >= 0);
     return true;
   } else {
     GetText ( info.textScan, info.textLength, textFragment );
     info.last = true; // end of text reached
     return true;
   }
 }


/*
9.6.5 DestroyTextScan

*/

void
NestedList::DestroyTextScan( TextScan& textScan ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  if ( textScan != 0 )
  {
    delete textScan;
    textScan = 0;
  }
}

/*

9.6.1 Text2String

*/

void
NestedList::Text2String( const ListExpr& textAtom, string& resultStr ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif

  ostringstream outStream;
  string textFragment = "";
  TextScanInfo info;
  while ( GetNextText(textAtom, textFragment, 1024,info) ) {
     outStream << textFragment;
  }
  resultStr = outStream.str();

}

string
NestedList::Text2String( const ListExpr& textAtom ) const {

#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  ostringstream outStream;
  string textFragment = "";
  TextScanInfo info;
  while ( GetNextText(textAtom, textFragment, 1024,info) ) {
     outStream << textFragment;
  }
  return outStream.str();

}



/*
Replace some critical symbols within strings representing text atoms.

*/

string transformText2Outtext(const string& value)
{
  string textReplaced = "";
  textReplaced = stringutils::replaceAll(value, "\\", "\\\\");
  textReplaced = stringutils::replaceAll(textReplaced, "'", "\\'");
  return textReplaced;
};

/*

10 AtomType

*/

NodeType
NestedList::AtomType (const ListExpr atom ) const
{
#ifdef THREAD_SAFE
   boost::lock_guard<boost::recursive_mutex> guard1(mtx);
#endif
  if ( !IsAtom( atom ) )
  {
    return (NoAtom);
  }
  else
  {
    return ((*nodeTable)[atom].nodeType);
  }
}

std::ostream& operator<<(ostream& out, StringRecord sr){
  out << string(sr.field,StringFragmentSize);
  if(sr.next) {
     out << "   -> " << sr.next << endl;
  }
  return out;
  
} 


std::ostream& NestedList::printTables(std::ostream& out) const{
   out  << " Nodes: " << endl;
   NodeRecord node;
   if(!nodeTable){
      out << "not present" << endl;
   } else {
     out << "nodeTable has " << nodeTable->NoEntries()  << " entries" << endl;
     for(size_t i=1;i<=nodeTable->NoEntries(); i++){
        out << i << " : " ;
        if(freeNodeSlots.find(i)!=freeNodeSlots.end()){
          out << " free " << endl; 
        } else {
          nodeTable->Get(i,node);
          out << node << endl;
        }
     } 
   }
   out << endl << " Strings : " << endl;
   StringRecord sr;
   if(!stringTable){
      out << "not present" << endl;
   } else {
     out << "stringTable has " << stringTable->NoEntries()  
         << " entries" << endl;
     for(size_t i=1;i<=stringTable->NoEntries(); i++){
        out << i << " : " ;
        if(freeStringSlots.find(i)!=freeStringSlots.end()){
          out << " free " << endl; 
        } else {
          stringTable->Get(i,sr);
          out << sr << endl;
        }
     } 
   }
   out << endl << " Texts : " << endl;
   return out;  


}




const double Tolerance::MINERR = 1e-10;
