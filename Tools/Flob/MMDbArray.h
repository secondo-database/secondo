/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
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


This is a fake version of DbArray working only in main memory.
This can be used to build main memory versions of temporarly objects.

*/

#ifndef MMDBARRAY_H
#define MMDBARRAY_H

#include <string.h>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "Flob.h"
#include "DbArray.h"


template<class B>
class AComparator{
  public:
     AComparator( int (*cmp__)( const void *a, const void *b) ): cmp(cmp__){}
     bool operator()(B a, B b){
       return cmp(&a,&b)<0;
     }     

  private:
     int (*cmp)( const void *a, const void *b) ;
};



template<class DbArrayElement>
class MMDbArray : public Flob
{
  public:
/*
~Standard constructor~

Does nothing. Should only be used within Cast functions.

*/
    inline MMDbArray()   {}

/*
~Constructor~

Creates a DbArray with a given capacity. Use this instead of the standard
constructor to create a new ~DbArray~!

*/
   inline MMDbArray( const int n ):
      Flob("dummy"), 
      content()
      { }

/*
~Destructor~

*/
    inline ~MMDbArray() {}

/*
~Resize~

Changes the capacity of the DbArray.

*/

bool resize( const SmiSize& newSize ){
   if(newSize==0){
    return clean();
   }
   // decrease size of underlying vector
   while(content.size()>newSize){
      content.pop_back();
   }
   // ignore enlargement
   return true;
}

virtual bool clean() {
  content.clear();
  return true;
}

inline bool Destroy() {
  content.clear();
  return true;
}


inline bool Append( const DbArrayElement& elem ) {
   content.push_back(elem);
   return true;
}

inline bool Append(const MMDbArray<DbArrayElement>& a){
   for(size_t i=0;i<a.content.size();i++){
     content.push_back(a.content[i]);
   }
   return true;
}

/*
This function copies a range from this DbArray into dest. The destOffset must
be less than or equal to the size of dest. That is, it allows only for
overwriting dest elems or appending to them, such that no gaps of uninitialized
values occur within dest. If the size of dest doesn't fit the copied range, it
will be resized automatically.

*/
inline bool copyTo(
    MMDbArray<DbArrayElement>& dest, 
    int sourceOffset,
    int numberOfElems, 
    int destOffset)
{
   for(int i=0;i<numberOfElems;i++){
      size_t si = sourceOffset + i;
      size_t di = destOffset + i;
      DbArrayElement elem = content[si];
      if(di < dest.content.size()){
         dest.content[di] = elem; // overwrite
      } else {
         dest.content.push_back(elem); // append
      }
   }
   return true;
}



/*
~Put~

Puts elem to the given position in the array. If the array is not
large enough, the array is growed automatically.

*/
bool Put( const int index, const DbArrayElement& elem ) {

   assert(index >= 0 && ((size_t)index<=content.size()));
   if((index>=0) && ((size_t)index<content.size())){
     content[index] = elem;
   } else {
     content.push_back(elem);
   }
   return true;
}




/*
~Get~

Returns the element stored at position __index__
within the array.

*/
 inline bool Get( int index, DbArrayElement* elem ) const{
    *elem = content[index];
    return true;
 }

 inline bool Get( int index, DbArrayElement& elem ) const{
   elem = content[index];
   return true;
 }

/*
~Size~

Returns the number of stored Elements.

*/

inline int Size() const {
   return content.size();
 }

/*
~TrimToSize~

Reduces a DbArray to the capacity to store exacly all stored elements.

*/

bool TrimToSize() {
  //content.shrink_to_fit();
  return true;
}


/*
~Sort~

Sorts this array according to the given comparison function.
In the current state, we assume that the complete Flob can be
hold in main memory.

*/

bool Sort( int (*cmp)( const void *a, const void *b) ) {
   sort(content.begin(), content.end(), AComparator<DbArrayElement>(cmp));
   return true;
}


/*
~Find~

Searches (binary search) for a given key in the database array given the ~cmp~
comparison criteria. It is assumed that the array is sorted. The function
returns true if the ~key~ is in the array and false otherwise. The position
is returned in the ~result~ argument. If ~key~ is not in the array, then 
~result~ contains the position where it should be.

*/

bool Find( const void *key,
           int (*cmp)( const void *a, const void *b),
           int& result ) const {

  DbArrayElement elem;

  if(content.size() == 0) {
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
  Get( content.size() - 1, elem );
  if( cmp( key, &elem ) > 0 ) {
     result = content.size();
     return false;
  }

   int first = 0, last = content.size() - 1, mid;

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
virtual bool Restrict( const std::vector< std::pair<int, int> >& intervals,
                       MMDbArray<DbArrayElement>& result ) const{
   result.content.clear();
   for(size_t i=0;i<intervals.size();i++){
      int s = intervals[i].first;
      int e = intervals[i].second;
      for(int j=s;j<=e;j++){
        result.content.push_back(content[j]);
      }
   }
   return true;
}


void destroy(){
  content.clear();
}

/*
~GetFlobSize~

Returns the capacity of the underlying Flob.

*/
size_t GetFlobSize() const {
   return content.capacity()*sizeof(DbArrayElement);
}

/*
~GetUsedSize~

Returns the total size occupied by the managed elements.

*/
size_t GetUsedSize() const {
   return content.size() * sizeof(DbArrayElement);
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
return content.capacity();
}


inline static size_t headerSize() {
  assert(false); // there is no persitent mechanism
  return 0;
}
 

/*
~SaveHeader~

Saves header information (this) to a record.

*/
virtual size_t serializeHeader( char* buffer, SmiSize& offset) const {
 assert(false);
 return 0;
}

virtual void restoreHeader(char* buffer,
                  SmiSize& offset)
{
  assert(false);
}


 bool copyFrom(const MMDbArray<DbArrayElement>& src){
    content = src.content;
    return true;
 }

 bool copyTo(DbArray<DbArrayElement>& dest){
    dest.content = this->content;
    return true;
 }

 bool copyFrom(const Flob& src){
   assert(false);
   return false;
 }

 bool copyTo(Flob& dest) const{
   assert(false);
   return false;
 }


class const_iterator{
 friend class MMDbArray;
 public:

     const_iterator(const const_iterator& other):
       mmdbarray(other.mmdbarray), 
       position(other.position), 
       elem(other.elem) {}

     const_iterator& operator=(const const_iterator& other){
        mmdbarray = other.mmdbarray;
        position = other.position;
        elem = other.elem;
        return *this;
     }

     const_iterator& operator++(){
        if(mmdbarray){
          if(position<mmdbarray->Size()){
            position++;
            if(position<mmdbarray->Size()){
                mmdbarray->Get(position,elem);
            }
          } else { // end() reached
             position = mmdbarray->Size();
          }
        } else { // single element iterator
           if(position<=0){ // 0 or -1 allowed
              position++; 
           } 
        } 
        return *this;
     }
     
     const_iterator& operator++(int dummy){
        if(mmdbarray){
          if(position<mmdbarray->Size()){
            position++;
            if(position<mmdbarray->Size()){
                mmdbarray->Get(position,elem);
            }
          } else { // end() reached
             position = mmdbarray->Size();
          }
        } else { // single element iterator
           if(position<=0){ // 0 or -1 allowed
              position++; 
           } 
        } 
        return *this;
     }

     const_iterator& operator--(){
        if(mmdbarray){
          if(position>=0){
            position--;
            if(position>=0 && position<mmdbarray->Size()){
               mmdbarray->Get(position, elem);
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
        if(mmdbarray){
          if(position>=0){
            position--;
            if(position>=0 && position<mmdbarray->Size()){
               mmdbarray->Get(position, elem);
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
         if(mmdbarray){
           assert(position>=0);
           assert(position < mmdbarray->Size());
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
       const MMDbArray<DbArrayElement>* mmdbarray;
       int position;
       DbArrayElement elem;

       const_iterator(const MMDbArray<DbArrayElement>* mmdbarray_, 
                      const int position_ = 0 ):
         mmdbarray(mmdbarray_), position(position_)  { 
         if(position<0){
            position=-1;
         } else if(position < mmdbarray->Size()){
            mmdbarray->Get(position,elem);
         } else {
           position = mmdbarray->Size();
         }
       }

       const_iterator(const DbArrayElement& elem_):
         mmdbarray(0),position(0), elem(elem_) {} 



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

    std::vector<DbArrayElement> content;


};

template< class DbArrayElement>
void convertDbArrays(const DbArray<DbArrayElement>& src, 
                     MMDbArray<DbArrayElement>& result) {
   result.resize(src.Size());
   DbArrayElement elem;
   for(int i=0;i<src.Size();i++){
      src.Get(i,elem);
      result.Put(i,elem);
   }
}

template< class DbArrayElement>
void convertDbArrays(const DbArray<DbArrayElement>& src, 
                     DbArray<DbArrayElement>& result) {
   result.resize(src.Size());
   DbArrayElement elem;
   for(int i=0;i<src.Size();i++){
      src.Get(i,elem);
      result.Put(i,elem);
   }
}


template< class DbArrayElement>
void convertDbArrays(const MMDbArray<DbArrayElement>& src, 
                     DbArray<DbArrayElement>& result) {
   result.resize(src.Size());
   DbArrayElement elem;
   for(int i=0;i<src.Size();i++){
      src.Get(i,elem);
      result.Put(i,elem);
   }
}

template< class DbArrayElement>
void convertDbArrays(const MMDbArray<DbArrayElement>& src, 
                     MMDbArray<DbArrayElement>& result) {
   result.resize(src.Size());
   DbArrayElement elem;
   for(int i=0;i<src.Size();i++){
      src.Get(i,elem);
      result.Put(i,elem);
   }
}

#endif //MMDBARRAY_H

