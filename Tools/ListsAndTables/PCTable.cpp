/*
1 Implementation File: Compact Table - Code for the Disk-Memory Implementation


Jan - May 2003 M. Spiekermann, Code was splitted into the two files CTable.cpp and PCTable.cpp.

August 2003 M. Spiekermann, Constructor changed. 

June 2004 M. Spiekermann, Operator [] changed.


Note: Since debugging of template classes is complicated, many instructions for
printing information at the display are included in this code. Comment it out if you need
this for bug-fixing. There is also some code which may be used for constructing a CTable
object from a stored Record on disk. Currently there is no need for this, but the code
presented here may be used as a first draft when implementing such a constructor. 

*/



/*

1.1 Constructor/Destructor of a CTable 

*/

#include <typeinfo>
#include "WinUnix.h"


template<typename T>

CTable<T>::CTable(  Cardinal const count ) : 
 setFALSE(false),
 setTRUE(true),
 dummyElem(new T()),
 isPersistent(true),
 elemCount(0), 
 leastFree(1),  
 highestValid(0)
{

  //cout << endl << "### CTable<T>::CTable(" << endl 
  //             << "ptr2RecFile: " << (void*) ptr2RecFile << endl;

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
CTable<T>::totalMemory( Cardinal &mem, Cardinal &pageChanges, Cardinal &slotAccess ) {
 
 T* ptrT = 0;
 bool* ptrb = 0;
 
 // calculation of allocated memory	 
 long dT = ((long)++ptrT); 
 long db = ((long)++ptrb);
  
 mem = (Cardinal)((dT + db) * elemCount);
 pageChanges = table->PageChanges(); 
 slotAccess = table->SlotAccess();
}


/*

1.1  Write to disk.

*/

template<typename T>

void
CTable<T>::Put( Cardinal const n,  T& elem ) {

  assert( n > 0 && n <= elemCount );

  if ( n == leastFree ) { // find the next free slot

    bool isvalid = false;
    do {
      ++leastFree;      
      valid->Get(leastFree-1, isvalid);
    }
    while ( leastFree <= elemCount && isvalid );
  }

  valid->Put(n-1, setTRUE);
  table->Put(n-1, elem);

  if (highestValid < n) {
    highestValid = n;
  }
  
}



/*

1.1 Read from Disk

*/

template<typename T>

void
CTable<T>::Get( Cardinal const n, T& elem ) {

  assert( n > 0 && n <= elemCount );

  table->Get(n-1, elem);
}


/*

1.1 For convenience: Access of an element as an rvalue

*/

template<typename T>

const T&
CTable<T>::operator[]( Cardinal n ) {

  static Cardinal lastIndex = 0;
  static T elem;

  if ( !(n > 0 && n <= elemCount) ) {
     cout << "An instance of CTable<" << typeid(elem).name() 
          << "> called operator[" << n << "]"
          << "but ElemCount is " << elemCount << endl;
     assert( n > 0 && n <= elemCount );
  }

  if ( lastIndex != n ) { // check if call of Get() is necessary
    table->Get(n-1, elem);
    lastIndex = n;
  }

  return elem;
}



/*

1.1 Check whether an element is valid

*/

template<typename T>

bool
CTable<T>::IsValid( Cardinal const index ) {

  assert( index > 0 && index <= elemCount );

  bool test;
  valid->Get(index-1, test); 
  //cerr << "### IsValid : " << " test = " << test << endl;
  
  return test;
}


template<typename T>

const Cardinal
CTable<T>::EmptySlot() {

  //cerr << "### EmptySlot: " << "leastFree, elemCount --> " << leastFree << "," << elemCount << endl;
  if ( leastFree > elemCount ) {
	  
    elemCount++;
    valid->Put(elemCount, setFALSE);
    table->Put(elemCount, *dummyElem);
  }
  //cerr << "### EmptySlot: " << "leastFree, elemCount --> " << leastFree << "," << elemCount << endl; 
  return leastFree;
}



template<typename T>

const Cardinal
CTable<T>::Add( const T& element ) {

  Cardinal index = EmptySlot();

  T elem = element;
  valid->Put(index-1, setTRUE);
  table->Put(index-1, elem);

  if ( index == leastFree ) {

    bool isvalid=false;
    do {
	    
      ++leastFree;
      valid->Get(leastFree-1, isvalid);
    }
    while ( leastFree <= elemCount && isvalid );
  }

  if (highestValid < index) {
    highestValid = index;
  }

  return index;

}


template<typename T>

void
CTable<T>::Remove( Cardinal const index ) {

  assert( index > 0 && index <= elemCount );
 
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
  CTable<T>::Iterator temp( *this );

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
