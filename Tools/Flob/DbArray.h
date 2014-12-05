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

1 Header File: DbArray

Version: 0.7

August 2002 RHG

April 2003 Victor Almeida chaged the implementation to not use templates.

June 2010 Christian Duentgen: The Flob interface and implementation has been
changed entirely by Thomas Behr. As a consequence, the functions Put() and Get()
now expect the target buffer to be provided (and deleted) by the caller.

1.1 Overview

This module offers a generic persistent array implemented on top of the
~Flob~ interface.

1.2 Interface methods

This module offers the following methods:

[23]    Creation/Removal        & Access        & Inquiries     \\
        [--------]
        DbArray                 & Get           & NoComponents  \\
        [tilde]DbArray          & Put           & Id            \\
        Destroy                 & resize        & GetFlobSize   \\
                                & Append        & GetUsedSize   \\
                                & clean         & GetElemSize   \\
                                & Find          & GetCapacity   \\
                                & Sort          &               \\
                                & Restrict      &               \\
                                & copyTo        &               \\
                                & copyFrom      &               \\

Operations have to follow the protocol shown below:

                Figure 1: Protocol [Protocol.eps]

1.3 Class ~DbArray~SetPersistentCache

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

Creates a DbArray with a given capacity. Use this instead of the standard
constructor to create a new ~DbArray~!

*/
   inline DbArray( const int n ):
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

Changes the capacity of the DbArray.

*/

bool resize( const SmiSize& newSize ){
   if(newSize==0){
    return clean();
   }

   if( newSize < nElements ){ // reduce size
        nElements = newSize;
   }

   maxElements = newSize;
   return Flob::resize( newSize * sizeof( DbArrayElement ) );
}

virtual bool clean() {
  nElements = 0;
  maxElements = 0;
  return Flob::clean();
}

inline bool Destroy() {
  nElements = 0;
  maxElements = 0;
  return Flob::destroy();
}




inline bool Append( const DbArrayElement& elem ) {
  return Put( nElements, elem );
}

inline bool Append(const DbArray<DbArrayElement>& a){
  if( a.nElements == 0){ // nothing to do
    return true;
  }
  if(nElements + a.nElements > maxElements){
    // resize of the underlying Flob required
    if(!resize((nElements + a.nElements)*2)){
       return false;
    }
  }
  // copy the FlobData
  size_t size = a.nElements * sizeof(DbArrayElement);
  char* buffer = new char[size];
  if(!a.Flob::read(buffer, size, 0)){
     return false;
  }
  bool ok = Flob::write(buffer, size, (nElements)*sizeof(DbArrayElement));
  delete [] buffer;
  nElements += a.nElements;
  return ok;
}

/*
This function copies a range from this DbArray into dest. The destOffset must
be less than or equal to the size of dest. That is, it allows only for
overwriting dest elems or appending to them, such that no gaps of uninitialized
values occur within dest. If the size of dest doesn't fit the copied range, it
will be resized automatically.

*/
inline bool copyTo(DbArray<DbArrayElement>& dest, int sourceOffset,
    int numberOfElems, int destOffset)
{
  if( numberOfElems == 0)  { // nothing to do
    return true;
  }
  if( numberOfElems < 0 || sourceOffset < 0 || destOffset < 0 ||
      nElements < sourceOffset + numberOfElems ||
      destOffset > dest.nElements) { //wrong arguments
    return false;
  }

  if(dest.maxElements < destOffset + numberOfElems){
    // resize of the destination Flob
    if(!dest.resize(destOffset + numberOfElems)){
       return false;
    }
  }
  // copy the FlobData
  size_t size = numberOfElems * sizeof(DbArrayElement);
  size_t readOffset = sourceOffset * sizeof(DbArrayElement);
  char* buffer = new char[size];
  if(!Flob::read(buffer, size, readOffset)){
     return false;
  }
  size_t writeOffset = destOffset * sizeof(DbArrayElement);
  bool ok = dest.Flob::write(buffer, size, writeOffset);
  delete [] buffer;
  dest.nElements = (dest.nElements > (numberOfElems + destOffset))?
      dest.nElements: (numberOfElems + destOffset);
  return ok;
}



/*
~Put~

Puts elem to the given position in the array. If the array is not
large enough, the array is growed automatically.

*/
bool Put( const int index, const DbArrayElement& elem ) {
  if(index >= nElements){ // put a new element
    nElements = index + 1;
    if(index >= maxElements){  // underlying flob is too small
       maxElements =  index + 1;
       if(maxElements <=9){
          maxElements = 16;
       } else {
          maxElements = maxElements * 2;
       }
       if(!Flob::resize( maxElements * sizeof( DbArrayElement ) )){
         return false;
       }
   }
  }
  return Flob::write((char*)&elem,                   // buffer
              sizeof(DbArrayElement),  // size
              index * sizeof(DbArrayElement)); // offset
 }




/*
~Get~

Returns the element stored at position __index__
within the array.

*/
 inline bool Get( int index, DbArrayElement* elem ) const{
   assert( index >= 0 );
   assert( index < nElements );

   if(!Flob::read((char*)elem, sizeof(DbArrayElement),
              index*sizeof(DbArrayElement))){
     return false;
   }
   elem = (new ((void*)elem) DbArrayElement);
   return true;
 }

 inline bool Get( int index, DbArrayElement& elem ) const{
   return Get(index, &elem);
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

bool TrimToSize() {
   bool ok = true;
   if (maxElements > nElements) {
     if (nElements == 0) {
         ok = Flob::clean();
     } else {
        ok = Flob::resize(nElements * sizeof(DbArrayElement));
     }
   }
   maxElements = nElements;
   return ok;
}


/*
~Sort~

Sorts this array according to the given comparison function.
In the current state, we assume that the complete Flob can be
hold in main memory.

*/

bool Sort( int (*cmp)( const void *a, const void *b) ) {
   if( nElements <= 1 ){
      return true;
   }
   uint32_t size = nElements * sizeof(DbArrayElement);
   char* buffer = new char[size];
   if(!Flob::read(buffer, size, 0)){
     delete [] buffer;
     return false;
   }
   qsort( buffer, nElements, sizeof( DbArrayElement ), cmp );
   bool res =  Flob::write(buffer, size, 0);
   delete[] buffer;
   return res;
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
  if( cmp( key, &elem ) < 0 ) {
    result = 0;
    return false;
  }
  // check if key is larger than the largest element
  Get( nElements - 1, elem );
  if( cmp( key, &elem ) > 0 ) {
     result = nElements;
     return false;
   }

   int first = 0, last = nElements - 1, mid;

   while (first <= last) {
     mid = ( first + last ) / 2;
     Get( mid, &elem );
     if( cmp( key, &elem ) > 0 ){
       first = mid + 1;
     } else if( cmp( key, &elem ) < 0 ) {
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
Restricts the DbArray to the interval set of indices passed as argument.

*/
virtual bool Restrict( const vector< pair<int, int> >& intervals,
                       DbArray<DbArrayElement>& result ) const{
  // compute the result size
  unsigned int newSize = 0;
  for( vector< pair<int, int> >::const_iterator it= intervals.begin();
           it < intervals.end();
           it++ ) {
      assert( it->first <= it->second );
      newSize += ( ( it->second - it->first ) + 1 ) * sizeof( DbArrayElement );
  }
assert( newSize <= Flob::getSize() );
DbArray<DbArrayElement> res(newSize/sizeof(DbArrayElement));
if( newSize == 0 ){
  result = res;
  return true;
} else {
  char *buffer = (char*)malloc( newSize );
  size_t offset = 0;
  DbArrayElement e;
  // copy value into the temporarly buffer
  for( vector< pair<int, int> >::const_iterator it = intervals.begin();
       it < intervals.end();
       it++ ) {
    for( int j = it->first; j <= it->second; j++ ) {
       if(!Get( j, &e )){
          return false;
       }
       memcpy( buffer + offset,(char*) &e, sizeof( DbArrayElement ) );
       offset += sizeof( DbArrayElement );
    }
  }
  res.nElements = newSize / sizeof( DbArrayElement );
  res.maxElements = nElements;
  if(!res.resize(nElements)){
    free(buffer);
    result = res;
    return false;
  }
  bool bres = res.Flob::write(buffer, newSize, 0);
  result = res;
  free(buffer);
  return bres;
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

Returns the total size occupied by the managed elements.

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

Saves header information (this) to a record.

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
 ReadVar<SmiSize>(nElements, buffer, offset);
 ReadVar<SmiSize>(maxElements, buffer, offset);
}


 bool copyFrom(const DbArray<DbArrayElement>& src){
    bool res = Flob::copyFrom(src);
    nElements = src.nElements;
    maxElements = src.maxElements;
    return res;
 }

 bool copyTo(DbArray<DbArrayElement>& dest){
    bool res = Flob::copyTo(dest);
    dest.nElements = nElements;
    dest.maxElements = maxElements;
    return res;
 }

 bool copyFrom(const Flob& src){
   DbArray<DbArrayElement> const* dbsrc =
              static_cast<DbArray<DbArrayElement> const* >(&src);
   return copyFrom(*dbsrc);
 }

 bool copyTo(Flob& dest) const{
   DbArray<DbArrayElement>* dbdest =
                    static_cast<DbArray<DbArrayElement>* >(&dest);
   return copyTo(*dbdest);
 }


class const_iterator{
 friend class DbArray;
 public:

     const_iterator(const const_iterator& other):
       dbarray(other.dbarray), 
       position(other.position), 
       elem(other.elem) {}

     const_iterator& operator=(const const_iterator& other){
        dbarray = other.dbarray;
        position = other.position;
        elem = other.elem;
        return *this;
     }

     const_iterator& operator++(){
        if(dbarray){
          if(position<dbarray->Size()){
            position++;
            if(position<dbarray->Size()){
                dbarray->Get(position,elem);
            }
          } else { // end() reached
             position = dbarray->Size();
          }
        } else { // single element iterator
           if(position<=0){ // 0 or -1 allowed
              position++; 
           } 
        } 
        return *this;
     }
     
     const_iterator& operator++(int dummy){
        if(dbarray){
          if(position<dbarray->Size()){
            position++;
            if(position<dbarray->Size()){
                dbarray->Get(position,elem);
            }
          } else { // end() reached
             position = dbarray->Size();
          }
        } else { // single element iterator
           if(position<=0){ // 0 or -1 allowed
              position++; 
           } 
        } 
        return *this;
     }

     const_iterator& operator--(){
        if(dbarray){
          if(position>=0){
            position--;
            if(position>=0 && position<dbarray->Size()){
               dbarray->Get(position, elem);
              }
            } else {
              position = -1;
            }
          } else {
            if(position >=0){
                position--;
            }
          }
          return *this;
       }
     

       const_iterator& operator--(int dummy){
        if(dbarray){
          if(position>=0){
            position--;
            if(position>=0 && position<dbarray->Size()){
               dbarray->Get(position, elem);
              }
            } else {
              position = -1;
            }
          } else {
            if(position >=0){
                position--;
            }
          }
          return *this;
       }

       const DbArrayElement& operator*() const{
         if(dbarray){
           assert(position>=0);
           assert(position < dbarray->Size());
           return elem; 
         } else {
           assert(position==0);
           return elem;
         }
       }

       bool operator==(const const_iterator& other) const{
          return position == other.position;
       }

       bool operator<(const const_iterator& other) const{
          return position < other.position;
       }
       bool operator>(const const_iterator& other) const{
          return position > other.position;
       }
       bool operator!=(const const_iterator& other) const{
          return position != other.position;
       }
       bool operator<=(const const_iterator& other) const{
          return position <= other.position;
       }
       bool operator>=(const const_iterator& other) const{
          return position >= other.position;
       }

   private:
       const DbArray<DbArrayElement>* dbarray;
       int position;
       DbArrayElement elem;

       const_iterator(const DbArray<DbArrayElement>* dbarray_, 
                      const int position_ = 0 ):
         dbarray(dbarray_), position(position_)  { 
         if(position<0){
            position=-1;
         } else if(position < dbarray->Size()){
            dbarray->Get(position,elem);
         } else {
           position = dbarray->Size();
         }
       }

       const_iterator(const DbArrayElement& elem_):
         dbarray(0),position(0), elem(elem_) {} 



 };



  const_iterator begin() const{
     const_iterator it(this);
     return it;
   }
 
   const_iterator end() const{
      const_iterator it(this,Size());
      return it;
   }

   static const_iterator elem_iter_begin(const DbArrayElement& elem){
      const_iterator it(elem);
      return it;
   }
   static const_iterator elem_iter_end(const DbArrayElement& elem){
      const_iterator it(elem);
      it++;
      return it;
   }


  private:

    SmiSize nElements;
/*
Stores the number of elements currently managed (contained) by the array.

*/
    SmiSize maxElements;
/*
Stores the total number of elements that can currently could be managed by
this array (without resizing it).

*/
};

#endif //DBARRAY_H

