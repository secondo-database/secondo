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

1.1 Overview

This module offers a generic persistent array implemented on top of the
FLOB interface.



1.2 Interface methods

This module offers the following methods:

[23]	Creation/Removal 	& Access   	& Inquiries	\\ 	
	[--------]
	DBArray        		& Get 		& NoComponents	\\  	
	[tilde]DBArray		& Put		& Id		\\
	MarkDelete		&		& 		\\

Operations have to follow the protocol shown below:

		Figure 1: Protocol [Protocol.eps]

1.3 Class ~DBArray~

An instance of the class is a handle to a persistent array of fixed size with
elements of type ~T~.

*/

#ifndef PARRAY_H
#define PARRAY_H

#include <iostream> 
#include <cassert>
#include "SecondoSMI.h"
#include "FLOB.h"

template<class T>
class DBArray
{
 public:

  DBArray( SmiRecordFile *file );

/*
Creates creates a new ~SmiRecord~ on the ~SmiRecordFile~ for this
persistent array. One can define an initial size of the persistent
array with the argument ~initsize~. 

*/
  
  DBArray( SmiRecordFile *file, const SmiRecordId& id, const bool update );
  DBArray( SmiRecordFile *file, const int n, const bool alloc, const bool update );

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

  void Put(int const index, const T& elem);

/*
Copies element ~elem~ into the persistent array at index ~index~.

*Precondition:* 0 [<=] ~index~ [<=] ~noComponents~ - 1. The array must be opened in update mode.

*/

  void Get(int const index, T& elem);

/*
Returns the element ~index~ of the array.

*Precondition:* 0 [<=] ~index~ [<=] ~noComponents~ - 1. 

*/

  const int GetNoComponents() const;

/*
Returns the number of components of this array.

*/

  const bool Save();

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
  FLOB *array;
};


/*
2 Implementation of DBArray

Version: 0.7

August 2002 RHG

2.1 Overview

This module offers a generic persistent array implemented on top of the
FLOB interface.

*/

template<class T>
DBArray<T>::DBArray( SmiRecordFile *file ) :
writeable( true ),
noComponents( 0 ),
canDelete( false ),
array( new FLOB( file, sizeof(int), true, true ) )
{
  array->Write( 0, sizeof(int), (char *)(&noComponents) );
}

template<class T>
DBArray<T>::DBArray( SmiRecordFile *file, const SmiRecordId& id, const bool update ) :
writeable( update ),
canDelete( false ),
array( new FLOB( file, id, update ) )
{
  array->Get( 0, sizeof( int ), (char *)(&noComponents) );
}

template<class T>
DBArray<T>::DBArray( SmiRecordFile *file, const int n, const bool alloc, const bool update ) :
writeable( update ),
noComponents( n ),
canDelete( false ),
array( new FLOB( file, sizeof(int) + n * sizeof(T), alloc, update ) )
{
  if( alloc )
    array->Write( 0, sizeof( int ), (char *)(&noComponents) );
}

template<class T>
DBArray<T>::~DBArray()
{
  if ( canDelete ) 
  {
    array->Destroy();
  }
  delete array;
}

template<class T>
void DBArray<T>::Put(const int index, const T& elem)
{
  assert ( writeable );
  if( noComponents <= index )
  {
    noComponents = index + 1;
    array->Resize( sizeof(int) + noComponents * sizeof(T) );	
  }
  array->Write(sizeof(int) + index * sizeof(T), sizeof(T), (char *)(&elem) );
}

template<class T>
void DBArray<T>::Get(int const index, T& elem)
{
  assert ( 0 <= index && index < noComponents );

  array->Get(sizeof(int) + index * sizeof(T), sizeof(T), (char *)(&elem));
}

template<class T>
void DBArray<T>::MarkDelete() 
{
  assert( writeable );
  canDelete = true;
}


template<class T>
const int DBArray<T>::GetNoComponents() const
{
  return noComponents;
}


template<class T>
const bool DBArray<T>::Save() 
{ 
  array->Write( 0, sizeof( int ), (char *)(&noComponents) );
  return array->SaveToLob();
}

template<class T>
const SmiRecordId DBArray<T>::GetRecordId() const 
{ 
  return array->GetLobId();
}

template<class T>
FLOB *DBArray<T>::GetArray() const 
{ 
  return array;
}

#endif

