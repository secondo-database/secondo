/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Header File of the Range Algebra

February, 2002 Victor Teixeira de Almeida

1 Overview

This implementation file essentially contains the implementation of the classes 
~Interval~ and ~Range~. The class ~Range~ corresponds to the memory representation of the type
constructor ~range($\alpha$)~. The class ~Interval~ is used by the class ~Range~.

For more detailed information see RangeAlgebra.h.

2 Defines and Includes

*/
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RangeAlgebra.h"

static NestedList* nl;
static QueryProcessor* qp;

/*
3 Type Constructor ~range~

This type constructor implements the carrier set for ~range($\alpha$)~.

3.1 Implementation of class ~Range~

*/
Range::Range( SmiRecordFile *recordFile, const int algebraId, const int typeId, const int size ):
algebraId( algebraId ),
typeId( typeId ),
size( size ),
writeable( true ),
parrays( recordFile ),
canDelete( false ),
intervalCount( 0 ),
ordered( true )
{
  parrays->AppendRecord( recid, record );
  record.Write( &algebraId, sizeof( int ), 0 );
  record.Write( &typeId, sizeof( int ), sizeof( int ) );
  record.Write( &size, sizeof( int ), 2 * sizeof( int ) );
  record.Write( &intervalCount, sizeof(int), 3 * sizeof( int ) );
}

Range::Range( SmiRecordFile *recordFile, const SmiRecordId id, const bool update ) :
writeable( update ),
parrays( recordFile ),
canDelete( false ),
ordered( true )
{
  SmiFile::AccessType at = update ? SmiFile::Update : SmiFile::ReadOnly;
  parrays->SelectRecord( id, record, at );
  recid = id;
  record.Read( &algebraId, sizeof( int ), 0 );
  record.Read( &typeId, sizeof( int ), sizeof( int ) );
  record.Read( &size, sizeof( int ), 2 * sizeof( int ) );
  record.Read( &intervalCount, sizeof(int), 3 * sizeof( int ) );
}

Range::~Range()
{
  if ( canDelete )
  {
    parrays->DeleteRecord( recid );
  }
  else if ( writeable )
  {
    record.Write( &intervalCount, sizeof(int), 3 * sizeof( int ) );
  }
}

void Range::Add( const Interval& interval )
{
  assert ( writeable );
  assert ( interval.GetBegin() != NULL && interval.GetEnd() != NULL );
  assert ( interval.GetBegin()->IsDefined() && interval.GetEnd()->IsDefined() );

  record.Write(interval.GetBegin(), size, 4 * sizeof(int) + size *  2 * intervalCount );
  record.Write(interval.GetEnd(),   size, 4 * sizeof(int) + size * (2 * intervalCount + 1) );

  intervalCount++;
}

void Range::Put( const int index, const Interval& interval )
{
  assert ( writeable );
  assert ( interval.GetBegin() != NULL && interval.GetEnd() != NULL );
  assert ( interval.GetBegin()->IsDefined() && interval.GetEnd()->IsDefined() );
  assert ( 0 <= index && index < intervalCount ); 

  record.Write(interval.GetBegin(), size, 4 * sizeof(int) + size *  2 * index );
  record.Write(interval.GetEnd(),   size, 4 * sizeof(int) + size * (2 * index + 1) );
}

void Range::Get(const int index, Interval& interval)
{
  assert ( 0 <= index && index < intervalCount );
  assert ( interval.GetBegin() != NULL && interval.GetEnd() != NULL );
  assert ( interval.GetBegin()->Sizeof() == size && interval.GetEnd()->Sizeof() == size );

  record.Read(interval.GetBegin(), size, 4 * sizeof(int) + size *  2 * index );
  record.Read(interval.GetEnd(),   size, 4 * sizeof(int) + size * (2 * index + 1) );

  assert( interval.GetBegin()->IsDefined() && interval.GetEnd()->IsDefined() );
}

void Range::Get(const int index, const IntervalPosition pos, StandardAttribute *a)
{
  assert ( 0 <= index && index < intervalCount );
  assert ( a != NULL );
  assert ( a->Sizeof() == size );

  if( pos == Begin )
    record.Read(a, size, 4 * sizeof(int) + size *  2 * index );
  else
    record.Read(a, size, 4 * sizeof(int) + size * (2 * index + 1) );

  assert( a->IsDefined() );
}

void Range::Destroy()
{
  assert( writeable );
  canDelete = true;
}

void Range::Sort()
{
  if( intervalCount > 1 )
  {
    int low = 0, high = intervalCount - 1;
    QuickSortRecursive( low, high );
  }
}

void Range::QuickSortRecursive( const int low, const int high )
{
  int i = high, j = low;

  // Creating the memory representation of the objects to be used in the sort
  // algorithm.
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval p( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
              (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval pi( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
               (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval pj( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
               (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Get( (int)( (low + high) / 2 ), p );

  do
  {
    Get( j, pj );
    while( pj < p )
      Get( ++j, pj );

    Get( i, pi );
    while( pi > p )
      Get( --i, pi );

    if( i >= j )
    {
      if ( i != j )
      {
        Put( i, pj );
        Put( j, pi );
      }

      i--;
      j++;
    }
  } while( j <= i );

  if( low < i )
    QuickSortRecursive( low, i );
  if( j < high )
    QuickSortRecursive( j, high );

  
  // Deleting the memory representation of the objects used in the sort algorithm.
  Word begin = SetWord(p.GetBegin()),
       end   = SetWord(p.GetEnd());
  (algM->CloseObj(algebraId, typeId))( begin );
  (algM->CloseObj(algebraId, typeId))( end );
  begin = SetWord(pi.GetBegin());
  end   = SetWord(pi.GetEnd());
  (algM->CloseObj(algebraId, typeId))( begin );
  (algM->CloseObj(algebraId, typeId))( end );
  begin = SetWord(pj.GetBegin());
  end   = SetWord(pj.GetEnd());
  (algM->CloseObj(algebraId, typeId))( begin );
  (algM->CloseObj(algebraId, typeId))( end );
}

void Range::StartBulkLoad()
{
  assert( IsOrdered() );
  ordered = false;
}

void Range::EndBulkLoad( const bool sort )
{
  assert( !IsOrdered() );
  if( sort )
    Sort();
  ordered = true;
}

const bool Range::IsOrdered() const
{
  return ordered;
}

int Range::operator==(Range& r)
{
  if( GetIntervalCount() != r.GetIntervalCount() )
    return false;

  bool result = true;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                     (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  for( int i = 0; i < GetIntervalCount(); i++ )
  {
    Get( i, thisInterval );
    r.Get( i, interval );
   
    if( thisInterval != interval )
    {
      result = false;
      break;
    }
  }

  Word begin = SetWord(thisInterval.GetBegin()),
       end   = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );
  begin = SetWord(interval.GetBegin());
  end   = SetWord(interval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );

  return result;
}

int Range::operator!=(Range& r)
{
 return !( *this == r );
}

const bool Range::Intersects(Range& r)
{
  if( IsEmpty() || r.IsEmpty() )
    return false;

  bool result = false;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                     (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  while( 1 )
  {
    if( thisInterval.Intersects( interval ) )
    {
      result = true;
      break;
    }

    if( thisInterval.Before( interval ) )
    {
      if( ++i == GetIntervalCount() )
      {
        result = false;
        break;
      }
      Get( i, thisInterval );
    }
    
    if( interval.Before( thisInterval ) )
    {
      if( ++j == r.GetIntervalCount() )
      {
        result = false;
        break;
      }
      r.Get( j, interval );
    }
  }

  Word begin = SetWord(thisInterval.GetBegin()),
       end   = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );
  begin = SetWord(interval.GetBegin());
  end   = SetWord(interval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );

  return result;
}

const bool Range::Inside(Range& r)
{
  if( IsEmpty() ) return true;
  if( r.IsEmpty() ) return false;

  bool result = true;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                     (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  while( 1 )
  {
    if( interval.Before( thisInterval ) )
    {
      if( ++j == r.GetIntervalCount() )
      {
        result = false;
        break;
      }
      r.Get( j, interval );
    }
    else if( thisInterval.Inside( interval ) )
    {
      if( ++i == GetIntervalCount() )
      {
        break;
      }
      Get( i, thisInterval );
    }
    else if( thisInterval.Before( interval ) )
    {
      result = false;
      break;
    }
    else 
    {
      // Intersection but no inside.
      result = false;
      break;
    }
  }

  Word begin = SetWord(thisInterval.GetBegin()),
       end   = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );
  begin = SetWord(interval.GetBegin());
  end   = SetWord(interval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );

  return result;
}

const bool Range::Contains(StandardAttribute *a)
{
  assert( IsOrdered() && a->IsDefined() );

  if( IsEmpty() )
    return false;

  bool result = false;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval midInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                        (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  int first = 0, last = GetIntervalCount() - 1;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;
    Get( mid, midInterval );
    if( midInterval.Contains( a ) )
    {
      result = true;
      break;
    }
    else if( midInterval.Before( a ) )
      first = mid + 1;
    else if( midInterval.After( a ) )
      last = mid - 1;
    else
    {
      result = true;
      break;
    }
  }

  Word begin = SetWord(midInterval.GetBegin()),
       end   = SetWord(midInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );

  return result;
}

const bool Range::Before(Range& r)
{
  assert( !IsEmpty() && !r.IsEmpty() );

  bool result = true;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                     (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  Get( GetIntervalCount() - 1, thisInterval );
  r.Get( 0, interval );
  result = thisInterval.Before( interval );

  Word begin = SetWord(thisInterval.GetBegin()),
       end   = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );
  begin = SetWord(interval.GetBegin());
  end   = SetWord(interval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );

  return result;
}

const bool Range::Before(StandardAttribute *a)
{
  assert( !IsEmpty() && a->IsDefined() );

  bool result = true;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  Get( GetIntervalCount() - 1, thisInterval );
  result = thisInterval.Before( a );

  Word begin = SetWord(thisInterval.GetBegin()),
       end   = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );

  return result;
}

const bool Range::After(StandardAttribute *a)
{
  assert( !IsEmpty() && a->IsDefined() );

  bool result = true;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  Get( 0, thisInterval );
  result = thisInterval.After( a );

  Word begin = SetWord(thisInterval.GetBegin()),
       end   = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );

  return result;
}

void Range::Intersection(Range& r, Range& result)
{
  assert( IsOrdered() && r.IsOrdered() && result.IsEmpty() );

  if( !Intersects( r ) )
    return;

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                     (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  result.StartBulkLoad();
  while( 1 )
  {
    if( thisInterval == interval )
    {
      Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                            (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
        result.Add( newInterval );
        Word begin = SetWord(newInterval.GetBegin()),
             end   = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( begin );
        (algM->DeleteObj(algebraId, typeId))( end );
        if( ++i < GetIntervalCount() )
          Get( i, thisInterval );
        else
          break;
        if( ++j < r.GetIntervalCount() )
          r.Get( j, interval );
        else
          break;
    }
    else if( thisInterval.Inside( interval ) )
    {
      Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                            (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
      result.Add( newInterval );
      Word begin = SetWord(newInterval.GetBegin()),
           end   = SetWord(newInterval.GetEnd());
      (algM->DeleteObj(algebraId, typeId))( begin );
      (algM->DeleteObj(algebraId, typeId))( end );
      if( ++i < GetIntervalCount() )
        Get( i, thisInterval );
      else
        break;
    }
    else if( interval.Inside( thisInterval ) )
    {
      Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr,
                            (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr );
      result.Add( newInterval );
      Word begin = SetWord(newInterval.GetBegin()),
           end   = SetWord(newInterval.GetEnd());
      (algM->DeleteObj(algebraId, typeId))( begin );
      (algM->DeleteObj(algebraId, typeId))( end );
      if( ++j < r.GetIntervalCount() )
        r.Get( j, interval );
      else
        break;
    }
    else if( thisInterval.Intersects( interval ) )
    {
      if( thisInterval <= interval )
      {
        Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr,
                              (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
        result.Add( newInterval );
        Word begin = SetWord(newInterval.GetBegin()),
             end   = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( begin );
        (algM->DeleteObj(algebraId, typeId))( end );
        if( ++i < GetIntervalCount() )
          Get( i, thisInterval );
        else
          break;
      }
      else
      {
        Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                              (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr );
        result.Add( newInterval );
        Word begin = SetWord(newInterval.GetBegin()),
             end   = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( begin );
        (algM->DeleteObj(algebraId, typeId))( end );
        if( ++j < r.GetIntervalCount() )
          r.Get( j, interval );
        else
          break;
      }
    }
    else if( thisInterval <= interval )
    {
      if( ++i < GetIntervalCount() )
        Get( i, thisInterval );
      else
        break;
    }
    else
    {
      if( ++j < r.GetIntervalCount() )
        r.Get( j, interval );
      else
        break;
    }
  }
  result.EndBulkLoad( false );

  Word begin = SetWord(thisInterval.GetBegin()),
       end   = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );
  begin = SetWord(interval.GetBegin());
  end   = SetWord(interval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( begin );
  (algM->DeleteObj(algebraId, typeId))( end );
}

void Range::Union(Range& r, Range& result)
{
  assert( IsOrdered() && r.IsOrdered() && result.IsEmpty() );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                     (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  result.StartBulkLoad();
  StandardAttribute *begin = NULL, *end = NULL;

  while( 1 )
  {
    if( thisInterval == interval )
    {
      Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                            (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
      result.Add( newInterval );
      Word begin = SetWord(newInterval.GetBegin()),
           end   = SetWord(newInterval.GetEnd());
      (algM->DeleteObj(algebraId, typeId))( begin );
      (algM->DeleteObj(algebraId, typeId))( end );

      if( ++i < GetIntervalCount() )
        Get( i, thisInterval );
      
      if( ++j < r.GetIntervalCount() )
        r.Get( j, interval );

      if( i >= GetIntervalCount() || j >= r.GetIntervalCount() )
        break;
    }
    else if( !thisInterval.Intersects( interval ) )
    {
      if( begin != NULL && end != NULL )
      {
        Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( begin ) ).addr,
                              (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( end ) ).addr );
        result.Add( newInterval );
        Word b = SetWord(newInterval.GetBegin()),
             e = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( b );
        (algM->DeleteObj(algebraId, typeId))( e );
        begin = NULL; 
        end   = NULL;

        if( thisInterval < interval )
        {
          if( ++i < GetIntervalCount() )
            Get( i, thisInterval );
          else
            break;
        }
        else
        {
          if( ++j < r.GetIntervalCount() )
            r.Get( j, interval );
          else
            break;
        }
      }
      else
      {
        if( thisInterval < interval )
        {
          Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                                (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
          result.Add( newInterval );
          Word begin = SetWord(newInterval.GetBegin()),
               end   = SetWord(newInterval.GetEnd());
          (algM->DeleteObj(algebraId, typeId))( begin );
          (algM->DeleteObj(algebraId, typeId))( end );

          if( ++i < GetIntervalCount() )
            Get( i, thisInterval );
          else
            break;
        }
        else
        {
          Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr,
                                (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr );
          result.Add( newInterval );
          Word begin = SetWord(newInterval.GetBegin()),
               end   = SetWord(newInterval.GetEnd());
          (algM->DeleteObj(algebraId, typeId))( begin );
          (algM->DeleteObj(algebraId, typeId))( end );

          if( ++j < r.GetIntervalCount() )
            r.Get( j, interval );
          else
            break;
        }
      }
    }
    else
    {
      if( begin == NULL && end == NULL )
      {
        if( thisInterval.GetBegin()->Compare( interval.GetBegin() ) <= 0 )
          begin = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr;
        else
          begin = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr;
  
        if( thisInterval.GetEnd()->Compare( interval.GetEnd() ) >= 0 )
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr;
        else
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr;
      }
      else
      {
        if( thisInterval <= interval )
        {
          if( begin->Compare( thisInterval.GetBegin() ) > 0 )
          {
            Word b = SetWord( begin );
            (algM->DeleteObj(algebraId, typeId))( b );
            
            begin = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr;
          }
        }
        else
        {
          if( begin->Compare( interval.GetBegin() ) > 0 )
          {
            Word b = SetWord( begin );
            (algM->DeleteObj(algebraId, typeId))( b );

            begin = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr;
          }
        }
  
        if( thisInterval.GetEnd()->Compare( interval.GetEnd() ) >= 0 )
        {
          if( end->Compare( thisInterval.GetEnd() ) < 0 )
          {
            Word e = SetWord( end );
            (algM->DeleteObj(algebraId, typeId))( e );
            
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr;
          }
        }
        else
        {
          if( end->Compare( interval.GetEnd() ) < 0 )
          {
            Word e = SetWord( end );
            (algM->DeleteObj(algebraId, typeId))( e );
            
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr;
          }
        }
      }

      if( thisInterval.GetEnd()->Compare( interval.GetEnd() ) < 0 )
      {
        if( ++i < GetIntervalCount() )
          Get( i, thisInterval );
        else
          break;
      }
      else if( thisInterval.GetEnd()->Compare( interval.GetEnd() ) > 0 )
      {
        if( ++j < r.GetIntervalCount() )
          r.Get( j, interval );
        else
          break;
      }
      else
      {
        if( ++i < GetIntervalCount() )
          Get( i, thisInterval );

        if( ++j < r.GetIntervalCount() )
          r.Get( j, interval );

        if( i >= GetIntervalCount() || j >= r.GetIntervalCount() )
          break;

        Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( begin ) ).addr,
                              (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( end ) ).addr );
        result.Add( newInterval );
        Word b = SetWord(newInterval.GetBegin()),
             e = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( b );
        (algM->DeleteObj(algebraId, typeId))( e );
        begin = NULL;
        end   = NULL;
      }
    }
  }
  if( begin != NULL && end != NULL )
  {
    Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( begin ) ).addr,
                          (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( end ) ).addr );
    result.Add( newInterval );
    Word b = SetWord(newInterval.GetBegin()),
         e = SetWord(newInterval.GetEnd());
    (algM->DeleteObj(algebraId, typeId))( b );
    (algM->DeleteObj(algebraId, typeId))( e );

    if( j >= r.GetIntervalCount() )
    {
      if( ++i < GetIntervalCount() )
        Get( i, thisInterval );
    }
    else if( i >= GetIntervalCount() )
    {
      if( ++j < r.GetIntervalCount() )
        r.Get( j, interval );
    }
  }
  while( i < GetIntervalCount() )
  {
    Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                          (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
    result.Add( newInterval );
    Word b = SetWord(newInterval.GetBegin()),
         e = SetWord(newInterval.GetEnd());
    (algM->DeleteObj(algebraId, typeId))( b );
    (algM->DeleteObj(algebraId, typeId))( e );

    if( ++i < GetIntervalCount() )
      Get( i, thisInterval );
  }
  while( j < r.GetIntervalCount() )
  {
    Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr,
                          (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr );
    result.Add( newInterval );
    Word b = SetWord(newInterval.GetBegin()),
         e = SetWord(newInterval.GetEnd());
    (algM->DeleteObj(algebraId, typeId))( b );
    (algM->DeleteObj(algebraId, typeId))( e );

    if( ++j < r.GetIntervalCount() )
      r.Get( j, interval );
  }
  result.EndBulkLoad( false );

  Word b = SetWord(thisInterval.GetBegin()),
       e = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( b );
  (algM->DeleteObj(algebraId, typeId))( e );
  b = SetWord(interval.GetBegin());
  e = SetWord(interval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( b );
  (algM->DeleteObj(algebraId, typeId))( e );
}

void Range::Minus(Range& r, Range& result)
{
  assert( IsOrdered() && r.IsOrdered() && result.IsEmpty() );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );
  Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr,
                     (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr );

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  result.StartBulkLoad();
  StandardAttribute *begin = NULL, *end = NULL;

  while( 1 )
  {
    if( thisInterval == interval )
    {
      if( ++i < GetIntervalCount() )
        Get( i, thisInterval );
      
      if( ++j < r.GetIntervalCount() )
        r.Get( j, interval );

      if( i >= GetIntervalCount() || j >= r.GetIntervalCount() )
        break;
    }
    else if( !thisInterval.Intersects( interval ) )
    {
      if( begin != NULL && end != NULL )
      {
        assert( thisInterval < interval );

        Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( begin ) ).addr,
                              (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( end ) ).addr );
        result.Add( newInterval );
        Word b = SetWord(newInterval.GetBegin()),
             e = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( b );
        (algM->DeleteObj(algebraId, typeId))( e );

        begin = NULL;
        end   = NULL;

        if( ++i < GetIntervalCount() )
          Get( i, thisInterval );
        else
          break;
      }
      else
      {
        if( thisInterval < interval )
        {
          Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                                (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
          result.Add( newInterval );
          Word b = SetWord(newInterval.GetBegin()),
               e = SetWord(newInterval.GetEnd());
          (algM->DeleteObj(algebraId, typeId))( b );
          (algM->DeleteObj(algebraId, typeId))( e );
 
          if( ++i < GetIntervalCount() )
            Get( i, thisInterval );
          else
            break;
        }
        else
        {
          if( ++j < r.GetIntervalCount() )
            r.Get( j, interval );
          else
            break;
        }
      }
    }
    else if( thisInterval.Inside( interval ) )
    {
      if( ++i < GetIntervalCount() )
        Get( i, thisInterval );
      else
        break;
    }
    else if( interval.Inside( thisInterval ) )
    {
      if( begin == NULL && end == NULL )
      {
        if( thisInterval.GetBegin()->Compare( interval.GetBegin() ) < 0 )
        {
          Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                                (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr );
          result.Add( newInterval );
          Word b = SetWord(newInterval.GetBegin()),
               e = SetWord(newInterval.GetEnd());
          (algM->DeleteObj(algebraId, typeId))( b );
          (algM->DeleteObj(algebraId, typeId))( e );
        }
      }
      else
      {
        end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr;

        Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( begin ) ).addr,
                              (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( end ) ).addr );
        result.Add( newInterval );
        Word b = SetWord(newInterval.GetBegin()),
             e = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( b );
        (algM->DeleteObj(algebraId, typeId))( e );
      }

      if( interval.GetEnd()->Compare( thisInterval.GetEnd() ) < 0 )
      {
        begin = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr;
        end   = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr;
 
        if( ++j < r.GetIntervalCount() )
          r.Get( j, interval );
        else
          break;
      }
      else
      {
        if( ++i < GetIntervalCount() )
          Get( i, thisInterval );
 
        if( ++j < r.GetIntervalCount() )
          r.Get( j, interval );

        if( i >= GetIntervalCount() || j >= r.GetIntervalCount() )
          break;
      }
    }
    else
    {
      assert( thisInterval.Intersects( interval ) );

      if( begin == NULL && end == NULL )
      {
        if( thisInterval < interval )
        {
          Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                                (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr );
          result.Add( newInterval );
          Word b = SetWord(newInterval.GetBegin()),
               e = SetWord(newInterval.GetEnd());
          (algM->DeleteObj(algebraId, typeId))( b );
          (algM->DeleteObj(algebraId, typeId))( e );
    
          if( ++i < GetIntervalCount() )
            Get( i, thisInterval );
          else
            break;
        }
        else
        {
          begin = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr;
          end   = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr;

          if( ++j < r.GetIntervalCount() )
            r.Get( j, interval );
          else
            break;

        }
      }
      else
      {
        assert( thisInterval < interval );
        assert( interval.GetBegin()->Compare( begin ) > 0 && interval.GetBegin()->Compare( end ) <= 0 );

        end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetBegin() ) ).addr;

        Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( begin ) ).addr,
                              (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( end ) ).addr );
        result.Add( newInterval );
        Word b = SetWord(newInterval.GetBegin()),
             e = SetWord(newInterval.GetEnd());
        (algM->DeleteObj(algebraId, typeId))( b );
        (algM->DeleteObj(algebraId, typeId))( e );

        if( interval.GetEnd()->Compare( thisInterval.GetEnd() ) < 0 )
        {
          begin = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.GetEnd() ) ).addr;
          end   = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr;

          if( ++j < r.GetIntervalCount() )
            r.Get( j, interval );
          else
            break;
        }
        else if( interval.GetEnd()->Compare( thisInterval.GetEnd() ) > 0 )
        {
          if( ++i < GetIntervalCount() )
            Get( i, thisInterval );
          else
            break;
        }
        else
        {
          if( ++i < GetIntervalCount() )
            Get( i, thisInterval );

          if( ++j < r.GetIntervalCount() )
            r.Get( j, interval );
         
          if( i >= GetIntervalCount() || j >= r.GetIntervalCount() )
            break;
        } 
      }
    }
  }
  if( begin != NULL && end != NULL )
  {
    Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( begin ) ).addr,
                          (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( end ) ).addr );
    result.Add( newInterval );
    Word b = SetWord(newInterval.GetBegin()),
         e = SetWord(newInterval.GetEnd());
    (algM->DeleteObj(algebraId, typeId))( b );
    (algM->DeleteObj(algebraId, typeId))( e );

    if( ++i < GetIntervalCount() )
      Get( i, thisInterval );
  }
  while( i < GetIntervalCount() )
  {
    Interval newInterval( (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetBegin() ) ).addr,
                          (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.GetEnd() ) ).addr );
    result.Add( newInterval );
    Word b = SetWord(newInterval.GetBegin()),
         e = SetWord(newInterval.GetEnd());
    (algM->DeleteObj(algebraId, typeId))( b );
    (algM->DeleteObj(algebraId, typeId))( e );

    if( ++i < GetIntervalCount() )
      Get( i, thisInterval );
  }
  result.EndBulkLoad( false );

  Word b = SetWord(thisInterval.GetBegin()),
       e = SetWord(thisInterval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( b );
  (algM->DeleteObj(algebraId, typeId))( e );
  b = SetWord(interval.GetBegin());
  e = SetWord(interval.GetEnd());
  (algM->DeleteObj(algebraId, typeId))( b );
  (algM->DeleteObj(algebraId, typeId))( e );
}

void Range::Maximum(StandardAttribute *result)
{
  assert( IsOrdered() && !IsEmpty() );

  Get( intervalCount-1, End, result );
}

void Range::Minimum(StandardAttribute *result)
{
  assert( IsOrdered() && !IsEmpty() );

  Get( 0, Begin, result );
}

const int Range::NoComponents()
{
  return intervalCount;
}

/*
3.3 List Representation

The list representation of a ~range($\alpha$)~ is

----    ( (i1b i1e) (i2b i2e) ... (inb ine) )
----

3.4 ~Out~-function

*/
static ListExpr
OutRange( ListExpr typeInfo, Word value )
{
  Range* range = (Range*)(value.addr);

  if( range->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
    ListExpr alphaInfo = nl->Second( typeInfo );
    int algebraId = nl->IntValue( nl->First( alphaInfo ) ),
        typeId = nl->IntValue( nl->Second( alphaInfo ) );
    ListExpr l = nl->TheEmptyList(), lastElem, intervalList;

    for( int i = 0; i < range->GetIntervalCount(); i++ )
    {
      Interval interval( (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( alphaInfo ).addr, 
                         (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( alphaInfo ).addr );
      range->Get( i, interval );
      intervalList = nl->TwoElemList( (algM->OutObj( algebraId, typeId ))( alphaInfo, SetWord(interval.GetBegin())),
                                      (algM->OutObj( algebraId, typeId ))( alphaInfo, SetWord(interval.GetEnd())));
      Word begin = SetWord(interval.GetBegin()),
           end   = SetWord(interval.GetEnd());
      (algM->DeleteObj(algebraId, typeId))( begin );
      (algM->DeleteObj(algebraId, typeId))( end );
      if (l == nl->TheEmptyList())
      {
        l = nl->Cons( intervalList, nl->TheEmptyList());
        lastElem = l;
      }
      else
        lastElem = nl->Append(lastElem, intervalList);
    }
    return l;
  }
}
/*
3.5 ~In~-function

*/
static Word
InRange( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  ListExpr alphaInfo = nl->Second( typeInfo );
  int algebraId = nl->IntValue( nl->First( alphaInfo ) ),
      typeId = nl->IntValue( nl->Second( alphaInfo ) );
  StandardAttribute *aux = (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( alphaInfo ).addr;
  int objSize = aux->Sizeof();
  Word w = SetWord( aux );
  (algM->DeleteObj(algebraId, typeId))( w );

  Range* range = new Range( SecondoSystem::GetLobFile(), algebraId, typeId, objSize );
  range->StartBulkLoad();

  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    if( nl->IsAtom( nl->First( first ) ) && nl->IsAtom( nl->Second( first ) ) )
    {
      Interval interval( (StandardAttribute *)(algM->InObj(algebraId, typeId))( alphaInfo, nl->First( first ), errorPos, errorInfo, correct ).addr, 
                         (StandardAttribute *)(algM->InObj(algebraId, typeId))( alphaInfo, nl->Second( first ), errorPos, errorInfo, correct ).addr );
      if( correct == false )
        return SetWord( Address(0) );
      range->Add( interval );
      Word begin = SetWord(interval.GetBegin()),
           end   = SetWord(interval.GetEnd());
      (algM->DeleteObj(algebraId, typeId))( begin );
      (algM->DeleteObj(algebraId, typeId))( end );
    }
    else
    {
      correct = false;
      return SetWord( Address(0) );
    }
  }
  range->EndBulkLoad( true );
  correct = true;
  return SetWord( range );
}

/*
3.6 ~Create~-function

*/
static Word
CreateRange( const ListExpr typeInfo )
{
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  ListExpr alphaInfo = nl->Second( typeInfo );
  int algebraId = nl->IntValue( nl->First( alphaInfo ) ),
      typeId = nl->IntValue( nl->Second( alphaInfo ) );
  StandardAttribute *aux = (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( alphaInfo ).addr;
  int objSize = aux->Sizeof();
  Word w = SetWord( aux );
  (algM->DeleteObj(algebraId, typeId))( w );
  return (SetWord( new Range( SecondoSystem::GetLobFile(), algebraId, typeId, objSize ) ));
}

/*
3.7 ~Delete~-function

*/
static void
DeleteRange( Word& w )
{
  ((Range *)w.addr)->Destroy();
  delete (Range *)w.addr;
  w.addr = 0;
}

/*
3.8 ~Close~-function

*/
static void
CloseRange( Word& w )
{
  delete (Range *)w.addr;
  w.addr = 0;
}

/*
3.9 ~Clone~-function

*/
static Word
CloneRange( const Word& w )
{
  return SetWord( 0 );
}

/*
3.10 ~Open~-function

*/
static bool
OpenRange( SmiRecord& valueRecord,
                const ListExpr typeInfo,
                Word& value )
{
  SmiRecordId recordId;

  valueRecord.Read( &recordId, sizeof( SmiRecordId ), 0 );
  Range *range = new Range( SecondoSystem::GetLobFile(), recordId );
  value = SetWord( range );

  return (true);
}

/*
3.11 ~Save~-function

*/
static bool
SaveRange( SmiRecord& valueRecord,
                const ListExpr typeInfo,
                Word& value )
{
  Range *range = (Range *)value.addr;
  SmiRecordId recordId = range->GetRecordId();

  valueRecord.Write( &recordId, sizeof( SmiRecordId ), 0 );
  
  return (true);
}

/*
3.12 function Describing the Signature of the Type Constructor

*/

static ListExpr
RangeProperty()
{
  return (nl->TwoElemList(
                nl->SymbolAtom("BASE"),
                nl->SymbolAtom("RANGE") ));
}

/*
3.13 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
static bool
CheckRange( ListExpr type, ListExpr& errorInfo )
{
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

  if ((nl->ListLength(type) == 2) && nl->IsEqual(nl->First(type), "range"))
  {
    return (algMgr->CheckKind("BASE", nl->Second(type), errorInfo));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("RANGE"), type));
    return false;
  }
}
/*
3.14 ~Cast~-function

*/
void* CastRange(void* addr)
{
  return ( 0 );
}
/*
3.15 Creation of the type constructor ~range~

*/
TypeConstructor range(
        "range",                                //name
        RangeProperty,                          //property function describing signature
        OutRange,               InRange,        //Out and In functions
        CreateRange,            DeleteRange,    //object creation and deletion
        OpenRange,              SaveRange,      // object open and save
        CloseRange,             CloneRange,     //object close and clone
        CastRange,                              //cast function
        CheckRange,                             //kind checking function
        0,                                              //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
5 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

5.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

5.1.1 Type mapping function RangeTypeMapBool1

It is for the operator ~isempty~ which have a ~range~ as input and ~bool~ result type.

*/
static ListExpr
RangeTypeMapBool1( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    if( nl->ListLength( nl->First( args ) ) == 2 )
    {
      ListExpr arg1 = nl->First( nl->First( args ) );
      if( nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType && nl->SymbolValue( arg1 ) == "range" )
        return (nl->SymbolAtom( "bool" ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.1.2 Type mapping function RangeRangeTypeMapBool

It is for the operators $=$, $\neq$, and ~intersects~ which have two 
~ranges~ as input and ~bool~ result type.

*/
static ListExpr
RangeRangeTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && nl->SymbolValue( nl->First( arg1 ) ) == "range" && 
          nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" && 
          nl->IsAtom( nl->Second( arg1 ) ) && nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
        return (nl->SymbolAtom( "bool" ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.1.3 Type mapping function RangeBaseTypeMapBool1

It is for the operator ~inside~ which have two ~ranges~ as input or a 
~BASE~ and a ~range~ in this order as arguments and ~bool~ as the result type.

*/
static ListExpr
RangeBaseTypeMapBool1( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg1 ) ) && nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
        return (nl->SymbolAtom( "bool" ));
    }
    else if( nl->IsAtom( arg1 ) && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg2 ) ) == nl->SymbolValue( arg1 ) )
        return (nl->SymbolAtom( "bool" ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.1.4 Type mapping function RangeBaseTypeMapBool2

It is for the operator ~before~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in any order as arguments and ~bool~ as the result type.

*/
static ListExpr
RangeBaseTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg1 ) ) && nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
        return (nl->SymbolAtom( "bool" ));
    }
    else if( nl->IsAtom( arg1 ) && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg2 ) ) == nl->SymbolValue( arg1 ) )
        return (nl->SymbolAtom( "bool" ));
    }
    else if( nl->ListLength( arg1 ) == 2 && nl->IsAtom( arg2 ) == 1 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg1 ) ) && nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( arg2 ) )
        return (nl->SymbolAtom( "bool" ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.1.5 Type mapping function RangeRangeTypeMapRange

It is for the operators ~intersection~, ~union~, and ~minus~ which have two 
~ranges~ as input and a ~range~ as result type.

*/
static ListExpr
RangeRangeTypeMapRange( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && nl->SymbolValue( nl->First( arg1 ) ) == "range" && 
          nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" && 
          nl->IsAtom( nl->Second( arg1 ) ) && nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
        return (nl->TwoElemList( nl->SymbolAtom( "range" ), nl->Second( arg1 ) ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.1.6 Type mapping function RangeTypeMapBase

It is for the aggregate operators ~min~, ~max~, and ~avg~ which have one
~range~ as input and a ~BASE~ as result type.

*/
static ListExpr
RangeTypeMapBase( ListExpr args )
{
  ListExpr errorInfo;
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

  if ( nl->ListLength( args ) == 1 )
  {
    if( nl->ListLength( nl->First( args ) ) == 2 )
    {
      ListExpr arg1 = nl->First( args );
      if( nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          algMgr->CheckKind("BASE", nl->Second( arg1 ), errorInfo) )
        return (nl->Second( arg1 ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.1.7 Type mapping function RangeTypeMapInt

It is for the ~no_components~ operator which have one
~range~ as input and a ~int~ as result type.

*/
static ListExpr
RangeTypeMapInt( ListExpr args )
{
  ListExpr errorInfo;
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

  if ( nl->ListLength( args ) == 1 )
  {
    if( nl->ListLength( nl->First( args ) ) == 2 )
    {
      ListExpr arg1 = nl->First( args );
      if( nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType &&
          nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          algMgr->CheckKind("BASE", nl->Second( arg1 ), errorInfo) )
        return (nl->SymbolAtom( "int" ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
5.1.8 The dummy model mapping:

*/
static Word
RangeNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

/*
5.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

5.2.1 Selection function ~SimpleSelect~

Is used for all non-overloaded operators.

*/
static int
SimpleSelect( ListExpr args )
{
  return (0);
}

/*
5.2.2 Selection function ~RangeSelectPredicates~

Is used for the ~inside~ and ~before~ operations.

*/
static int
RangeSelectPredicates( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 &&
      nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
      nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
      nl->IsAtom( nl->Second( arg1 ) ) && nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
      nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
      nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
    return (0);

  if( nl->IsAtom( arg1 ) && nl->ListLength( arg2 ) == 2 &&
      nl->IsAtom( nl->First( arg2 ) ) && nl->AtomType( nl->First( arg2 ) ) == SymbolType && nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
      nl->IsAtom( nl->Second( arg2 ) ) && nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
      nl->IsAtom( arg1 ) && nl->AtomType( arg1 ) == SymbolType &&
      nl->SymbolValue( nl->Second( arg2 ) ) == nl->SymbolValue( arg1 ) )
    return (1);

  if( nl->ListLength( arg1 ) == 2 && nl->IsAtom( arg2 ) &&
      nl->IsAtom( nl->First( arg1 ) ) && nl->AtomType( nl->First( arg1 ) ) == SymbolType && nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
      nl->IsAtom( nl->Second( arg1 ) ) && nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
      nl->IsAtom( arg2 ) && nl->AtomType( arg2 ) == SymbolType &&
      nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( arg2 ) )
    return (2);

  return (-1); // This point should never be reached
}

/*
5.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

5.3.1 Value mapping functions of operator ~isempty~

*/
static int
IsEmpty_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->IsEmpty() )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
5.3.2 Value mapping functions of operator $=$ (~equal~)

*/
static int
Equal_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range*)args[0].addr) == *((Range*)args[1].addr) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
5.3.3 Value mapping functions of operator $\neq$ (~not equal~)

*/
static int
NotEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range*)args[0].addr) != *((Range*)args[1].addr) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
5.3.4 Value mapping functions of operator ~intersects~

*/
static int
Intersects_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Intersects( *((Range*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
5.3.5 Value mapping functions of operator ~inside~

*/
static int
Inside_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Inside( *((Range*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

static int
Inside_ar( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[1].addr)->Contains( ((StandardAttribute*)args[0].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
5.3.6 Value mapping functions of operator ~before~

*/
static int
Before_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Before( *((Range*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

static int
Before_ar( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[1].addr)->After( ((StandardAttribute*)args[0].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

static int
Before_ra( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Before( ((StandardAttribute*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
5.3.7 Value mapping functions of operator ~intersection~

*/
static int
Intersection_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Intersection( *((Range*)args[1].addr), (*(Range*)result.addr) );
  return (0);
}

/*
5.3.8 Value mapping functions of operator ~union~

*/
static int
Union_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Union( *((Range*)args[1].addr), (*(Range*)result.addr) );
  return (0);
}

/*
5.3.9 Value mapping functions of operator ~minus~

*/
static int
Minus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Minus( *((Range*)args[1].addr), (*(Range*)result.addr) );
  return (0);
}

/*
5.3.10 Value mapping functions of operator ~min~

*/
static int
Minimum_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
  {
    ((StandardAttribute *)result.addr)->SetDefined( false );
  }
  else
  {
    ((Range*)args[0].addr)->Minimum( (StandardAttribute *)result.addr);
  }
  return (0);
}

/*
5.3.10 Value mapping functions of operator ~max~

*/
static int
Maximum_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
  {
    ((StandardAttribute *)result.addr)->SetDefined( false );
  }
  else
  {
    ((Range*)args[0].addr)->Maximum( (StandardAttribute *)result.addr);
  }
  return (0);
}

/*
5.3.11 Value mapping functions of operator ~max~

*/
static int
NoComponents_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)->Set( true, ((Range*)args[0].addr)->NoComponents() );
  return (0);
}

/*
5.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

*/
ValueMapping rangeisemptymap[] = { IsEmpty_r };
ValueMapping rangeequalmap[] = { Equal_rr };
ValueMapping rangenotequalmap[] = { NotEqual_rr };
ValueMapping rangeintersectsmap[] = { Intersects_rr };
ValueMapping rangeinsidemap[] = { Inside_rr, Inside_ar };
ValueMapping rangebeforemap[] = { Before_rr, Before_ar, Before_ra };
ValueMapping rangeintersectionmap[] = { Intersection_rr };
ValueMapping rangeunionmap[] = { Union_rr };
ValueMapping rangeminusmap[] = { Minus_rr };
ValueMapping rangeminmap[] = { Minimum_r };
ValueMapping rangemaxmap[] = { Maximum_r };
ValueMapping rangenocomponentsmap[] = { NoComponents_r };

ModelMapping rangenomodelmap[] = { RangeNoModelMapping, RangeNoModelMapping,
                                   RangeNoModelMapping, RangeNoModelMapping,
                                   RangeNoModelMapping, RangeNoModelMapping };

const string RangeSpecIsEmpty = "(<text> (range x) -> bool</text---><text> Returns whether the range is empty or not. </text--->)";
const string RangeSpecEqual = "(<text> ( (range x) (range x) ) -> bool</text---><text> Equal. </text--->)";
const string RangeSpecNotEqual = "(<text> ( (range x) (range x) ) -> bool</text---><text> Not equal. </text--->)";
const string RangeSpecIntersects = "(<text> ( (range x) (range x) ) -> bool</text---><text> Intersects. </text--->)";
const string RangeSpecInside = "(<text> ( (range x) (range x) ) -> bool, ( x (range x) ) -> bool</text---><text> Inside. </text--->)";
const string RangeSpecBefore = "(<text> ( (range x) (range x) ) -> bool, ( x (range x) ) -> bool, ( (range x) x ) -> bool</text---><text> Inside. </text--->)";
const string RangeSpecIntersection = "(<text> ( (range x) (range x) ) -> (range x)</text---><text> Intersection. </text--->)";
const string RangeSpecUnion = "(<text> ( (range x) (range x) ) -> (range x)</text---><text> Union. </text--->)";
const string RangeSpecMinus = "(<text> ( (range x) (range x) ) -> (range x)</text---><text> Minus. </text--->)";
const string RangeSpecMinimum = "(<text> (range x) -> x</text---><text> Minimum. </text--->)";
const string RangeSpecMaximum = "(<text> (range x) -> x</text---><text> Maximum. </text--->)";
const string RangeSpecNoComponents = "(<text> (range x) -> int</text---><text> Number of components. </text--->)";

Operator rangeisempty( "isempty", RangeSpecIsEmpty, 1, rangeisemptymap, rangenomodelmap, SimpleSelect, RangeTypeMapBool1 );
Operator rangeequal( "=", RangeSpecEqual, 1, rangeequalmap, rangenomodelmap, SimpleSelect, RangeRangeTypeMapBool );
Operator rangenotequal( "#", RangeSpecNotEqual, 1, rangenotequalmap, rangenomodelmap, SimpleSelect, RangeRangeTypeMapBool );
Operator rangeintersects( "intersects", RangeSpecIntersects, 1, rangeintersectsmap, rangenomodelmap, SimpleSelect, RangeRangeTypeMapBool );
Operator rangeinside( "inside", RangeSpecInside, 2, rangeinsidemap, rangenomodelmap, RangeSelectPredicates, RangeBaseTypeMapBool1 );
Operator rangebefore( "before", RangeSpecBefore, 3, rangebeforemap, rangenomodelmap, RangeSelectPredicates, RangeBaseTypeMapBool2 );
Operator rangeintersection( "intersection", RangeSpecIntersection, 1, rangeintersectionmap, rangenomodelmap, SimpleSelect, RangeRangeTypeMapRange );
Operator rangeunion( "union", RangeSpecUnion, 1, rangeunionmap, rangenomodelmap, SimpleSelect, RangeRangeTypeMapRange );
Operator rangeminus( "minus", RangeSpecMinus, 1, rangeminusmap, rangenomodelmap, SimpleSelect, RangeRangeTypeMapRange );
Operator rangemin( "min", RangeSpecMinimum, 1, rangeminmap, rangenomodelmap, SimpleSelect, RangeTypeMapBase );
Operator rangemax( "max", RangeSpecMaximum, 1, rangemaxmap, rangenomodelmap, SimpleSelect, RangeTypeMapBase );
Operator rangenocomponents( "no_components", RangeSpecNoComponents, 1, rangenocomponentsmap, rangenomodelmap, SimpleSelect, RangeTypeMapInt );

/*
6 Creating the Algebra

*/

class RangeAlgebra : public Algebra
{
 public:
  RangeAlgebra() : Algebra()
  {
    AddTypeConstructor( &range );

    range.AssociateKind( "RANGE" );

    AddOperator( &rangeisempty );
    AddOperator( &rangeequal );
    AddOperator( &rangenotequal );
    AddOperator( &rangeintersects );
    AddOperator( &rangeinside );
    AddOperator( &rangebefore );
    AddOperator( &rangeintersection );
    AddOperator( &rangeunion );
    AddOperator( &rangeminus );
    AddOperator( &rangemin );
    AddOperator( &rangemax );
    AddOperator( &rangenocomponents );
  }
  ~RangeAlgebra() {};
};

RangeAlgebra rangeAlgebra;

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeRangeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&rangeAlgebra);
}

