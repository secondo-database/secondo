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
SecondoSMI interface.



1.2 Interface methods

This module offers the following methods:

[23]	Creation/Removal 	& Access   	& Inquiries	\\ 	
	[--------]
	DBArray        		& Get 		& Size		\\  	
	[tilde]DBArray		& Put		& Id		\\
	MarkDelete		&		& 		\\


1.3 Class ~DBArray~

An instance of the class is a handle to a persistent array of fixed size with

elements of type ~T~.

*/

#ifndef MARRAY_H
#define MARRAY_H

#include <iostream> 
#include <cassert>
#include <vector>
#include <algorithm>
#include "GArray.h"

template<class T>
class DBArray : public GArray<T>
{
 public:

  DBArray( SmiRecordFile *parrays, const int initsize = 0 );
/*
Creates a new ~SmiRecord~ on the ~SmiRecordFile~ for this
persistent array. One can define an initial size of the persistent
array with the argument ~initsize~. 

*/

  DBArray( const int initsize = 0 );
/*
Create a new memory version of the DBArray. It is used for temporary arrays.

*/
  
  DBArray( SmiRecordFile *parrays, const SmiRecordId& id, const bool update = true );
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

*Precondition:* 0 [<=] ~index~ [<=] ~size~ - 1. The array must be opened in update mode.

*/

  void Get(int const index, T& elem);
/*
Returns the element ~index~ of the array.

*Precondition:* 0 [<=] ~index~ [<=] ~size~ - 1. 

*/

  void Clear();
/*
Clears the persistent array.

*/

  void Sort( bool (*cmp)(const T&, const T&) )  
  {
    if( size <= 1 ) 
      return;

    sort( marray->begin(), marray->end(), cmp );
  }
/*
Sorts the persisten array given the ~cmp~ comparison criteria.

*/

  const int Size() const;
/*
Returns the size of this array.

*/

  const SmiRecordId Id() const;
/*
Returns the identifier of this array.

*/

 private:

  bool writeable;
  int size;
  vector<T> *marray;

};


/*
2 Implementation of DBArray

Version: 0.7

August 2002 RHG
October 2003 M. Spiekermann

2.1 Overview

This module offers the interface of a generic persistent array implemented on top of the
SecondoSMI interface. It is the same as DBArray but renamed into DBArray

*/

template<class T>
DBArray<T>::DBArray( SmiRecordFile *parrays, const int initsize ) :
writeable( true ),
size( 0 ),
marray( new vector<T>( initsize ) )
{
}

template<class T>
DBArray<T>::DBArray( const int initsize ) :
writeable( true ),
size( 0 ),
marray( new vector<T>( initsize ) )
{
}

template<class T>
DBArray<T>::DBArray( SmiRecordFile *parrays, const SmiRecordId& id, const bool update ) 
{
  assert( false );
}

template<class T>
DBArray<T>::~DBArray()
{
  delete marray;  
}

template<class T>
void DBArray<T>::Clear()
{
  marray->clear();
  size = 0;
}

template<class T>
void DBArray<T>::Put(const int index, const T& elem)
{
  assert ( writeable );
  
  if ( size <= index ) 
  {
    size = index + 1;
    marray->resize( size );
  }

  (*marray)[index] = elem;
}


template<class T>
void DBArray<T>::Get(int const index, T& elem)
{
  assert ( 0 <= index && index < size );

  elem = (*marray)[index];
}

template<class T>
void DBArray<T>::MarkDelete() 
{
  assert( writeable );
}


template<class T>
const int DBArray<T>::Size() const
{
  return size;
}


template<class T>
const SmiRecordId DBArray<T>::Id() const 
{ 
  return 0;
}

#endif

