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

April 2003 Victor Almeida chaged the implementation to not use templates.

1.1 Overview

This module offers a generic persistent array implemented on top of the
FLOB interface.

1.2 Interface methods

This module offers the following methods:

[23]	Creation/Removal 	& Access   	& Inquiries	\\
	[--------]
	DBArray        		& Get 		& NoComponents	\\
	[tilde]DBArray		& Put		  & Id		        \\
	MarkDelete		    &		      & 		          \\

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
#include "SecondoSystem.h"
#include "FLOB.h"

using namespace std;

#define _PAGED_DBARRAY_

template<class DBArrayElement>
class DBArray : public FLOB
{
  public:

    inline DBArray() {}
/*
This constructor should not be used.

*/
    inline DBArray( int n ):
      FLOB( n * sizeof( DBArrayElement ) ),
      nElements( 0 ),
      maxElements( n )
      {}
/*
The constructor of the array passing the number of elements to initialize it.

*/

    inline ~DBArray()
      {}

/*
The destructor.

*/
    void Resize( const int newSize )
    {
      assert( type == InMemory );
      assert( nElements <= maxElements );
      assert( newSize > 0 );

      if( newSize < nElements )
        nElements = newSize;

      maxElements = newSize;
      FLOB::Resize( newSize * sizeof( DBArrayElement ) );
    }

    inline void Clear()
    {
      nElements = 0;
      maxElements = 0;
      FLOB::Clean();
    }

    inline void Destroy()
    {
      nElements = 0;
      maxElements = 0;
      FLOB::Destroy();
    }

    inline void Append( const DBArrayElement& elem )
    {
      Put( nElements, elem );
    }

    void Put( int index, const DBArrayElement& elem )
    {
      assert( type == InMemory );
      assert( index >= 0 );
      assert( nElements <= maxElements );

      if( index < nElements )
      {
        FLOB::Put( index * sizeof( DBArrayElement ),
                   sizeof( DBArrayElement ),
                   &elem );
      }
      else
      {
        nElements = index + 1;
        if( nElements > maxElements )
        {
          maxElements = nElements + 16;
          FLOB::Resize( maxElements * sizeof( DBArrayElement ) );
        }

        FLOB::Put( index * sizeof( DBArrayElement ),
                   sizeof( DBArrayElement ),
                   &elem );
      }
    }

/*
Copies element ~elem~ into the persistent array at index ~index~.

*Precondition:* 0 [<=] ~index~.

*/
#ifdef _PAGED_DBARRAY_
    inline void Get( int index, DBArrayElement const*& elem ) const
    {
      assert( type != InMemoryCached );

      const char *buf;
      if( type == InMemoryPagedCached ||
          type == InDiskLarge ) 
      {
        size_t pos = ((index % ElemsPerPage()) * sizeof( DBArrayElement )) +
                     ((index / ElemsPerPage()) * FLOB::PAGE_SIZE);
        FLOB::Get( pos, &buf, true );
      }
      else
      {
        FLOB::Get( index * sizeof( DBArrayElement ), &buf, true );
      }
      elem = (new ((void*)buf) DBArrayElement);
    }
#else
    inline void Get( int index, DBArrayElement const*& elem ) const
    {
      const char *buf;
      FLOB::Get( index * sizeof( DBArrayElement ), &buf );
      elem = (new ((void*)buf) DBArrayElement);
    }
#endif

/*
Returns the element ~index~ of the array.

*Precondition:* 0 [<=] ~index~ [<=] ~noComponents~ - 1.

*/

    inline int Size() const
    {
      return nElements;
    }

/*
Returns the number of components of this array.

*/

    void Sort( int (*cmp)( const void *a, const void *b) )
    {
      assert( type == InMemory );
      if( nElements <= 1 )
        return;
      qsort( FLOB::fd.inMemory.buffer, nElements, 
             sizeof( DBArrayElement ), cmp );
    }
/*
Sorts the database array given the ~cmp~ comparison criteria. The
sort is done in memory using an STL vector.

*/

    bool Find( const void *key,
               int (*cmp)( const void *a, const void *b),
               int& result ) const
    {
      const DBArrayElement *elem;

      Get( 0, elem );
      if( nElements == 0 ||
          cmp( key, elem ) < 0 )
      {
        result = 0;
        return false;
      }

      Get( nElements - 1, elem );
      if( cmp( key, elem ) > 0 )
      {
        result = nElements;
        return false;
      }

      int first = 0, last = nElements - 1, mid;

      while (first <= last)
      {
        mid = ( first + last ) / 2;
        Get( mid, elem );
        if( cmp( key, elem ) > 0 )
          first = mid + 1;
        else if( cmp( key, elem ) < 0 )
          last = mid - 1;
        else
        {
          result = mid;
          return true;
        }
      }
      result = first < last ? first + 1 : last + 1;
      return false;
    }
/*
Searches (binary search) for a given key in the database array given the ~cmp~
comparison criteria. It is assumed that the array is sorted. The function returns
true if the ~key~ is in the array and false otherwise. The position is returned in
the ~result~ argument. If ~key~ is not in the array, then ~result~ contains the
position where it should be.

*/

    inline size_t NoPages() const
    {
      assert( type == InMemoryPagedCached ||
              type == InMemory );
      if( nElements % ElemsPerPage() == 0 )
        return nElements / ElemsPerPage();
      return nElements / ElemsPerPage() + 1;
    }
/*
Returns how many pages are necessary to store the whole DBArray.

*/

    inline size_t ElemsPerPage() const
    {
      return FLOB::PAGE_SIZE / sizeof( DBArrayElement );
    }

#ifdef _PAGED_DBARRAY_
    void SaveToLob( SmiRecordId& lobFileId, SmiRecordId lobId = 0 ) const
    {
      if( type == InDiskLarge )
      {
        SmiFileId auxLobFileId = FLOB::fd.inDiskLarge.lobFileId;
        SmiRecordId auxLobId = FLOB::fd.inDiskLarge.lobId;
        type = InMemoryPagedCached;
        FLOB::fd.inMemoryPagedCached.lobFileId = auxLobFileId;
        FLOB::fd.inMemoryPagedCached.lobId = auxLobId;
        FLOB::fd.inMemoryPagedCached.buffer = 0;
        FLOB::fd.inMemoryPagedCached.pageno = -1;

        SaveToLob( lobFileId, lobId );
      }
      else 
      {
        SmiRecordId auxLobId = lobId;
        if( type == InMemoryPagedCached )
        {
          // write all pages
          for( size_t pageno = 0; pageno < NoPages(); pageno++ )
          {
            GetPage( pageno );
            SecondoSystem::GetFLOBCache()->
              PutFLOB( lobFileId, auxLobId,
                       pageno, FLOB::PAGE_SIZE, false,
                       FLOB::fd.inMemoryPagedCached.buffer );
          }

          // clear the buffer
          assert( FLOB::fd.inMemoryPagedCached.buffer != 0 );
          if( FLOB::fd.inMemoryPagedCached.cached )
            SecondoSystem::GetFLOBCache()->
              Release( FLOB::fd.inMemoryPagedCached.lobFileId,
                       FLOB::fd.inMemoryPagedCached.lobId,
                       FLOB::fd.inMemoryPagedCached.pageno );
          else
            free( FLOB::fd.inMemoryPagedCached.buffer );
        }
        else if( type == InMemory )
        {
          // write all pages except the last one
          char *pageBuf = (char*)malloc( FLOB::PAGE_SIZE );

          size_t pageno;
          for( pageno = 0; pageno < NoPages() - 1; pageno++ )
          {
            assert( pageno * ElemsPerPage() < size );
            assert( pageno * ElemsPerPage() + 
                    ElemsPerPage() * sizeof( DBArrayElement ) <= size );
            size_t pos = pageno * ElemsPerPage() * sizeof( DBArrayElement ),
                   count = ElemsPerPage() * sizeof( DBArrayElement );
            memset( pageBuf, 0, FLOB::PAGE_SIZE );
            memcpy( pageBuf, FLOB::fd.inMemory.buffer + pos, count );
            SecondoSystem::GetFLOBCache()->
              PutFLOB( lobFileId, auxLobId, pageno, 
                       FLOB::PAGE_SIZE, false, pageBuf );
          } 
          // write the last page
          assert( nElements - pageno * ElemsPerPage() != 0 ); 
          assert( pageno * ElemsPerPage() < size );
          assert( pageno * ElemsPerPage() + 
                  (nElements - pageno * ElemsPerPage()) * 
                  sizeof( DBArrayElement ) <= size );
          size_t pos = pageno * ElemsPerPage() * sizeof( DBArrayElement ),
                 count = (nElements - pageno * ElemsPerPage()) * 
                         sizeof( DBArrayElement );
          memset( pageBuf, 0, FLOB::PAGE_SIZE );
          memcpy( pageBuf, FLOB::fd.inMemory.buffer + pos, count );
          SecondoSystem::GetFLOBCache()->
            PutFLOB( lobFileId, auxLobId, pageno, 
                     FLOB::PAGE_SIZE, false, pageBuf );

          // clear the buffer
          if( FLOB::fd.inMemory.canDelete )
            free( FLOB::fd.inMemory.buffer );
          free( pageBuf );
        }
        else 
          assert( false );

        // change the type to InDiskLarge
        type = InDiskLarge;
        FLOB::fd.inDiskLarge.lobFileId = lobFileId;
        FLOB::fd.inDiskLarge.lobId = auxLobId;
      }
    }
/*
Saves the DBArray to the LOB file in paged format. 

*/
#endif

#ifdef _PAGED_DBARRAY_
    const char *BringToMemory() const
    {
      assert( type != InDiskLarge && 
              type != InMemoryPagedCached );
      return FLOB::BringToMemory(); 
    }
/*
Brings a disk DBArray to memory. We do not allow this to be done because a DBArray
is divided into several pages with padding.

*/
    virtual void Restrict( const vector< pair<int, int> >& intervals )
    {
      size_t newSize = 0;
      for( vector< pair<int, int> >::const_iterator i = intervals.begin();
           i < intervals.end();
           i++ )
      {
        assert( i->first <= i->second );
        newSize += ( ( i->second - i->first ) + 1 ) * sizeof( DBArrayElement );
      }
      assert( newSize <= size );
      if( newSize < size );
      {
        if( newSize > 0 )
        {
          char *buffer = (char*)malloc( newSize );
          size_t offset = 0;
          const DBArrayElement *e;
    
          for( vector< pair<int, int> >::const_iterator i = intervals.begin();
               i < intervals.end();
               i++ )
          {    
            for( int j = i->first; j <= i->second; j++ )
            {
              Get( j, e );
              assert( offset + sizeof( DBArrayElement ) <= newSize );
              memcpy( buffer + offset, e, sizeof( DBArrayElement ) );
              offset += sizeof( DBArrayElement );
            }  
          }
          if( FLOB::type == InMemoryPagedCached ) 
          {
            assert( FLOB::fd.inMemoryPagedCached.buffer != 0 );
            if( FLOB::fd.inMemoryPagedCached.cached )
              SecondoSystem::GetFLOBCache()->
                Release( FLOB::fd.inMemoryPagedCached.lobFileId,
                         FLOB::fd.inMemoryPagedCached.lobId,
                         FLOB::fd.inMemoryPagedCached.pageno );
            else
              free( FLOB::fd.inMemoryPagedCached.buffer );
          }

          type = InMemory;
          FLOB::fd.inMemory.buffer = buffer;
          FLOB::fd.inMemory.canDelete = true;
        }
        else
        {
          type = InMemory;
          FLOB::fd.inMemory.buffer = 0;
          FLOB::fd.inMemory.canDelete = false;
        }
        assert( newSize % sizeof( DBArrayElement ) == 0 );
        nElements = newSize / sizeof( DBArrayElement );
        maxElements = nElements;
        size = newSize;
      }
    }
/*
Restricts the DBArray to the interval set of indices passed as argument.

*/
#endif
    
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

