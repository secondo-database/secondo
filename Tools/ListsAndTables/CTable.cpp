/*
1 Implementation File: Compact Table - Code for the In-Memory Implementation


Jan - May 2003 M. Spiekermann, Code was splitted into the two files CTable.cpp and PCTable.cpp.
 
*/




#include <assert.h>
#include <sstream>
#include <iostream>

using namespace std;

/*

1.1 Constructor/Destructor of a CTable 

*/



template<typename T>

CTable<T>::CTable( Cardinal const count ) : 
isPersistent(false),
elemCount(count),
leastFree(1),
highestValid(0)
{
  assert( count > 0 );

  CalcSlotSize();

  // initialize vectors
  table.resize( count );
  valid.resize( count );

  for ( Cardinal j = 0; j < count; j++ )
  {
    valid[j] = false;
  }
}


template<typename T>

CTable<T>::~CTable() {}


template<typename T>

void
CTable<T>::TotalMemory( Cardinal &mem, 
                        Cardinal &pageChanges, 
			Cardinal &slotAccess  ) 
{ 
  mem = (Cardinal)(slotSize * elemCount);
  pageChanges = 0;
  slotAccess = 0;
}


/*

1.1 Access of an element as an lvalue

*/

template<typename T>

void
CTable<T>::UpdateSlotCounters(Cardinal const n) {

  if ( n == leastFree ) { // find the next free slot

    do {
      ++leastFree;
    }
    while ( leastFree <= elemCount && valid[leastFree-1] );
  }

  if (highestValid < n) { // set the highest used slot number
    highestValid = n;
  }
}


template<typename T>

T&
CTable<T>::operator[]( Cardinal n ) {

  if ( !((n>0) && (n <= elemCount)) ) {
     cerr << "CTable<" << typeid(T).name() << ">"
          << "slot n=" << n << " is out of range." << endl;
     assert( n > 0 && n <= elemCount );
  }

  UpdateSlotCounters(n);

  valid[n-1] = true;
  return table[n-1];
}


/*

1.1 Access of an element as an rvalue

*/

template<typename T>

const T&
CTable<T>::operator[]( Cardinal n ) const {

  assert( n > 0 && n <= elemCount );
  return table[n-1];
}


template<typename T>

void
CTable<T>::Get( Cardinal const n, T& elem ) {

   elem = (*this)[n];
}

template<typename T>

void
CTable<T>::Put( Cardinal const n, T& elem ) {

   (*this)[n] = elem;
}



/*

1.1 Check whether an element is valid

*/

template<typename T>

bool

CTable<T>::IsValid( Cardinal const index ) {

  assert( index > 0 && index <= elemCount );
  return valid[index-1];
}



template<typename T>

const Cardinal
CTable<T>::EmptySlot()
{
  //static Cardinal last = 0;
  if ( leastFree > elemCount )
  {
    Cardinal newElemCount = 2 * elemCount;
    table.resize( newElemCount );
    valid.resize( newElemCount );

    for ( Cardinal j = elemCount; j < newElemCount; j++ )
    {
      valid[j] = false;
    }
    elemCount = newElemCount;
    //leastFree++;
  }
  
  //assert( leastFree-1 != last );
  //last = leastFree-1;
  return leastFree;
}



template<typename T>

const Cardinal
CTable<T>::Add( T& element )
{
  Cardinal index = EmptySlot();
  (*this)[index] = element;
   
  return index;
}



template<typename T>

void
CTable<T>::Remove( Cardinal const index )
{
  assert( index > 0 && index <= elemCount );

  valid[index-1] = false;

  if ( index < leastFree )
    leastFree = index;

  if ( index == highestValid )
  {
    do
    {
      --highestValid;
    }
    while ( highestValid > 0 && !valid[highestValid-1] );
  }

}



/*

1.1 Constructor for iterator

*/

template<typename T>

CTable<T>::Iterator::Iterator( CTable<T>* ctPtr )
{

  ct      = ctPtr;
  current = 0;

  while ( current < ct->highestValid && !ct->valid[current] )
  {
    ++current;
  }

}


/*

1.1 Dereference of an iterator

*/

template<typename T>

T&
CTable<T>::Iterator::operator*() const
{
  assert( ct != 0 );
  assert( current < ct->elemCount && ct->valid[current] );
  return ct->table[current];
}


/*

1.1 Iterator increment (prefix and postfix notation)

*/

template<typename T>

typename CTable<T>::Iterator&

CTable<T>::Iterator::operator++()
{
  assert( ct != 0 );
  while ( current < ct->highestValid && !ct->valid[++current] );
  return *this;
}



template<typename T>

const typename CTable<T>::Iterator

CTable<T>::Iterator::operator++( int )
{
  assert( ct != 0 );
  CTable<T>::Iterator temp( *this );
  while (current < ct->highestValid && !ct->valid[++current] );
  return temp;
}


