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

//paragraph     [10]    title:          [{\Large \bf ] [}]
//paragraph     [11]    title:          [{\large \bf ] [}]
//paragraph     [12]    title:          [{\normalsize \bf ] [}]
//paragraph     [21]    table1column:   [\begin{quote}\begin{tabular}{l}]       [\end{tabular}\end{quote}]
//paragraph     [22]    table2columns:  [\begin{quote}\begin{tabular}{ll}]      [\end{tabular}\end{quote}]
//paragraph     [23]    table3columns:  [\begin{quote}\begin{tabular}{lll}]     [\end{tabular}\end{quote}]
//paragraph     [24]    table4columns:  [\begin{quote}\begin{tabular}{llll}]    [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]     verbatim:       [$]     [$]
//characters    [2]     formula:        [$]     [$]
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

[23]    Creation/Removal        & Access        & Inquiries     \\
        [--------]
        DBArray                 & Get           & NoComponents  \\
        [tilde]DBArray          & Put             & Id                  \\
        MarkDelete                  &                 &                           \\

Operations have to follow the protocol shown below:

                Figure 1: Protocol [Protocol.eps]

1.3 Class ~DBArray~

An instance of the class is a handle to a persistent array of fixed size.

*/

#ifndef DBARRAY_H
#define DBARRAY_H

#include <string.h>
#include <vector>
#include <algorithm>
#include "Flob.h"
#include "Serialize.h"
#include <assert.h>

using namespace std;


template<class DbArrayElement>
class DbArray : public Flob
{
  public:
/*
~Standard constructor~

Does nothing. Should only be used within Cast functions.

*/
    inline DbArray() : Flob() {}

/*
~Constructor~

Creates a DbArray with a given capacity.

*/
   inline DbArray( int n ):
      Flob( n * sizeof( DbArrayElement ) ),
      nElements( 0 ),
      maxElements( n )
      {}

/*
~Destructor~

*/
    inline ~DbArray() {}

/*
~Resize~

Changes the capacity of an DbArray. 

*/

virtual void resize( const int newSize ){
   if(newSize==0){
    clean();
    return;
   }

   if( newSize < nElements ){ // reduce size
        nElements = newSize;
   }

   maxElements = newSize;
   Flob::resize( newSize * sizeof( DbArrayElement ) );
}

virtual void clean() {
  nElements = 0;
  maxElements = 0;
  Flob::clean();
}

inline void Destroy() {
  nElements = 0;
  maxElements = 0;
  Flob::destroy();
}

inline void Append( const DbArrayElement& elem ) {
  Put( nElements, elem );
}


/*
~Put~

Puts elem to the given position in the array. If the array is not
large enough, the array is growed automatically.

*/
void Put( int index, const DbArrayElement& elem ) {
  if(index >= nElements){ // enlarging required
    nElements = index + 1;
    if(nElements <=9){
       maxElements = 16;
    } else {
       maxElements = nElements * 2;
    }
    Flob::resize( maxElements * sizeof( DbArrayElement ) );
  }
  Flob::write((char*)&elem,                   // buffer
              sizeof(DbArrayElement),  // size
              index * sizeof(DbArrayElement)); // offset

 }

/*
~Get~

Returns the element stored at position __index__ 
within the array. 

*/
 inline void Get( int index, DbArrayElement* elem ) const{
   assert( index >= 0 );
   assert( index < nElements );

   Flob::read((char*)elem, sizeof(DbArrayElement),
              index*sizeof(DbArrayElement));
   elem = (new ((void*)elem) DbArrayElement);
 }


/*
~Size~

Returns the number of stored Elements.

*/

inline int Size() const {
  return nElements;
 }

/*
~TrimToSize~

Reduces a DbArray to the capacity to store exacly all stored elements.

*/

void TrimToSize() {
   if (maxElements > nElements) {
     if (nElements == 0) {
         Flob::clean();
     } else {
        Flob::resize(nElements * sizeof(DbArrayElement));
     }
   }
   maxElements = nElements;
}


/*
~Sort~

Sorts this array according to the given comparison function.
In the current state, we assume that the complete Flob can be
hold in main memory.

*/

void Sort( int (*cmp)( const void *a, const void *b) ) {
   if( nElements <= 1 ){
      return;
   }
   uint32_t size = nElements * sizeof(DbArrayElement);
   char buffer[size];
   Flob::read(buffer, size, 0);
   qsort( buffer, nElements, sizeof( DbArrayElement ), cmp );
}


/*
~Find~

Searches (binary search) for a given key in the database array given the ~cmp~
comparison criteria. It is assumed that the array is sorted. The function returns
true if the ~key~ is in the array and false otherwise. The position is returned in
the ~result~ argument. If ~key~ is not in the array, then ~result~ contains the
position where it should be.

*/

bool Find( const void *key,
           int (*cmp)( const void *a, const void *b),
           int& result ) const {

  DbArrayElement elem;

  if(nElements == 0) {
     result = 0;
     return false;
  }

  // check if key is smaller than the smallest element
  Get( 0, &elem );
  if( cmp( key, elem ) < 0 ) {
    result = 0;
    return false;
  }
  // check if key is larger than the largest element
  Get( nElements - 1, elem );
  if( cmp( key, elem ) > 0 ) {
     result = nElements;
     return false;
   }

   int first = 0, last = nElements - 1, mid;

   while (first <= last) {
     mid = ( first + last ) / 2;
     Get( mid, &elem );
     if( cmp( key, elem ) > 0 ){
       first = mid + 1;
     } else if( cmp( key, elem ) < 0 ) {
       last = mid - 1;
     } else {
       result = mid;
       return true;
     }
   }
   result = first < last ? first + 1 : last + 1;
   return false;
 }


/*
Restricts the DBArray to the interval set of indices passed as argument.

*/
virtual void Restrict( const vector< pair<int, int> >& intervals ) {
  // compute the result size
  int newSize = 0;
  for( vector< pair<int, int> >::const_iterator it= intervals.begin();
           it < intervals.end();
           it++ ) {
      assert( it->first <= it->second );
      newSize += ( ( it->second - it->first ) + 1 ) * sizeof( DbArrayElement );
  }
  assert( newSize <= nElements );
  if( newSize == 0 ){
    clean();
  } else {
    char *buffer = (char*)malloc( newSize );
    size_t offset = 0;
    DbArrayElement e;
    // copy value into the temporarly buffer    
    for( vector< pair<int, int> >::const_iterator it = intervals.begin();
         it < intervals.end();
         it++ ) {    
      for( int j = it->first; j <= it->second; j++ ) {
         Get( j, &e );
         memcpy( buffer + offset, &e, sizeof( DbArrayElement ) );
         offset += sizeof( DbArrayElement );
      }  
    }
    nElements = newSize / sizeof( DbArrayElement );
    maxElements = nElements;
    resize(nElements);
    Flob::write(buffer, newSize, 0);
    free(buffer);
  }
}
    
/*
~GetFlobSize~

Returns the capacity of the underlying Flob.

*/    

 
size_t GetFlobSize() const {
      return Flob::getSize();
}       

/*
~GetUsedSize~

Returns the size occupied by elements.

*/
size_t GetUsedSize() const {
  return nElements * sizeof(DbArrayElement);
}       

/*
~GetElemSize~

Returns the size of a single element.

*/
size_t GetElemSize() const {
  return sizeof(DbArrayElement);
}       

/*
~GetCapacity~

Returns the capacity of the array.

*/
size_t GetCapacity() const {
  return maxElements;
}       

/*
~SaveHeader~

Saves  header information (this) to a record.

*/
virtual size_t serializeHeader( char* buffer, SmiSize& offset) const {
   // first save the flob
   SmiSize sz = Flob::serializeHeader(buffer, offset);
   // append nElements and maxElements
   WriteVar<int>(nElements, buffer, offset);   
   WriteVar<int>(maxElements, buffer, offset);
   sz += 2*sizeof(int);
   return sz;
}       

virtual void restoreHeader(char* buffer, 
                    SmiSize& offset)
{
   Flob::restoreHeader(buffer, offset);
   ReadVar<int>(nElements, buffer, offset);    
   ReadVar<int>(maxElements, buffer, offset);
}


   void copyFrom(const DbArray<DbArrayElement>& src){
      Flob::copyFrom(src);
      nElements = src.nElements;
      maxElements = src.maxElements;
   }

   void copyTo(DbArray<DbArrayElement>& dest){
      Flob::copyTo(dest);
      dest.nElements = nElements;
      dest.maxElements = maxElements;
   } 

  private:

    int nElements;
/*
Store the number of elements inserted in the array.

*/
    int maxElements;
/*
Store the total number of elements that can be added to the array.

*/
};

#endif //DBARRAY_H

