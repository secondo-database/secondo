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

November 1996 RHG Revision

January 2002 Ulrich Telle Port to C++

November 2002 M. Spiekermann, method reportVectorSizes added.

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

	Iterator   & ++	            & Load	\\

	Begin      & EndOfScan	    & Save	\\

	End	   & GetIndex       &	\\

	operator== & operator[star] &	\\

	operator!= & operator=	    &	\\	



1.2 Imports, Types



*/



#ifndef CTABLE_H

#define CTABLE_H



#include <vector>
#include <string>



typedef unsigned long Cardinal;



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

  CTable( Cardinal const count );

/* 

Creates a table with ~count~ slots. If the table

needs to grow it does so automatically.



*/

  ~CTable(); 

/* 

Destroys ~CTable~, releasing its storage.



*/

/************************************



3.2.1 Size Info



*/

  Cardinal Size();
  Cardinal totalMemory();

/* 

Size() returns the size (number of slots) of the table. The method
totalMemory() calculates the allocated memory via pointer arithmetic. 

*/

  Cardinal NoEntries();

/* 

Returns the highest valid index (the largest index of a filled slot).

In particular useful when the table is filled sequentially. 



*/

/************************************



3.3.1 Accessing Elements



The following two ~Select~ operations get the address of a slot for

reading, or writing its contents. After writing into a slot it is

considered valid and changed. If used as a lvalue the slot is marked

valid. 



*/

  const T& operator[]( Cardinal n ) const;

/* 

``Select for reading''. Returns the address of the slot with index ~index~. 



*Precondition*: "1 [<=] index [<=] Size(Table)"[1] and slot ~index~ must be valid.



*/

  T& operator[]( Cardinal n );

/* 

``Select for writing''. Returns the address of the slot with index ~index~.

Makes slot ~index~ valid and marks it as changed. 



*Precondition*: "1 [<=] Index [<=] Size (Table)".



*/

/************************************

3.4.1 Managing a Set



*/

  bool IsValid( Cardinal const index ) const;

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

    T& operator*() const;

    Iterator& operator=( const Iterator& other );

    bool operator==( const Iterator& other ) const;

/*

Compares two iterators and returns ~true~ if they are equal.



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

1.7.1 Persistence



*NOTE*: The methods ~Load~ and ~Save~ for support of persistence are currently

not implemented. Since the class ~CTable~ is based on the C++ template mechanism

the type of the slots of compact table could be any C++ class. To support

persistency it would be necessary to have a common base class to all classes

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

  std::vector<T>    table;     // Array of table elements

  std::vector<bool> valid;     // Array of table element states

  Cardinal elemCount;     // Size of compact table

  Cardinal leastFree;     // Position of free slot

  Cardinal highestValid;  // Position of highest valid slot

};



/*

1.1.1 Inclusion of the implementation of the template class ~CTable~



*/

#include "CTable.cpp"



#endif



