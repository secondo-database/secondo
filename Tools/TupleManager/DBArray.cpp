/*
//paragraph [10]  title:    [{\Large \bf ] [}]
//paragraph [11]  title:    [{\large \bf ] [}]
//paragraph [12]  title:    [{\normalsize \bf ] [}]
//paragraph [21]  table1column: [\begin{quote}\begin{tabular}{l}] [\end{tabular}\end{quote}]
//paragraph [22]  table2columns:  [\begin{quote}\begin{tabular}{ll}]  [\end{tabular}\end{quote}]
//paragraph [23]  table3columns:  [\begin{quote}\begin{tabular}{lll}] [\end{tabular}\end{quote}]
//paragraph [24]  table4columns:  [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]  [\hline]
//characters  [1] verbatim: [$] [$]
//characters  [2] formula:  [$] [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [$\leq$]
//[#]  [\neq]
//[tilde] [\verb|~|]

2 Implementation of DBArray

Version: 0.7

August 2002 RHG

April 2003 Victor Almeida chaged the implementation to not use templates.

2.1 Overview

This module offers a generic persistent array implemented on top of the
FLOB interface.

*/
#include "DBArray.h"

DBArray::DBArray( const int elemSize ) :
writeable( true ),
noComponents( 0 ),
canDelete( false ),
elemSize( elemSize ),
array( new FLOB( sizeof(int), true, true ) )
{
  array->Write( 0, sizeof(int), (char *)(&noComponents) );
}

DBArray::DBArray( const int elemSize, SmiRecordFile *file, const SmiRecordId& id, const bool update ) :
writeable( update ),
canDelete( false ),
elemSize( elemSize ),
array( new FLOB( file, id, update ) )
{
  array->Get( 0, sizeof( int ), (char *)(&noComponents) );
}

DBArray::DBArray( const int elemSize, const int n, const bool alloc, const bool update ) :
writeable( update ),
noComponents( n ),
canDelete( false ),
elemSize( elemSize ),
array( new FLOB( sizeof(int) + n * elemSize, alloc, update ) )
{
  if( alloc )
    array->Write( 0, sizeof( int ), (char *)(&noComponents) );
}

DBArray::~DBArray()
{
  if ( canDelete )
  {
    array->Destroy();
  }
  delete array;
}

void DBArray::Put(const int index, const DBArrayElement& elem)
{
  assert ( writeable );
  if( noComponents <= index )
  {
    noComponents = index + 1;
    array->Resize( sizeof(int) + noComponents * elemSize );
  }
  array->Write(sizeof(int) + index * elemSize, elemSize, elem.ToString() );
}

void DBArray::Get(int const index, DBArrayElement& elem)
{
  assert ( 0 <= index && index < noComponents );

  char *buf = (char *)malloc( elemSize );
  array->Get(sizeof(int) + index * elemSize, elemSize, buf);
  elem.FromString( buf );
  free( buf );
}

void DBArray::MarkDelete()
{
  assert( writeable );
  canDelete = true;
}


const int DBArray::GetNoComponents() const
{
  return noComponents;
}


const bool DBArray::Save( SmiRecordFile *file )
{
  array->Write( 0, sizeof( int ), (char *)(&noComponents) );
  return array->SaveToLob( file );
}

const SmiRecordId DBArray::GetRecordId() const
{
  return array->GetLobId();
}

FLOB *DBArray::GetArray() const
{
  return array;
}

