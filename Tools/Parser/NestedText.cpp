/****************************************************************************

2.2 Implementation Part

*****************************************************************************/

using namespace std;

#include <cstdio>
#include <string>
#include <string.h>
#include "NestedText.h"

/*
If ~left~ is NULL then this represents an atom, otherwise it is a list in
which case ~atomstring~ must be NULL.

*/

Cardinal               NestedText::firstFreeChar = 0;
CTable<ListExpression> NestedText::nodeSpace( 64 );
CTable<TextNode>       NestedText::textSpace( 16 );

/***************************************

The function ~atom~ creates from a character string ~string~ of length
~length~ a list expression which is an atom containing this string.
Possible errors: The text buffer or storage space for nodes may overflow.

***************************************/

Cardinal
NestedText::Atom( char* str, int length )
{
  int i;

  /* put string into text buffer: */

  Cardinal textNode = firstFreeChar / TEXTNODE_SIZE + 1;
  Cardinal offset   = firstFreeChar % TEXTNODE_SIZE;
  for ( i = 0; i< length; i++ )
  {
    textSpace[textNode].text[offset] = str[i];
    offset++;
    if ( offset >= TEXTNODE_SIZE )
    {
      textNode++;
      offset = 0;
      textSpace.EmptySlot();
    }
  }
    
  /* create new node */

  Cardinal newNode = nodeSpace.EmptySlot();
  ListExpression& newNodeRef = nodeSpace[newNode];
  newNodeRef.left  = 0;
  newNodeRef.right = 0;
  newNodeRef.atom  = firstFreeChar; 
  newNodeRef.length = length;
  firstFreeChar  += length;

  return (newNode);
}

/****************************************

The function ~atomc~ works like ~atom~ except that the parameter should
be a null-terminated string. It determines the length itself. To be used
in particular for string constants written directly into the function call.

******************************************/

Cardinal
NestedText::AtomC( char* str )
{
  return (Atom( str, strlen( str ) ));
}

/**************************************

The function ~concat~ concats two lists; it returns a list expression
representing the concatenation. Possible error: the storage space for
nodes may be exceeded.

****************************************/

Cardinal
NestedText::Concat( Cardinal list1, Cardinal list2 )
{
  Cardinal newNode = nodeSpace.EmptySlot();
  ListExpression& newNodeRef = nodeSpace[newNode];
  newNodeRef.left   = list1;
  newNodeRef.right  = list2;
  newNodeRef.atom   = 0;
  newNodeRef.length = 0;
  return (newNode);
}

/***************************************

Function ~print~ writes the character strings from all atoms in ~list~ in
the right order to standard output. 

*****************************************/

void
NestedText::Print( Cardinal list )
{
  Cardinal i;

  if ( IsAtom( list ) )
  {
    Cardinal start = nodeSpace[list].atom;
    Cardinal block  = start / TEXTNODE_SIZE +1;
    Cardinal offset = start % TEXTNODE_SIZE;
    
    for ( i = 0; i < nodeSpace[list].length; i++ )
    {
      putchar( textSpace[block].text[offset] );
      offset++;
      if (offset >= TEXTNODE_SIZE )
      {
        block++;
        offset = 0;
      }
    }
  }
  else
  {
    Print( nodeSpace[list].left );
    Print( nodeSpace[list].right );
  }
}

/******************************************

Function ~isatom~ obviously checks whether a list expression ~list~ is an atom.

*******************************************/

bool
NestedText::IsAtom( Cardinal list )
{
  return (nodeSpace[list].left == 0);
}

/*******************************************

The function ~copyout~ copies the character strings from all atoms in
~list~ in the right order into a string variable ~target~. Parameter
~lengthlimit~ ensures that the maximal available space in ~target~ is
respected; an error occurs if the list expression ~list~ contains too
many characters. ~Copyout~ just calls an auxiliary recursive procedure
~copylist~ which does the job.

*******************************************/

void
NestedText::CopyOut( Cardinal list, string& target )
{
  target = "";
  CopyList( list, target );
}

void
NestedText::CopyList( Cardinal list, string& target )
{
  if ( IsAtom( list ) )
  {
    Cardinal i;
    Cardinal firstBlock  = nodeSpace[list].atom / TEXTNODE_SIZE + 1;
    Cardinal startOffset = nodeSpace[list].atom % TEXTNODE_SIZE;
    Cardinal totalLength = nodeSpace[list].length;
    Cardinal lastBlock   = (nodeSpace[list].atom+totalLength) / TEXTNODE_SIZE + 1;
    Cardinal endOffset   = (nodeSpace[list].atom+totalLength) % TEXTNODE_SIZE;
    for ( i = firstBlock; i < lastBlock; i++ )
    {
      target.append( &textSpace[i].text[startOffset], TEXTNODE_SIZE-startOffset );
      totalLength -= (TEXTNODE_SIZE-startOffset);
      startOffset = 0;
    }
    if ( endOffset > 0 )
    {
      target.append( &textSpace[lastBlock].text[startOffset], totalLength );
    }
  }
  else
  {
    CopyList( nodeSpace[list].left, target );
    CopyList( nodeSpace[list].right, target );
  }
}

/****************************************

Function ~release-storage~ destroys the contents of the text and node
buffers. Should be used only when a complete piece of text has been
recognized and written to the output. Do not use it for text pieces
whose recognition needs look-ahead!

*****************************************/

void
NestedText::ReleaseStorage()
{
  Cardinal i;
  for ( i = nodeSpace.NoEntries(); i > 0; i-- )
  {
    nodeSpace.Remove( i );
  }
  for ( i = textSpace.NoEntries(); i > 0; i-- )
  {
    textSpace.Remove( i );
  }
  firstFreeChar = 0;
}

/****************************************

Function ~show-storage~ writes the contents of the text and node buffers
to standard output; only used for testing.

*****************************************/

void
NestedText::ShowStorage()
{
  Cardinal i;
  Cardinal lastBlock = firstFreeChar / TEXTNODE_SIZE + 1;
  Cardinal offset    = firstFreeChar % TEXTNODE_SIZE;
  string str;
  for ( i = 1; i < lastBlock; i++ )
  {
    str = "";
    str.append( textSpace[i].text, TEXTNODE_SIZE );
    cout << str;
  }
  if ( offset > 0 )
  {
    str = "";
    str.append( textSpace[lastBlock].text, offset );
    cout << str;
  }
  cout << endl;

  for ( i = 1; i < nodeSpace.NoEntries(); i++ )
  {
    cout << "Node: "     << i <<
            ", left: "   << nodeSpace[i].left <<
            ", right: "  << nodeSpace[i].right <<
            ", atom: "   << nodeSpace[i].atom <<
            ", length: " << nodeSpace[i].length << endl;
  }
}

