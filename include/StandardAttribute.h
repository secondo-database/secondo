/*
//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[11]	title:		[{\large \bf ] [}]
//paragraph	[12]	title:		[{\normalsize \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Standard Attribute

May 1998 Stefan Dieker

December 1998 Friedhelm Becker

April 2002 Ulrich Telle Adjustments for the new Secondo version

1 Class ~StandardAttribute~

*/

#ifndef STANDARDATTRIBUTE_H
#define STANDARDATTRIBUTE_H

#include "Attribute.h"

class StandardAttribute : public Attribute
{
 public:
  virtual size_t HashValue() = 0;
/*
The hash function.

*/

  virtual void CopyFrom(StandardAttribute* right) = 0;
/*
Copies the contents of ~right~ into ~this~. Assumes that ~this~ and
~right~ are of the same type. This can be ensured by the type checking functions
in the algebras.

*/
};

/*
2 Class ~IndexableStandardAttribute~

This class is intended to be used for data types that need to be indexed by B-Trees.
The standard types such as ~int~, ~string~, and ~real~ are directly supported. For
the other (more complex) data types, they need to be converted to a string 
(char pointer) and then indexed. The string representation of the objects should 
preserve the ordering. These data types must also belong to the kind ~INDEXABLE~. 
For an example, see the ~DateTime~ algebra.

*/

class IndexableStandardAttribute : public StandardAttribute
{
  public:
    virtual void WriteTo( char *dest ) const = 0;
/*
This function writes the object value to a string ~dest~.

*/

    virtual void ReadFrom( const char *src ) = 0;
/*
This function reads the object value from a string ~src~.

*/

    virtual SmiSize SizeOfChars() const = 0;
/*
This function returns the number of bytes of the object's string representation.

*/ 
};

#endif

