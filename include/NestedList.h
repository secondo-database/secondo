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

//paragraph    [10]    title:          [{\Large \bf ]                          [}]
//paragraph    [21]    table1column:   [\begin{quote}\begin{tabular}{l}]       [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:  [\begin{quote}\begin{tabular}{ll}]      [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:  [\begin{quote}\begin{tabular}{lll}]     [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:  [\begin{quote}\begin{tabular}{llll}]    [\end{tabular}\end{quote}]
//characters    [1]    verbatim:       [\verb@]                                [@]
//characters	[2]    formula:       [$]                                     [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[--------]    [\hline]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[star] [{\*}]

1 Header File: Nested List

Copyright (C) 1995 Gral Support Team
    
November 1995 Ralf Hartmut G[ue]ting

May 13, 1996 Carsten Mund

June 10, 1996 RHG Changed result type of procedure ~RealValue~ back to REAL.

September 24, 1996 RHG Cleaned up PD representation.

October 22, 1996 RHG Made operations ~ListLength~ and ~WriteListExpr~ available.

February 2002 Ulrich Telle Port to C++

November 28, 2002 M. Spiekermann. Method reportVectorSizes() added. 

December 05, 2002 M. Spiekermann. Methods InitializeListMemory() and CopyList() supplemented.

Aug/Sept 2003, M. Spiekermann. Some often called methods were defined as inline
functions to reduce the runtime stack.  Producing a nested list in textual
format is now done by an ostream object to avoid creating big string objects
when it is possible to write to a stream (e.g.  cout, file or a TCP/IP
socket). Moreover, a new method WriteBinaryTo() creates a byte sequence
representing a nested list which is much smaller than the textual format. All
this modifications gain a speed up of the client-server communication. 

February 2004, M. Spiekermann. Reading of binary encoded lists was implemented.

June 2004, M. Spiekermann. The persistent implementation of this module was finished. Now it
is possible to process lists which have big textual representations (e.g. 500MB).

July 2004, M. Spiekermann. A runtime check IsPersistentImpl() was added.

July 14, 2004, M. Spiekermann. The struct NodeRecord has been carefully changed
to reduce the size from 20 bytes to 12 bytes. Moreover values for int, real,
bool and small strings and symbols (up to 8 characters) could be stored
directly in the NodeRecord instead of an index for a CTable. Hence one of the
compact tables could be removed. All in all in the average case (no big text
atoms) the memory representation of a nested list will only take about 50
percent compared to the implementation before.

July 2005, M. Spiekermann. Function ~TextAtom~ overloaded. Now a string can be passed
directly in order to set a value.

1.1 Overview

A ~nested list~ can be viewed in two different ways. The first is to consider
it as a list, which may be empty, where each element is either an ~atom~ or a
nested list. An example of a nested list (in textual representation) is:

---- (query (select cities (fun (c city) (> (attribute c pop) 500))))
----

The structure of this list can be shown better by writing it indented:

---- (   query 
         (   select 
             cities 
             (   fun 
                 (c city) 
                 (> (attribute c pop) 500)
             )
         )
     )
----

Hence this is a list with two elements; the first is the atom ``query'', the
second a list again. This list consists of three elements, which are the two
atoms ``select'' and ``cities'', followed by another list. And so forth. Note
that the structure of the list is determined only by parentheses and blanks.

The second, perhaps more implementation-oriented, view of a nested list is
that it is a binary tree. Atoms are the leaves of this tree. 

		Figure 1: Nested List [SNLFigure4.eps]

The nested list module described here offers a type ~ListExpr~ to represent
such structures. A value of this type can be viewed as a pointer which can
point to:

  * nothing; in this case it represents an ~empty list~,

  * a leaf of the binary tree; it represents an ~atom~,

  * an internal node; it represents a ~list~.

Atoms are typed and can be of the following types (on the right hand side
example atoms are shown):

---- Integer          12, -371
     Real             3.14, 62E05
     Boolean          TRUE, FALSE
     String           "Hello, World", "Germany"
     Symbol           fun, c, <=, type
     Text             <text>This is a so-called "text"!</text--->
----    

The precise textual format of the various atoms is important for the
representation of nested lists in files. Files can be edited by a user;
furthermore it should be possible to exchange files and to read them by
programs in other languages than C++. The formats are defined as follows:

For integers and reals, the conventions of C++ for the representation of
constants (literals) apply. The two boolean values are represented as shown.
A string value is a sequence of characters enclosed by double quotes with at
most 48 characters which does not contain a double quote. A symbol value is a
sequence of at most 48 characters described by the following grammar:

----  <symbol> ::= <letter> {<letter> | <digit> | <underline char>}*
                  | <other char> {<other char>}*
----

Here "<other char>"[1] is any character which is not a letter, a digit,
underline or blank and is not contained in the following list of ``forbidden
characters'':

----  (    )    "
----
   
Finally, a text value is any sequence of characters of arbitrary length
enclosed within brackets of the form "<text>"[1] and "</text--->".

1.2 Interface methods

Nested lists are offered in a module ~NestedList~ offering the type ~ListExpr~
and the following operations:

[24]    Construction    & Test        & Traversal  & Input/Output    \\
        [--------]
        TheEmptyList   & IsEmpty     & First      & ReadFromFile    \\
        Cons           & IsAtom      & Rest       & WriteToFile     \\
        Append         & EndOfList   &            & ReadFromString  \\
        Destroy        & ListLength  &            & WriteToString   \\
                       & ExprLength  &            & WriteListExpr   \\
        OneElemList    & Equal       &            & WriteStringTo   \\
        TwoElemList    & IsEqual     & Second     & WriteBinaryTo   \\
        ThreeElemList  &             & Third      & ReadBinaryFrom  \\
        FourElemList   &             & Fourth                       \\
        FiveElemList   &             & Fifth                        \\
        SixElemList    &             & Sixth                        \\
                                
[23]    Construction of atoms  & Reading atoms  & Atom test   \\
        [--------]
        IntAtom                & IntValue       & AtomType    \\
        RealAtom               & RealValue                    \\
        BoolAtom               & BoolValue                    \\
        StringAtom             & StringValue                  \\
        SymbolAtom             & SymbolValue                  \\
                               &                              \\
        TextAtom               & CreateTextScan               \\
        AppendText             & GetText                      \\
                               & EndOfText                    \\
                               & Text2String                  \\

[21]	Initialization and Analysis \\
	[--------]
	initializeListMemory        \\
	ReportTableSizes           \\
			       
The operations are defined below. 

1.3 Includes, Constants and Types

*/

#ifndef NESTED_LIST_H
#define NESTED_LIST_H

#include <string>
#include <iostream>
#include <assert.h>

/* 

The implementation of the nested list structure is based on the CTABLE
structure which has a memory and a SMI based implementation. In order to manage
lists of arbitary size you should switch on the persistent implementation.
define switch NL\_PERSISTENT with the -D option of gcc in order to use
persistent memory representation. This should be configured in the file
makefile.env at the top level of SECONDOs directory structure. But be careful,
the interfaces are not exactly the same. The restrictions are explained in the
file CTable.h. If you change code in the nested list module take care that it
works with both implementaions.

*/
#ifdef NL_PERSISTENT
#ifndef CTABLE_PERSISTENT
#define CTABLE_PERSISTENT
#endif
#endif

#include "CTable.h"
#include "SecondoSMI.h"

/*

Nested lists are represented by four compact tables called
~nodeTable~, ~intTable~, ~stringTable~, and ~textTable~, which are private
member variables of the nested list container class ~NestedList~.

*/
const int INITIAL_ENTRIES = 10000;
/*
The first specifies the default size of the compact tables. This value can be
overwritten in the constructor. 

*/

typedef unsigned long ListExpr;
/*
Is the type to represent nested lists.

*/

typedef unsigned char NodeType;

const NodeType NoAtom = 1;
const NodeType IntType = 2;
const NodeType RealType = 3;
const NodeType BoolType = 4;
const NodeType StringType = 5; 
const NodeType SymbolType = 6;
const NodeType TextType = 7;



/*
Is an enumeration of the different node types of a nested list.

*/

typedef Cardinal NodesEntry;
typedef Cardinal IntsEntry;
typedef Cardinal StringsEntry;
typedef Cardinal TextsEntry;

/*
Pointers into the various tables are all represented by integers; 
0 is interpreted as a nil pointer.

*/

struct Constant
{
  union
  {
    bool   boolValue;
    long   intValue;
    double realValue;
  };
};

/*
Entries in the ~intTable~ table are managed by overlaying scalar variables
of the appropriate scalar data type of an integer, real, or boolean value.

*/

struct TextScanRecord
{
  TextsEntry currentFragment;
  Cardinal   currentPosition;
};
typedef TextScanRecord* TextScan;

/* 
Text entries can be of arbitrary size and are split across as many nodes
of the ~textTable~ table as necesary. It is possible to iterate over these
nodes using a text scan. A ~TextScanrecord~ is used to hold the state of
such a scan. ~currentFragment~ is a pointer to a (valid) entry in the table
~textTable~; ~currentPos~ is a pointer to a character of the current text fragment.

*/

const unsigned char STRINGSIZE = 16;
const unsigned char MAX_STRINGSIZE = 3 * STRINGSIZE;
const unsigned char StringFragmentSize = STRINGSIZE - sizeof(StringsEntry);
const unsigned char STRING_INTERNAL_SIZE = 2*sizeof(TextsEntry);

typedef char StringAtomCharVec[MAX_STRINGSIZE+1];

struct StringRecord 
{ 
  StringsEntry next;
  char field[StringFragmentSize]; 
};
/*
Symbols and strings with a maximum size of "3\times STRINGSIZE"[2] characters
are represented as at most "4"[2] chunks of "STRINGSIZE"[2] characters. This
approach was chosen to minimize memory consumption. If a string is smaller than
STRING\_INTERNAL\_SIZE it can directly be stored in a node record.

*NOTE*: The struct type ~StringRecord~ is introduced only because the vector
templates used in the implementation of compact tables don't allow character
arrays as the template data type.

*/

const unsigned int TEXTSIZE = 64;
const unsigned int TextFragmentSize = TEXTSIZE - sizeof(TextsEntry);
  
struct TextRecord
{
  TextsEntry next;
  char       field[TextFragmentSize];
  // unsigned char emptyChars; This may be useful for storing binary data in Text Atoms
  // currently a value of 0 in field indicates the end.
  // Cardinal length of the has been removed from the NodeRecord definition to
  // shrink the size of a node record. A future improvement could be a meta record
  // which stores the length and/or other information about a text atom.
};
typedef TextRecord* Text;

/*
A text entry is represented as a simple linked list of text chunks.

*/

struct NodeRecord
{
  NodeType nodeType;
  unsigned char isRoot;     // only used for nodeType NoAtom
  unsigned char strLength;  // only used for nodeType String
  unsigned char inLine;     // only used for nodeType String
  union 
  {
    struct                   // NoAtom
    {
      NodesEntry left;
      NodesEntry right;
    } n;
    struct                  // IntType, RealType, BoolType
    {
      Constant value;
    } a;
    struct                  // StringType, SymbolType
    {
      union {        
	StringsEntry first;
        char field[STRING_INTERNAL_SIZE];
      };
    } s;
    struct                  // TextType
    {
      TextsEntry start;
      TextsEntry last;
    } t;
  };
};
typedef NodeRecord* Node;

typedef unsigned char byte;
/*
A ~NodeRecord~ represents all node types of a nested list.

Here only some of the fields need further explanation. ~isRoot~ is ~true~
after creation of an internal node; it is set to ~false~ when the node is
used as an argument to ~Cons~ or as a second argument to ~Append~ which means
this node is made the son of some other node. For a string or symbol atom,
~second~ and ~third~ may be 0 (nil pointers). For a text atom, ~start~ points
to the first entry of the linked list of pieces of text and ~last~ to the last
one; ~length~ is the total number of characters that have been entered into a
text atom.

*/

/*
1.4 Class "NestedList"[2]

*/

class NestedList
{
 public:
  NestedList( SmiRecordFile* ptr2RecFile = 0,
	      Cardinal NodeEntries = 2*INITIAL_ENTRIES,
	      Cardinal ConstEntries = INITIAL_ENTRIES, 
	      Cardinal StringEntries = INITIAL_ENTRIES,
	      Cardinal TextEntries = INITIAL_ENTRIES / 10 );
/*
Creates an instance of a nested list container. The compact tables which
store the nodes of nested lists reserve initially memory for holding at
least ~initialEntries~ nodes.

*/

  virtual ~NestedList();
  
/*
Destroys a nested list container.

*/
/*
1.3.2 Construction Operations

*/
  inline ListExpr TheEmptyList() { return (0); };
/*
Returns a pointer to an empty list (a ``nil'' pointer).

*/
  ListExpr Cons( const ListExpr left, const ListExpr right );
/*
Creates a new node and makes ~left~ its left and ~right~ its right son.
Returns a pointer to the new node.

*Precondition*: ~right~ is no atom.

*/
  ListExpr Append( const ListExpr lastElem,
                   const ListExpr newSon );
/*
Creates a new node ~p~ and makes ~newSon~ its left son. Sets the right son of 
~p~ to the empty list. Makes ~p~ the right son of ~lastElem~ and returns a 
pointer to ~p~. That means that now ~p~ is the last element of the list and 
~lastElem~ the second last.... ~Append~ can now be called with ~p~ as the 
first argument. In this way one can build a list by a sequence of ~Append~ 
calls.

*Precondition*: ~lastElem~ is not the empty list and no atom, but is the last
element of a list. That is: "EndOfList( lastElem ) == true"[4],
"IsEmpty( lastElem ) == false"[4], "IsAtom( lastElem ) = false"[4].

Note that there are no restrictions on the element ~newSon~ that is appended; 
it may also be the empty list.

*/
  void Destroy( const ListExpr list );
/*
Destroys the complete subtree (including all atoms) below the root ~list~.

*Precondition*: ~list~ must be the root of a list binary tree. That means, it 
must not have been used as an argument to ~Cons~ or as a  second argument 
(~newSon~) to ~Append~. This also implies that ~list~ is no atom and is not 
empty. Note that a list structure can have several roots. In such a case one 
has to call ~Destroy~ for each of the roots (and that is permitted). If 
~Destroy~ is called only for some of the roots, an incorrect structure will 
result.

1.3.3 Test Operations

*/
  inline bool IsEmpty( const ListExpr list ) { return (list == 0); };
/*
Returns "true"[4] if ~list~ is the empty list.

*/
  inline bool IsAtom( const ListExpr list )
  {
    if ( IsEmpty( list ) )
    {
       return (false);
    }
    else
    {
      return ((*nodeTable)[list].nodeType != NoAtom);
    }
  };
/*
Returns "true"[4] if ~list~ is an atom.

*/
  inline bool IsNodeType( const NodeType n, const ListExpr list )
  {
    if ( IsEmpty( list ) )
    {
       return (false);
    }
    else
    {
      return ((*nodeTable)[list].nodeType == n);
    }
  };


  inline bool EndOfList( ListExpr list )
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
  };

/*
Returns "true"[4] if ~Right~(~list~) is the empty list. Returns "false"[4] 
otherwise and if ~list~ is empty or an atom.

*/
  int ListLength( ListExpr list );
/*
~list~ may be any list expression. Returns the number of elements, if it is 
a list, and -1, if it is an atom. *Be warned:* unlike most others, this is 
not a constant time operation; it requires a list traversal and therefore 
time proportional to the length that it returns.

*/
  int ExprLength( ListExpr expr );
/* 
Reads a list expression ~expr~ and counts the number ~length~ of
subexpressions.

*/
  bool Equal( const ListExpr list1, const ListExpr list2 );
/*
Tests for deep equality of two nested lists. Returns "true"[4] if ~list1~ is
equivalent to ~list2~, otherwise "false"[4].

*/
  bool IsEqual( const ListExpr atom, const string& str,
                const bool caseSensitive = true );
/* 
Returns "true"[4] if ~atom~ is a symbol atom and has the same value as ~str~.

*/ 

/*
1.3.4 Traversal

*/
  inline ListExpr First( const ListExpr list )
  {
    assert( !IsEmpty( list ) && !IsAtom( list ) );
    return ((*nodeTable)[list].n.left);
  };
/*
Returns (a pointer to) the left son of ~list~. Result can be the empty list.

*Precondition*: ~list~ is no atom and is not empty.

*/
  inline ListExpr Rest( const ListExpr list )
  {
    assert( !IsEmpty( list ) && !IsAtom( list ) );
    return ((*nodeTable)[list].n.right);
  };
/*
Returns (a pointer to) the right son of ~list~. Result can be the empty list.

*Precondition*: ~list~ is no atom and is not empty.

1.3.5 Input/Output

*/
  bool ReadFromFile( const string& fileName,
                     ListExpr& list );
/*
Reads a nested list from file ~filename~ and assigns it to ~list~. 
The format of the file must be as explained above. Returns "true"[4] if reading 
was successful; otherwise "false"[4], if the file could not be accessed, or the 
line number in the file where an error occurred.

*/
  bool WriteToFile( const string& fileName,
                    const ListExpr list );
/*
Writes the nested list ~list~ to file ~filename~. 
The format of the file will be as explained above. The previous contents 
of the file will be lost. Returns "true"[4] if writing was successful, "false"[4]
if the file could not be written properly.

*Precondition*: ~list~ must not be an atom.

*/
  bool ReadFromString( const string& nlChars,
                       ListExpr& list );
  bool ReadBinaryFrom( istream& in, ListExpr& list);
/*
Like ~ReadFromFile~, but reads a nested list from string ~nlChars~ or istream ~in~. 
Returns "true"[4] if reading was successful.

*/
  bool WriteToString( string& nlChars,
                      const ListExpr list );
/*
Like ~WriteToFile~, but writes to the string ~nlChars~. Returns "true"[4]
if writing was successful, "false"[4] if the string could not be written properly.

*Precondition*: ~list~ must not be an atom.

*/
  bool WriteStringTo( const ListExpr list, ostream& os );
  bool WriteBinaryTo( const ListExpr list, ostream& os );
/*
Writes the list in a binary coded or textual format into the referenced stream.

Note: When using an fstream with WriteBinaryTo initialize it as ios::binary
otherwise the output of bytes will be influenced by platform specific
implementations

*/

  string ToString( const ListExpr list );

/*
A wrapper for ~WriteToString~ which directly returns a string object. 

*/

  void WriteListExpr( ListExpr list, ostream& ostr );
  void WriteListExpr( ListExpr list );

/*
Write ~list~ indented by level to standard output.

1.3.5 Auxiliary Operations for Construction

A number of procedures is offered to construct lists with one, two, three,
etc. up to six elements. 

*/
  inline ListExpr OneElemList( const ListExpr elem1 )
  {
    return (Cons( elem1, TheEmptyList() )); };

  inline ListExpr TwoElemList( const ListExpr elem1,
                               const ListExpr elem2 )
  {
    return (Cons( elem1, OneElemList(elem2) )); };

  inline ListExpr ThreeElemList( const ListExpr elem1,
                                 const ListExpr elem2,
                                 const ListExpr elem3 )
  {
    return (Cons( elem1, TwoElemList(elem2, elem3) )); };


  inline ListExpr FourElemList( const ListExpr elem1,
                                const ListExpr elem2,
                                const ListExpr elem3,
                                const ListExpr elem4 )
  {
    return (Cons( elem1, ThreeElemList(elem2, elem3, elem4) )); };

  inline ListExpr FiveElemList( const ListExpr elem1,
                                const ListExpr elem2,
                                const ListExpr elem3,
                                const ListExpr elem4,
                                const ListExpr elem5 )
  {
    return (Cons( elem1, FourElemList(elem2, elem3, elem4, elem5) )); };

  inline ListExpr SixElemList( const ListExpr elem1,
                               const ListExpr elem2,
                               const ListExpr elem3,
                               const ListExpr elem4,
                               const ListExpr elem5,
                               const ListExpr elem6 )
  {
    return (Cons( elem1, FiveElemList(elem2, elem3, elem4, elem5, elem6) )); };
    
/*
A pointer to the new list is returned.

1.3.6 Auxiliary Operations for Traversal

Similarly, there are procedures to access the second, ..., sixth element. 
Acessing the first element is a basic operation defined above.

*/
  inline ListExpr Second( const ListExpr list ) { return (NthElement( 2, 2, list )); };
  inline ListExpr  Third( const ListExpr list ) { return (NthElement( 3, 3, list )); };
  inline ListExpr Fourth( const ListExpr list ) { return (NthElement( 4, 4, list )); };
  inline ListExpr  Fifth( const ListExpr list ) { return (NthElement( 5, 5, list )); };
  inline ListExpr  Sixth( const ListExpr list ) { return (NthElement( 6, 6, list )); };
  inline ListExpr  Nth( int n, const ListExpr list ) { return (NthElement( n, n, list )); };

/*
A pointer to the respective element is returned. Result may be the empty list,
of course.

*Precondition*: ~list~ must not be an atom and must have at least as many
elements.

1.3.7 Construction of Atoms

There is a set of operations to transform each of the basic types into a
corresponding atom:

*/
  ListExpr IntAtom( const long value );
  ListExpr RealAtom( const double value );
  ListExpr BoolAtom( const bool value );
  ListExpr StringAtom( const string& value, bool isString=true );
  ListExpr SymbolAtom( const string& value );

  ListExpr inline SetStringAtom( const StringAtomCharVec& value) { 
    return StringAtom( string(value) ); 
  };
  ListExpr inline SetSymbolAtom( const StringAtomCharVec& value) { 
    return StringAtom( string(value) ); 
  };

/*
Note: ~Symbols~ and ~Strings~ are character sequences up to 3[star]STRINGSIZE.
SymbolAtom is only a wrapper which calls Stringatom(value,false) to avoid
duplicated code.

Values of type ~Text~ may have arbitrary length. To construct ~Text~ atoms,
two operations are offered:

*/
  ListExpr TextAtom();
  inline ListExpr TextAtom(const string& value) 
  { 
    ListExpr l = TextAtom(); 
    AppendText(l,value); 
    return l;
  }
  void AppendText( const ListExpr atom,
                   const string&  textBuffer );
/*
The first operation ~TextAtom~ creates the atom. Calls of  ~AppendText~ add
pieces of text stored in ~textBuffer~ at the end.

*Precondition*: ~atom~ must be of type ~Text~.

1.3.8 Reading Atoms

There are corresponding procedures to get typed values from atoms:

*/
  long IntValue( const ListExpr atom );
/*
*Precondition*: ~atom~ must be of type ~Int~.

*/
  double RealValue( const ListExpr atom );
/*
*Precondition*: ~atom~ must be of type ~Real~.

*/
  bool BoolValue( const ListExpr atom);
/*
*Precondition*: ~atom~ must be of type ~Bool~.

*/
  string StringValue( const ListExpr atom );
/*
*Precondition*: ~atom~ must be of type ~String~.

*/
  string SymbolValue( const ListExpr atom);
/*
*Precondition*: ~atom~ must be of type ~Symbol~.

*/

 const ListExpr TypeError() const { return typeError; }

/*
Again, the treatment of ~Text~ values is a little more difficult.
To read from a ~Text~ atom, a ~TextScan~ is opened.

*/
  TextScan CreateTextScan( const ListExpr atom );
/*
Creates a text scan. Current position is 0 (the first character in the ~atom~).

*Precondition*: ~atom~ must be of type ~Text~.

*/
  void GetText( TextScan textScan, const Cardinal noChars,
                string& textBuffer );
  bool GetNextText(const ListExpr textAtom, string& textFragment, const int size);


/*
Copies ~noChars~ characters, starting from the current position in the ~scan~
and appends them to the string ~textBuffer~.

The text behind the current position of the ~scan~ may be shorter than ~noChars~.
In this case, all characters behind the current ~scan~ position are copied.

The second alternative of iteration returns true while ~size~ characters are in the
text. The size can not be changed during subseqent calls. The function returns false
when the text ends and the next call of the function will restart the iteration.

*/
  bool EndOfText( const TextScan textScan );
/*
Returns "true"[4], if the current position of the ~TextScan~ is behind the last
character of the text.

*/
  void DestroyTextScan( TextScan& textScan );
/*
Destroys the text scan ~textScan~ by deallocating the corresponding memory.

*/
  Cardinal TextLength( const ListExpr textAtom );
/*
Returns the number of characters of ~textAtom~.

*Precondition*: ~atom~ must be of type ~Text~.

*/

void Text2String( const ListExpr& textAtom, string& resultStr );
string Text2String( const ListExpr& textAtom);

/*
Transforms the text atom into C++ string object 

1.3.10 Atom Test

*/
  NodeType AtomType( const ListExpr atom );
  void ExtractAtoms( const ListExpr list, vector<ListExpr>& atomVec) {
    
    if ( IsEmpty(list) )
       return;

    if ( IsAtom(list) ) { 
       atomVec.push_back(list);
       return; 

    } else {

       ExtractAtoms( First(list), atomVec );
       ExtractAtoms( Rest(list), atomVec );
    }
  }
	
/*

~AtomType~ determines the type of list expression ~atom~ according to the enumeration
type ~NodeType~. If the parameter is not an atom, the function returns the
value 'NoAtom'. ~ExtractAtoms~ returns a flat list of atoms stored in a vector. 
Afterwards you can easily iterate over the atoms.

1.3.11 Size and Implementation Info

*/
  const string ReportTableSizes(const bool onOff, const bool prettyPrint = false);
  const string ReportTableStates() { 
    return ( "Nodes: " + nodeTable->StateToStr() + "\n" );
  }
  static string SizeOfStructs(); 

/*
Reports the slot numbers and allocated memory of all
private CTable members and the underlying vector classes.  

*/

  static const bool IsPersistentImpl() { return isPersistent; }
  string MemoryModel();


/*
Returns the Memory-Model of the underlying CTable data structures. Possible values
are PERSISTENT and NON-PERSISTENT. The PERSISTENT variant uses Berkeley-DB Records
for its nodes instead of the NON-PERSISTENT version which uses heap memory. SizeOfStructs
returns the memory used for some of the structs defined above.


1.3.12 New Initialization of List Memory

*/

  void initializeListMemory(
			     Cardinal NodeEntries = 2*INITIAL_ENTRIES,
		             Cardinal ConstEntries = INITIAL_ENTRIES,
		             Cardinal StringEntries = INITIAL_ENTRIES,
			     Cardinal TextEntries = INITIAL_ENTRIES / 10 );

/*
Creates new ~CTable~ objects with the given size and deletes the old ones.
The default values are tuning parameters and reflect values which are
useful in the present development state of SECONDO.

1.3.13 Copying of Lists

*/

  const ListExpr CopyList( const ListExpr list, NestedList* target );

/*
Copies a nested list from ~this~ instance to the target instance.

*/

 protected:
  const ListExpr CopyRecursive( const ListExpr list, const NestedList* target );
  
  void DestroyRecursive ( const ListExpr list );
  void DeleteListMemory();                            // delete CTable pointers
  void PrintTableTexts();


  string NodeType2Text( NodeType type );
  string BoolToStr( const bool boolValue );

  ListExpr NthElement( const Cardinal n,
                       const Cardinal initialN,
                       const ListExpr list );

  bool WriteList( ListExpr list, const int level,
                  const bool afterList, const bool toScreen );

  void WriteAtom( const ListExpr atom, bool toScreen );

  bool WriteToStringLocal( ostream& nlChars, ListExpr list );

 private:
 
  unsigned int UsedBytesOfTextFragment(const TextRecord& fragment);
 
  // Common code for symbols and strings
  string NestedList::StringSymbolValue( const ListExpr atom );
 
  // Two alternative list copying methods
  const ListExpr SimpleCopy( const ListExpr list, NestedList* target );
  const ListExpr SophisticatedCopy( const ListExpr list, const NestedList* target );
  
  // prototypes for functions used for the binary encoding/decoding of lists
  bool  WriteBinaryRec( ListExpr list, ostream& os );
  bool  ReadBinaryRec( ListExpr& result, istream& in );
  bool  ReadBinarySubLists( ListExpr& LE, istream& in, unsigned long length );
  long  ReadShort( istream& in );  
  long  ReadInt( istream& in, const int len = 4 );
  void  ReadString( istream& in, string& outStr, unsigned long length );
  
  byte  GetBinaryType(ListExpr list);
  char* hton(long value);
  inline void  NestedList::swap(char* buffer,int size);
  ListExpr typeError; 
  
  CTable<NodeRecord>   *nodeTable;   // nodes
  //CTable<Constant>     *intTable;    // ints;
  CTable<StringRecord> *stringTable; // strings
  CTable<TextRecord>   *textTable  ; // texts

  ostream*             outStream;

  static bool          doDestroy;
  static const bool    isPersistent;
  void AppendShortText( const ListExpr atom,
                        const string&  textBuffer );


/*
The class member ~doDestroy~ defines whether the ~Destroy~ method really
destroys a nested list. Only if ~doDestroy~ is "true"[4], nested lists are
destroyed.

As long as the ~Nested List~ class does not support reference counting
it might be necessary to set ~doDestroy~ to "false"[4] to avoid problems due
to deleting parts of nested lists which are still in use elsewhere.

*/
};




#endif

