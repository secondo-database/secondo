/*
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

    DBArray() {}
/*
This constructor should not be used.

*/
    DBArray( int n ):
      FLOB( n * sizeof( DBArrayElement ) ),
      nElements( 0 ),
      maxElements( n )
      {}
/*
The constructor of the array passing the number of elements to initialize it.

*/

    ~DBArray()
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

      assert( FLOB::Size() % sizeof( DBArrayElement ) == 0 );
      assert( maxElements == FLOB::Size() / (int)sizeof( DBArrayElement ) );
    }

    void Clear()
    {
      nElements = 0;
      maxElements = 0;
      FLOB::Clear();
    }

    void Destroy()
    {
      nElements = 0;
      maxElements = 0;
      FLOB::Destroy();
    }

    void Append( const DBArrayElement& elem )
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
          maxElements = nElements;
          FLOB::Resize( maxElements * sizeof( DBArrayElement ) );
        }

        FLOB::Put( index * sizeof( DBArrayElement ),
                   sizeof( DBArrayElement ),
                   &elem );

        assert( FLOB::Size() % sizeof( DBArrayElement ) == 0 );
        assert( maxElements == FLOB::Size() / (int)sizeof( DBArrayElement ) );
      }
    }

/*
Copies element ~elem~ into the persistent array at index ~index~.

*Precondition:* 0 [<=] ~index~.

*/

    void Get( int index, DBArrayElement& elem )
    {
      assert( index >= 0 && index < nElements );
      char *buf = (char*)malloc( sizeof( DBArrayElement ) );
      FLOB::Get( index * sizeof( DBArrayElement ),
                 sizeof( DBArrayElement ),
                 buf );
      DBArrayElement *auxElem = (new ((void*)buf) DBArrayElement);
      memcpy( &elem, auxElem, sizeof(DBArrayElement) );
      free( buf );
    }

/*
Returns the element ~index~ of the array.

*Precondition:* 0 [<=] ~index~ [<=] ~noComponents~ - 1.

*/

    int Size() const
    {
      return nElements;
    }

/*
Returns the number of components of this array.

*/

    void Sort( bool (*cmp)(const DBArrayElement&, const DBArrayElement&) )
    {
      if( nElements <= 1 )
        return;

      vector<DBArrayElement> elements;

      for( int i = 0; i < nElements; i++ )
      {
        DBArrayElement elem;
        Get( i, elem );
        elements.push_back( elem );
      }

      sort( elements.begin(), elements.end(), cmp );

      for( size_t i = 0; i < elements.size(); i++ )
        Put( i, elements[i] );
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

