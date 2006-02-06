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

#include <vector>
#include <algorithm>
#include "FLOB.h"

using namespace std;

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
      FLOB::Clear();
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
    inline void Get( int index, DBArrayElement const*& elem ) const
    {
      const char *buf;
      FLOB::Get( index * sizeof( DBArrayElement ),
                 &buf );
      elem = (new ((void*)buf) DBArrayElement);
    }

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
      qsort( fd.inMemory.buffer, nElements, sizeof( DBArrayElement ), cmp );
    }
/*
Sorts the database array given the ~cmp~ comparison criteria. The
sort is done in memory using an STL vector.

*/
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

