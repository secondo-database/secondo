/*

//paragraph	[10]	title:		[{\Large \bf ] [}]

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

//[<=] [\leq]

//[#]  [\neq]

//[tilde] [\verb|~|]

//[star]  [\verb|*|]

//[cppref] [\verb|[]|]



1 Header File: Compact Table


February 1994 Gerd Westerman

November 1996 RHG, Revision.

January 2002 Ulrich Telle, Port to C++.

November 2002 M. Spiekermann, method reportVectorSizes added.

Jan - May 2003 M. Spiekermann, CTable implemented on top of the PArray template. 


1.1 Concept


A compact table is a sequence of elements of equal size indexed by natural
numbers starting from 1. Whereas a list offers only sequential access, a
table offers random access to all its elements.


		Figure 1: Concept of a compact table [CompactTable.eps]


A compact table also provides the storage for its elements. 
It can be used in several ways: 
The first is like an array in programming languages by selecting a component 
by index or assigning a value to a component. The second way of using it is as 
a container for a set of elements retrievable by index. In that case it doesn't 
matter under which index an element is kept. For the latter purpose, the table 
maintains a record of which of its slots are filled or empty, respectively, and 
is also able to extend its size automatically when all slots are filled. The

third way is writing and reading it sequentially, like a list.

[24]	Creation/Removal & Size info & Element access & Managing a set	\\ 

	[--------]

	CTable		 & Size	     & [cppref] const & IsValid		\\

	[tilde]CTable	 & NoEntries & [cppref]	      & EmptySlot	\\

			 &	     &  	      & Add		\\

			 &	     &  	      & Remove		\\



[23]	Iterator   & Scanning	    & Persistence	\\

	[--------]

	Iterator   & ++	            & Load (not implemented yet) \\

	Begin      & EndOfScan	    & Save (not implemented yet) \\

	End	   & GetIndex       &	\\

	operator== & operator[star] &	\\

	operator!= & operator=	    &	\\	


The CTable has two implementations. The first is on top of the standard vector template and
the second is build up with the PArray template class, an array implementation which uses
a variable length SmiRecord to store the elements. 

You can request the persistence version with use of the preprocessor directive
#define CTABLE_PERSISTENT before #include "CTable.h". But there are some inevitable limitations:

  * operator[star] can be used only in an rvalue.

  * [cppref] can also be used only in an rvalue.

For example if 'it' is an iterator for an CTable<int> object, then [star]it = 5 will cause a compiler
error message. The same holds for [cppref] at the left side of an assignment. You have to substitute
those expressions by code fragments like this:

----   int elem = Expression; myTable.Put(pos, elem);

----
       
In order to write code that is independent from the CTABLE_PERSISTENT switch don't use
the [star] and [cppref] operations as lvalue.

The in-memory version will have some dummy functions, because some methods only make sense
for the persistence version. The Code is organized in three files. This file contains all
declarations and the implementation of code which is independent from the vector or PArray
classes. The file CTable.cpp is included for the vector based implementation and the file PCTable.cpp
contains code for the PArray version. 

1.2 Imports, Types

*/



#ifndef CTABLE_H
#define CTABLE_H

#include <assert.h>
#include <sstream>
#include <iostream>
#include <string>


#ifdef CTABLE_PERSISTENT
#include "PagedArray.h"
#else
#include <vector>
typedef unsigned long Cardinal;
#endif


using namespace std;

/**************************************************************************

1.3 Class "CTable"[1]


*/



template <typename T> 

class CTable
{

public:

/**************************************************************************

3.1.1 Construction and destruction

*/

#ifndef CTABLE_PERSISTENT
  CTable( Cardinal const count );
#else
  CTable( Cardinal const count, SmiRecordFile* ptr2RecFile = 0);
#endif

   string MemoryModel();
/* 

Creates a table with ~count~ slots. If the table
needs to grow it does so automatically. The second constructor
creates an handle for an CTable which is already stored on disk.
The flag indicates if the CTable is read-only or not. It was not
possible to declare it as default argument, because SmiRecordId and
Cardinal have the same typdefs.

MemoryModel returns the values "PERSISTENT" or "NON-PERSISTENT".

*/

  ~CTable(); 

/* 

Destroys ~CTable~, releasing its storage.

*/

/************************************



3.2.1 Size Info

*/

  Cardinal Size();
  void totalMemory(Cardinal &mem, Cardinal &pageChanges, Cardinal &slotAccess );

/* 

Size() returns the size (number of slots) of the table. The method
totalMemory() calculates the allocated memory via pointer arithmetic. 
The number of page changes and slot accesses are interesting when using
the persistent implementation.

*/

  Cardinal NoEntries();

/* 

Returns the highest valid index (the largest index of a filled slot).
In particular useful when the table is filled sequentially. 



*/

/************************************



3.3.1 Accessing Elements

The following ~Select~ operations get the address of a slot for
reading, or writing its contents. After writing into a slot it is
considered valid and changed. If used as a lvalue the slot is marked
valid. 

*/

  void Get( Cardinal const n, T& elem );
  void Put( Cardinal const n, T& elem );

#ifdef CTABLE_PERSISTENT
  const T& operator[]( Cardinal n );
#else
  const T& operator[]( Cardinal n ) const;
  T& operator[]( Cardinal n );
#endif
  
  
/* 

``Select for reading''. Returns the address of the slot with index ~index~. 

*Precondition*: "1 [<=] index [<=] Size(Table)"[1] and slot ~index~ must be valid.


``Select for writing''. Returns the address of the slot with index ~index~.
Makes slot ~index~ valid and marks it as changed. In the persistent implementation
only the Put method can be used to write a value into a slot. 

Warning: If you want to write code that can be used with both implementations you 
must always use the following sequence of operations, to ~write~ values into a slot:

index = EmptySlot() 
Get(index, Record)
change Record values
Put(index, Record)

Be careful: references to Records are invalid after the next call of the Get method. 

*Precondition*: "1 [<=] Index [<=] Size (Table)".

*/

/************************************

3.4.1 Managing a Set


*/

  bool IsValid( Cardinal const index );

/* 

Determines whether slot ~index~ is valid. 

*Precondition*: "1 [<=] index [<=] Size(CTable)".


*/

  const Cardinal EmptySlot();

/* 

Returns the index of an empty slot. If necessary (because the table is
full) the table is made larger before returning the index. 

*NOTE*: An initial sequence of ~EmptySlot~ operations (before any ~Remove~
operations) returns always the empty slot with the lowest index. Hence,
one can fill a table sequentially by a sequence of operations of the form

   

----    i := Empty Slot (t); t[i] := p; ... <fill entry>

----


After ~Remove~ operations this is not guaranteed any more (in contrast to
earlier definitions of compact tables!).

*/

  const Cardinal Add( const T& element );

/* 

Copies the element referenced by ~element~ into some empty slot of ~Table~,
which may be automatically extended for this purpose, and returns the index
where the element was put.

Provided for convenience; is the same as:
     

----    i := EmptySlot (Table); Table\[i\] := element; RETURN i;

----

An initial sequence of ~Add~ operations (before any ~Remove~ operations and without ~Remove~ or ~EmptySlot~ in between) is guaranteed to fill the table sequentially and hence maintains the order of insertions. 

*/

  void Remove( Cardinal const index );

/* 

Makes slot ~Index~ empty (no more valid). 

*Precondition*: "1 [<=] Index [<=] Size (Table)".


*/

/***************



1.1 Scan Operations


Iterators allow to scan through this ~CTable~ by enumerating
only the valid slots in increasing order.


*/

  class Iterator;            // Declaration required

  friend class Iterator;     // Make it a friend

  class Iterator             // Definition
  {

   public:

    Iterator();

/*

Creates a default iterator. The iterator can't be used before an initialized
iterator is assigned.

*/

    Iterator( const Iterator& other );

/*

Creates a copy of iterator ~other~.


*/

//    Iterator( const Iterator& other, bool );

    Iterator& operator++();           // Prefix ++

    const Iterator operator++( int ); // Postfix ++

/* 

Advances the scan to the next element. No effect if ~EndOfScan~ holds
before. ~EndOfScan~ may become true.

*/

#ifdef CTABLE_PERSISTENT
    const T& operator*() const;
#else
    T& operator*() const;
#endif
/*

Dereferencing of an iterator in the persistent implementation yields an
const reference to T. So use as lvalue is not supported.

*/

    Iterator& operator=( const Iterator& other );

    bool operator==( const Iterator& other ) const;

/*

Copy Constructor and Comparison. The second
compares two iterators and returns ~true~ if they are equal.

*/

    bool operator!=( const Iterator& other ) const;

/*

Compares two iterators and returns ~true~ if they are not equal.


*/

    Cardinal GetIndex() const;

/* 

Returns the index in the table of the current scan element.
The element may then be accessed by the table operations.


*/

    bool EndOfScan() const;

/* 

True if the scan is at position ~end~ (no more elements present). 

*/

   private:

    Iterator( CTable<T>* ctPtr );

/*

Creates an iterator for the CTable referenced by ~ctPtr~ pointing to the
first valid slot.


*/

    Iterator( CTable<T>* ctPtr, bool );

/*

Creates an iterator for the CTable referenced by ~ctPtr~ pointing beyond the
highest valid slot. Such an iterator can be used to mark the end of a scan.


*/

    CTable<T>* ct;          // referenced Compact Table

    Cardinal current;       // current iterator position

    friend class CTable<T>;

  };

  Iterator Begin();

/* 

Creates an iterator for this ~CTable~, pointing to the first valid slot.

*/

  Iterator End();

/* 

Creates an iterator for this ~CTable~, pointing beyond the last valid slot.

*/

/*********************************************************************

1.7.1 Serialization

*NOTE*: The methods ~Load~ and ~Save~ for support of serialization are currently
not implemented. Since the class ~CTable~ is based on the C++ template mechanism
the type of the slots of compact table could be any C++ class. To support
serialization it would be necessary to have a common base class to all classes
used as template types. This would be quite limiting.


*/

//  bool Load( string const fileName );

/*

Loads a table from file ~fileName~. 

   If the return value is "false"[4] something went wrong.


*/

//  bool Save( string const fileName );

/* 

Saves the ~Compact Table~ to file ~fileName~. 


1.7.1 Private Members

*/


private:

#ifdef CTABLE_PERSISTENT

  void initialize(); // Define default values;
 
  struct ObjectState {    // Reflects the state of an instance. Used for persistence.
  
   SmiRecordId tableId;   // Record ids of stored PArray objects.
   SmiRecordId validId;
   SmiFileId fileId;
   Cardinal elemCount;   // state of the CTable
   Cardinal leastFree;
   Cardinal highestValid;	
  };
 
  SmiRecordFile* ptr2RecFile;  // Create anonymous SmiFile if needed.
  bool doRecFilePtrDelete;

  bool setFALSE;     // variables needed for the PArray.Put(int index, T& elem) method
  bool setTRUE;
  T* dummyElem;

  PagedArray<T>* table;        // Array of table elements
  PagedArray<bool>* valid;     // Array of table element states

  Cardinal elemCount;      // Size of compact table
  Cardinal leastFree;      // Position of free slot
  Cardinal highestValid;   // Position of highest valid slot
 
#else

  std::vector<T> table;       // Array of table elements
  std::vector<bool> valid;    // Array of table element states

  Cardinal elemCount;      // Size of compact table
  Cardinal leastFree;      // Position of free slot
  Cardinal highestValid;   // Position of highest valid slot
  

#endif
};



/*

1.1.1 implementation of common parts

*/


#ifdef CTABLE_PERSISTENT
/*

1.1 Size of a CTable

*/

template<typename T>

Cardinal
CTable<T>::Size() {

  return elemCount;
}

/*

1.1 Number of entries in a CTable

*/

template<typename T>

Cardinal
CTable<T>::NoEntries() {

  return highestValid;
}

/*

1.1 Creation of a Begin iterator

*/

template<typename T>

typename CTable<T>::Iterator
CTable<T>::Begin() {

  return CTable<T>::Iterator( this );
}

/*

1.1 Creation of an End iterator

*/

template<typename T>

typename CTable<T>::Iterator
CTable<T>::End() {

  return CTable<T>::Iterator( this, false );
}


/*

1.1 Default constructor for iterator

*/

template<typename T>

CTable<T>::Iterator::Iterator() : ct(0), current(0) 
{
}

template<typename T>

CTable<T>::Iterator::Iterator( CTable<T>* ctPtr, bool ) {

  ct = ctPtr;
  current = ct->highestValid;
}


template<typename T>

CTable<T>::Iterator::Iterator( Iterator const &other ) {

  ct = other.ct;
  current = other.current;
}



/*

1.1 Iterator assignment

*/

template<typename T>

typename CTable<T>::Iterator&
CTable<T>::Iterator::operator=( CTable<T>::Iterator const &other ) {

  ct = other.ct;
  current = other.current;

  return *this;
}


/*

1.1 Iterator comparison (equality and inequality)

*/

template<typename T>

bool
CTable<T>::Iterator::operator==( const Iterator& other ) const {

  return (ct == other.ct) && (current == other.current);
}


template<typename T>

bool
CTable<T>::Iterator::operator!=( const Iterator& other ) const {

  return (ct != other.ct) || (current != other.current);
}

/*

1.1 Index of element iterator is pointing to

*/

template<typename T>

Cardinal
CTable<T>::Iterator::GetIndex() const {

  assert( ct != 0 );
  
  return current+1;
}

/*

1.1 Test for end of scan

*/

template<typename T>

bool
CTable<T>::Iterator::EndOfScan() const {

  assert( ct != 0 );

  return current >= ct->highestValid;
}

/*

1.1.1 Inclusion of the implementation dependent parts

*/

#endif

#ifdef CTABLE_PERSISTENT
#include "PCTable.cpp"
#else
#include "CTable.cpp"
#endif


#endif

