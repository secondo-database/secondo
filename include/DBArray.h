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
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [$\leq$]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: DBArray

Version: 0.7

August 2002 RHG

April 2003 Victor Almeida chaged the implementation to not use templates.

1.1 Overview

This module offers a generic persistent array implemented on top of the
FLOB interface.

1.2 Interface methods

This module offers the following methods:

[23]	Creation/Removal 	& Access   	& Inquiries	\\ 	
	[--------]
	DBArray        		& Get 		& NoComponents	\\  	
	[tilde]DBArray		& Put		  & Id		        \\
	MarkDelete		    &		      & 		          \\

Operations have to follow the protocol shown below:

		Figure 1: Protocol [Protocol.eps]

1.3 Class ~DBArray~

An instance of the class is a handle to a persistent array of fixed size.
The elements of the array must implement the abstract class ~DBArrayElement~.

*/

#ifndef DBARRAY_H
#define DBARRAY_H

#include "SecondoSMI.h"
#include "FLOB.h"

class DBArrayElement
{
  public:
    virtual ~DBArrayElement() {}; 
    virtual char *ToString() const = 0;
    virtual void FromString( char *str ) = 0;
};

class DBArray
{
 public:

  DBArray( const int elemSize );
  DBArray( const int elemSize, const int n, const bool alloc, const bool update );

/*
Creates creates a new ~SmiRecord~ on the ~SmiRecordFile~ for this
persistent array. One can define an initial size of the persistent
array with the argument ~initsize~. 

*/
  
  DBArray( const int elemSize, SmiRecordFile *file, const SmiRecordId& id, const bool update );

/*
Opens the ~SmiRecordFile~ and the ~SmiRecord~ for the persistent array. The boolean 
value ~update~ indicates if open mode: ~true~ for update and ~false~ for read-only.

*/

  ~DBArray();

/*
Destroys the handle. If the array is marked for deletion, then it also destroys the
persistent array.

*/

  void MarkDelete();

/*
Marks the persistent array for deletion. It will be permanently deleted on the 
destruction of the object.

*Precondition:* The array must be opened in update mode.

*/

  void Put(int const index, const DBArrayElement& elem);

/*
Copies element ~elem~ into the persistent array at index ~index~.

*Precondition:* 0 [<=] ~index~ [<=] ~noComponents~ - 1. The array must be opened in update mode.

*/

  void Get(int const index, DBArrayElement& elem);

/*
Returns the element ~index~ of the array.

*Precondition:* 0 [<=] ~index~ [<=] ~noComponents~ - 1. 

*/

  const int GetNoComponents() const;

/*
Returns the number of components of this array.

*/

  const bool Save( SmiRecordFile *file );

/*
Saves the array and returns its identifier.

*/

  const SmiRecordId GetRecordId() const;
/*
Returns the record identifier of this array.

*/

  FLOB *GetArray() const;
/*
Returns a pointer to the (FLOB) array.

*/

 private:
  bool writeable;
  int noComponents;
  bool canDelete;
  int elemSize;
  FLOB *array;
};


#endif //DBARRAY_H

