#include <assert.h>
#include <sstream>
#include <iostream>

using namespace std;

/*

1.1 Constructor/Destructor of a CTable 



*/

template<typename T>

CTable<T>::CTable( Cardinal const count )

{

  assert( count > 0 );

  table.resize( count );

  valid.resize( count );

  for ( Cardinal j = 0; j < count; j++ )

  {

    valid[j] = false;

  }

  elemCount    = count;

  leastFree    = 1;

  highestValid = 0;

}



template<typename T>

CTable<T>::~CTable()

{

}



/*

1.1 Size of a CTable



*/

template<typename T>

Cardinal

CTable<T>::Size()

{

  return elemCount;

}

template<typename T>

Cardinal
CTable<T>::totalMemory() {
 
 T* ptrT = 0;
 bool* ptrb = 0;
 
 // calculation of allocated memory	 
 long dT = ((long)++ptrT); 
 long db = ((long)++ptrb);
  
 return (Cardinal)(dT * table.capacity()) + (db * valid.capacity());
}


/*

1.1 Number of entries in a CTable



*/

template<typename T>

Cardinal

CTable<T>::NoEntries()

{

  return highestValid;

}



/*

1.1 Access of an element as an lvalue



*/

template<typename T>

T&

CTable<T>::operator[]( Cardinal n )

{

  assert( n > 0 && n <= elemCount );

  if ( n == leastFree )

  {

    do

    {

      ++leastFree;

    }

    while ( leastFree <= elemCount && valid[leastFree-1] );

  }

  valid[n-1] = true;

  if (highestValid < n)

    highestValid = n;

  return table[n-1];

}



/*

1.1 Access of an element as an rvalue



*/

template<typename T>

const T&

CTable<T>::operator[]( Cardinal n ) const

{

  assert( n > 0 && n <= elemCount );

  return table[n-1];

}



/*

1.1 Check whether an element is valid



*/

template<typename T>

bool

CTable<T>::IsValid( Cardinal const index ) const

{

  assert( index > 0 && index <= elemCount );

  return valid[index-1];

}



template<typename T>

const Cardinal

CTable<T>::EmptySlot()

{

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

  }

  return leastFree;

}



template<typename T>

const Cardinal

CTable<T>::Add( const T& element )

{

  Cardinal index = EmptySlot();

  valid[index-1] = true;

  table[index-1] = element;

  if ( index == leastFree )

  {

    do

    {

      ++leastFree;

    }

    while ( leastFree <= elemCount && valid[leastFree-1] );

  }

  if (highestValid < index)

    highestValid = index;

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

1.1 Creation of a Begin iterator



*/

template<typename T>

CTable<T>::Iterator

CTable<T>::Begin()

{

  return CTable<T>::Iterator( this );

}



/*

1.1 Creation of an End iterator



*/

template<typename T>

CTable<T>::Iterator

CTable<T>::End()

{

  return CTable<T>::Iterator( this, false );

}



/*

1.1 Default constructor for iterator



*/

template<typename T>

CTable<T>::Iterator::Iterator() : ct(0), current(0)

{

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



template<typename T>

CTable<T>::Iterator::Iterator( CTable<T>* ctPtr, bool )

{

  ct      = ctPtr;

  current = ct->highestValid;

}



/*

1.1 Copy constructors for iterator



*/

template<typename T>

CTable<T>::Iterator::Iterator( Iterator const &other )

{

  ct      = other.ct;

  current = other.current;

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

1.1 Iterator assignment



*/

template<typename T>

CTable<T>::Iterator& 

CTable<T>::Iterator::operator=( CTable<T>::Iterator const &other )

{

  ct      = other.ct;

  current = other.current;

  return *this;

}



/*

1.1 Iterator increment (prefix and postfix notation)



*/

template<typename T>

CTable<T>::Iterator&

CTable<T>::Iterator::operator++()

{

  assert( ct != 0 );

  while ( current < ct->highestValid && !ct->valid[++current] );

  return *this;

}



template<typename T>

const CTable<T>::Iterator

CTable<T>::Iterator::operator++( int )

{

  assert( ct != 0 );

  CTable<T>::Iterator temp( *this );

  while (current < ct->highestValid && !ct->valid[++current] );

  return temp;

}



/*

1.1 Iterator comparison (equality and inequality)



*/

template<typename T>

bool

CTable<T>::Iterator::operator==( const Iterator& other ) const

{

  return (ct == other.ct) && (current == other.current);

}



template<typename T>

bool

CTable<T>::Iterator::operator!=( const Iterator& other ) const

{

  return (ct != other.ct) || (current != other.current);

}



/*

1.1 Index of element iterator is pointing to



*/

template<typename T>

Cardinal

CTable<T>::Iterator::GetIndex() const

{

  assert( ct != 0 );

  return current+1;

}



/*

1.1 Test for end of scan



*/

template<typename T>

bool

CTable<T>::Iterator::EndOfScan() const

{

  assert( ct != 0 );

  return current >= ct->highestValid;

}



