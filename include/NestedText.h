/**************************************************
//[x] [$\times $]
//[->] [$\rightarrow $]
//paragraph [2] verse:	[\begin{verse}]	[\end{verse}]

2 The Module NestedText

2.1 Definition Part

(File ~PDNestedText.h~)

This module allows one to create nested text structures and to write them to
standard output. It provides in principle a data type ~listexpr~ (representing
such structures) and operations:

[2]	atom: string [x] int [->] listexpr		\\
	atomc: string [->] listexpr			\\
	concat: listexpr [x] listexpr [->] listexpr	\\
	print: listexpr [->] e				\\
	copyout: listexpr [->] string [x] int		\\
	release-storage					\\

However, for use by ~lex~ and ~yacc~ generated lexical analysers and parsers
which only allow to associate integer values with grammar symbols, we represent
a ~listexpr~ by an integer (which is, in fact, an index into an array for
nodes). Hence we have a signature:

[2]	atom: string [x] int [->] int			\\
	atomc: string [->] int				\\
	concat: int [x] int [->] int			\\
	print: int [->] e				\\
	copyout: int [->] string [x] int		\\
	release-storage					\\

The module uses two storage areas. The first is a buffer for text characters,
it can take up to STRINGMAX characters, currently set to 30000. The second
provides nodes for the nested list structure; currently up to NODESMAX = 30000
nodes can be created.

The operations are defined as follows:

----	int atom(string, length) 
	char *string; 
	int length;
----

List expressions, that is, values of type ~listexpr~ are either atoms or lists.
The function ~atom~ creates from a character string ~string~ of length ~length~
a list expression which is an atom containing this string. Possible errors: The
text buffer or storage space for nodes may overflow.

----	int atomc(string) 
	char *string;
----

The function ~atomc~ works like ~atom~ except that the parameter should be a
null-terminated string. It determines the length itself. To be used in
particular for string constants written directly into the function call.

----	int concat(list1, list2) 
	int list1, list2;
----

Concats the two lists; returns a list expression representing the concatenation.
Possible error: the storage space for nodes may be exceeded.

----	print(list)
	int list;
----

Writes the character strings from all atoms in ~list~ in the right order to
standard output. 

----	copyout(list, target, lengthlimit)
	int list;
	char *target;
	int lengthlimit;
----

Copies the character strings from all atoms in ~list~ in the right order into a
string variable ~target~. Parameter ~lengthlimit~ ensures that the maximal
available space in ~target~ is respected; an error occurs if the list expression
~list~ contains too many characters.

----	release_storage()
----

Destroys the contents of the text and node buffers. Should be used only when a
complete piece of text has been recognized and written to the output. Warning:
Must not be used after pieces of text have been recognized for which the parser
depends on reading a look-ahead token! This token will be in the text and node
buffers already and be lost. Currently this applies to lists.

The following is what is technically exported from this file:

***************************************/

#ifndef NESTED_TEXT_H
#define NESTED_TEXT

#include <string>

#include "MemCTable.h"

struct ListExpression
{
  Cardinal left;
  Cardinal right;
  Cardinal atom;      /* index into array text */
  Cardinal length;    /* no of chars in atomstring*/
};

//const Cardinal TEXTNODE_SIZE = 4096;
const Cardinal TEXTNODE_SIZE = 128;
struct TextNode
{
  char text[TEXTNODE_SIZE];
};

class NestedText
{
 public:
  static Cardinal Atom( char* str, int length );
  static Cardinal AtomC( char* str );
  static bool     IsAtom( Cardinal list );
  static Cardinal Concat( Cardinal list1, Cardinal list2 );
  static void     Print( Cardinal list );
  static void     CopyOut( Cardinal list, string& target );
  static void     ReleaseStorage();
  static void     ShowStorage();   /* show_storage used only for testing */
 private:
  NestedText();
  ~NestedText();
  static void     CopyList( Cardinal list, string& target );
  static Cardinal               firstFreeChar;
  static Cardinal               maxFreeChar;
  static MemCTable<ListExpression> nodeSpace;
  static MemCTable<TextNode>       textSpace;
};

#endif

