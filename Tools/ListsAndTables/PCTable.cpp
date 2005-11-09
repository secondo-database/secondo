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

1 Implementation File: Compact Table - Code for the Disk-Memory Implementation


Jan - May 2003 M. Spiekermann, Code was splitted into the two files CTable.cpp and PCTable.cpp.

August 2003 M. Spiekermann, Constructor changed. 

June 2004 M. Spiekermann, Operator [] changed.

August 2004 M. Spiekermann. Static variables in Operator [] caused a strange bug 
since its not possible to reinitialize them new private class members have been
introduced.

Note: Since debugging of template classes is complicated, many instructions for
printing information at the display are included in this code. Comment it out if you need
this for bug-fixing.

*/



/*

1.1 Constructor/Destructor of a CTable 

*/

#include <assert.h>
#include <sstream>
#include <iostream>
#include <string>
#include <typeinfo>

#include "WinUnix.h"


template<typename T>

CTable<T>::CTable(  Cardinal const count ) : 
 setFALSE(false),
 setTRUE(true),
 dummyElem(new T()),
 lastAccessedElem(*dummyElem),
 lastAccessedIndex(0),
 isPersistent(true),
 elemCount(0), 
 leastFree(1),  
 highestValid(0)
{
  CalcSlotSize();

  // In this implementation of CTable the total slot size grows one by
  // one, hence count is used to define the initial buffer size.
  // There seems to be some administration data for each record, since 
  // Berkeley-DB cannot create records of the full pagesize.

  table = new PagedArray<T>(WinUnix::getPageSize()-100, count, true);
  valid = new PagedArray<bool>(WinUnix::getPageSize()-100, count, true);

}

template<typename T>

CTable<T>::~CTable() {

  delete dummyElem;
  delete table;
  delete valid;

}


template<typename T>

void
CTable<T>::TotalMemory( Cardinal &mem, 
                        Cardinal &pageChanges, 
			                  Cardinal &slotAccess  ) 
{  
  mem = (Cardinal)(slotSize * elemCount);
  pageChanges = table->PageChanges(); 
  slotAccess = table->SlotAccess();
}


/*

1.1  Write to disk.

*/

template<typename T>

void
CTable<T>::UpdateSlotCounters(Cardinal const n) {

  if ( n == leastFree ) { // find the next free slot

    bool isvalid = false;
    do {
      ++leastFree;      
      valid->Get(leastFree-1, isvalid);
    }
    while ( leastFree <= elemCount && isvalid );
    assert(leastFree <= elemCount + 1);
  }
  
  if (highestValid < n) { // set the highest used slot number
    highestValid = n;
  }
  
}



template<typename T>

void
CTable<T>::Put( Cardinal const n,  T& elem ) {

  if ( OutOfRange(n) ) { assert( 0 ); } 
  UpdateSlotCounters(n);

  valid->Put(n-1, setTRUE);
  table->Put(n-1, elem);
  
}



/*

1.1 Read from Disk

*/

template<typename T>

void
CTable<T>::Get( Cardinal const n, T& elem ) {

  if ( OutOfRange(n) ) { assert( 0 ); }  
  table->Get(n-1, elem);
}


/*

1.1 For convenience: Access of an element as an rvalue

*/

template<typename T>

const T&
CTable<T>::operator[]( Cardinal n ) {

  if ( OutOfRange(n) ) { assert( 0 ); }

  if ( lastAccessedIndex != n ) { // check if call of Get() is necessary
    table->Get(n-1, lastAccessedElem);
    lastAccessedIndex = n;
  }

  return lastAccessedElem;
}



/*

1.1 Check whether an element is valid

*/

template<typename T>

bool
CTable<T>::IsValid( Cardinal const index ) {

  if ( OutOfRange(index) ) { assert( 0 ); }

  bool test;
  valid->Get(index-1, test); 
  //cerr << "### IsValid : " << " test = " << test << endl;
  
  return test;
}


template<typename T>

const Cardinal
CTable<T>::EmptySlot() { 

  static Cardinal last = 0;
  if ( leastFree > elemCount ) { // initialize a new slot
	  
    elemCount++;
    valid->Put(elemCount-1, setFALSE);
    //table->Put(elemCount-1, *dummyElem);
  }
  
  if ( leastFree == last ) { 
     // guarantee that two successive calls get different slots
     // The last slot will be blocked until it is removed.
     valid->Put(last-1, setTRUE);
     UpdateSlotCounters(last);
     elemCount++;    
  }
  
  last = leastFree;
	//cout << "CTable<" << typeid(T).name() << ">::EmptySlot last=" << last << endl;
	
  return last;
}



template<typename T>

const Cardinal
CTable<T>::Add( T& element ) {

  Cardinal index = EmptySlot();
  Put(index, element);
  
  return index;
}


template<typename T>

void
CTable<T>::Remove( Cardinal const index ) {

  cerr << "CTable::Remove called!" << endl;

  if ( OutOfRange(index) ) { assert( 0 ); }
 
  valid->Put(index-1, setFALSE);

  if ( index < leastFree ) {

    leastFree = index;
  }
  
  if ( index == highestValid ) {
    
    bool isvalid = false;
    do {
      --highestValid;
      valid->Get(highestValid-1, isvalid);
    }
    while ( highestValid > 0 && !isvalid );
  }

}



/*

1.1 Constructor for iterator

*/

template<typename T>

CTable<T>::Iterator::Iterator( CTable<T>* ctPtr ) {

  ct = ctPtr;
  current = 0;

  bool isvalid = false;
  ct->valid->Get(current, isvalid);
  while ( current < ct->highestValid && !isvalid ) {
    ++current;
    ct->valid->Get(current, isvalid);
  }
}



/*

1.1 Copy constructors for iterator

*/




/*

1.1 Dereference of an iterator

*/

template<typename T>

const T&
CTable<T>::Iterator::operator*() const {

  assert( ct != 0 );

  bool isvalid = false;
  ct->valid->Get(current, isvalid);
  assert( current < ct->elemCount && isvalid );

  static T elem;
  ct->table->Get(current, elem);
  
  return elem;
}


/*

1.1 Iterator increment (prefix and postfix notation)

*/

template<typename T>

typename CTable<T>::Iterator&
CTable<T>::Iterator::operator++() {

  assert( ct != 0 );

  // increment as long until next slot is valid
  bool isvalid = false;
  do {
     if (current >= ct->highestValid) break;
     current++;
     ct->valid->Get(current, isvalid);
     //cerr << "### it_++prefix: current = " << current << endl;
  }
  while (!isvalid);
  
  return *this;
}



template<typename T>

const typename CTable<T>::Iterator
CTable<T>::Iterator::operator++( int ) {

  assert( ct != 0 );
  typename CTable<T>::Iterator temp( *this );

  // increment as long until next slot is valid
  bool isvalid = false;
  do {
     if (current >= ct->highestValid) break;
     current++;
     ct->valid->Get(current, isvalid);
     //cerr << "### it_++prefix: current = " << current << endl;
  }
  while (!isvalid);

  return temp;
}
