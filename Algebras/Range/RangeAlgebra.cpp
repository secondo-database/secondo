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

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 Type Constructor ~range~

This type constructor implements the carrier set for ~range($\alpha$)~.

3.1 Implementation of struct ~Interval~

*/
Interval::Interval( const int algebraId, const int typeId ) :
algebraId( algebraId ),
typeId( typeId ),
start( NULL ),
end( NULL ),
lc( lc ),
rc( rc )
{
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  start = (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr;
  end   = (StandardAttribute *)(algM->CreateObj(algebraId, typeId))( nl->TheEmptyList() ).addr;
}

Interval::Interval( const Interval& interval ):
algebraId( interval.algebraId ),
typeId( interval.typeId ),
start( (StandardAttribute*)interval.start->Clone() ),
end( (StandardAttribute*)interval.end->Clone() ),
lc( interval.lc ),
rc( interval.rc )
{
}

Interval::Interval( const int algebraId,
                    const int typeId,
                    StandardAttribute* start,
                    StandardAttribute* end,
                    const bool lc,
                    const bool rc ) :
algebraId( algebraId ),
typeId( typeId ),
start( (StandardAttribute*)start->Clone() ),
end( (StandardAttribute*)end->Clone() ),
lc( lc ),
rc( rc )
{
}
 
Interval::~Interval()
{
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  Word wstart = SetWord( start ),
       wend   = SetWord( end );

  (algM->DeleteObj(algebraId, typeId))( wstart );
  (algM->DeleteObj(algebraId, typeId))( wend );
}

bool Interval::IsValid() const
{
  if( !IsDefined() )
    return false;

  if( start->Compare( end ) < 0 )
  {
    if( start->Adjacent( end ) )
      return lc || rc;
  }
  else if( start->Compare( end ) == 0 )
  {
    return rc && lc;
  }

  return true;
}

Interval& Interval::operator=( const Interval& interval )
{
  assert( algebraId == interval.algebraId &&
          typeId == interval.typeId );

  if( start != NULL )
  {
    assert( start->IsDefined() && 
            end != NULL && end->IsDefined() );

    AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
    Word s = SetWord(start),
         e = SetWord(end);
    (algM->DeleteObj(algebraId, typeId))( s );
    (algM->DeleteObj(algebraId, typeId))( e );
  }
    
  start = (StandardAttribute*)interval.start->Clone();
  end = (StandardAttribute*)interval.end->Clone();
  lc = interval.lc;
  rc = interval.rc;

  return *this;
}
 
bool Interval::operator==(const Interval& i) const
{ 
  assert( IsDefined() && i.IsDefined() );
  return( ( start->Compare( i.start ) == 0 && lc == i.lc && 
            end->Compare( i.end ) == 0 && rc == i.rc ) ||
          ( start->Compare( i.start ) == 0 && lc == i.lc && 
            end->Compare( i.end ) != 0 && end->Adjacent( i.end ) && ( !rc || !i.rc ) ) ||
          ( end->Compare( i.end ) == 0 && rc == i.rc && 
            start->Compare( i.start ) != 0 && start->Adjacent( i.start ) && ( !lc || !i.lc ) ) ||
          ( start->Compare( i.start ) != 0 && start->Adjacent( i.start ) && (!lc || !i.lc) && 
            end->Compare( i.end ) != 0 && end->Adjacent( i.end ) && ( !rc || !i.rc ) ) ); 
}

bool Interval::operator!=(const Interval& i) const
{ 
  assert( IsDefined() && i.IsDefined() );
  return !( *this == i ); 
}

bool Interval::R_Disjoint(const Interval& i) const
{
  return( end->Compare( i.start ) < 0 ||
          ( end->Compare( i.start ) == 0 && !( rc && i.lc ) ) );
}

bool Interval::Disjoint(const Interval& i) const
{
  return( R_Disjoint( i ) || i.R_Disjoint( *this ) );
}

bool Interval::R_Adjacent(const Interval& i) const
{
  return( Disjoint( i ) && 
          ( end->Compare( i.start ) == 0 && (rc || i.lc) ) ||
          ( ( end->Compare( i.start ) < 0 && rc && i.lc ) && end->Adjacent( i.start ) ) );
}

bool Interval::Adjacent(const Interval& i) const
{
  return( R_Adjacent( i ) || i.R_Adjacent( *this ) );
}

bool Interval::Inside(const Interval& i) const
{ 
  assert( IsDefined() && i.IsDefined() );

  return( ( start->Compare( i.start ) > 0 ||
            ( start->Compare( i.start ) == 0 && ( !lc || i.lc ) ) ) &&
          ( end->Compare( i.end ) < 0 ||
            ( end->Compare( i.end ) == 0 && ( !rc || i.rc ) ) ) );   
}

bool Interval::Contains(StandardAttribute* a) const
{ 
  assert( IsDefined() && a->IsDefined() );
  return ( ( start->Compare( a ) < 0 || 
             ( start->Compare( a ) == 0 && lc ) ) && 
           ( end->Compare( a ) > 0 ||
             ( end->Compare( a ) == 0 && rc ) ) ); 
}

bool Interval::Intersects(const Interval& i) const
{ 
  assert( IsDefined() && i.IsDefined() );
  return !( 
            ( ( (start->Compare( i.start ) < 0) ||
                (start->Compare( i.start ) == 0 && !(lc && i.lc)) ) && 
              ( (end->Compare( i.start ) < 0) ||
                (end->Compare( i.start ) == 0 && !(rc && i.lc)) ) ) ||
            ( ( (start->Compare( i.end ) > 0) ||
                (start->Compare( i.end ) == 0 && !(lc && i.rc)) ) &&
              ( (end->Compare( i.end ) > 0) ||
                (end->Compare( i.end ) == 0 && !(rc && i.rc)) ) ) 
          );
}

bool Interval::Before(StandardAttribute* a) const
{ 
  assert( IsDefined() && a->IsDefined() );
  return ( end->Compare( a ) <= 0 );
}

bool Interval::After(StandardAttribute* a) const
{ 
  assert( IsDefined() && a->IsDefined() );
  return ( start->Compare( a ) >= 0 );
}

bool Interval::Before(const Interval& i) const
{ 
  assert( IsDefined() && i.IsDefined() );
  return ( Before( i.start ) ); 
}

/*
3.2 Implementation of class ~Range~

*/
Range::Range( const int algebraId, const int typeId, const int size, const int n ):
algebraId( algebraId ),
typeId( typeId ),
size( size ),
canDelete( false ),
noComponents( 0 ),
ordered( true ),
intervals( n * 2 * ( size + sizeof(bool) ) )
{
}

Range::~Range()
{
  if ( canDelete )
    intervals.Destroy();
}

void Range::Clear()
{
  noComponents = 0;
  ordered = true;
  intervals.Clear();
}

void Range::Add( const Interval& interval )
{
  assert ( interval.start != NULL && interval.end != NULL );
  assert ( interval.start->IsDefined() && interval.end->IsDefined() );

  if( intervals.Size() < (noComponents + 1) * ( 2 * ( size + (int)sizeof(bool) ) ) )
    intervals.Resize( (noComponents + 1) * ( 2 * ( size + (int)sizeof(bool) ) ) );
  Put( noComponents, interval );
  noComponents++;
}

void Range::Put( const int index, const Interval& interval )
{
  intervals.Put( index * ( 2 * ( size + sizeof(bool) ) ), size, interval.start );
  intervals.Put( index * ( 2 * ( size + sizeof(bool) ) ) + size, size, interval.end );
  intervals.Put( index * ( 2 * ( size + sizeof(bool) ) ) + ( 2 * size ), sizeof(bool), &interval.lc );
  intervals.Put( index * ( 2 * ( size + sizeof(bool) ) ) + ( 2 * size ) + sizeof(bool), sizeof(bool), &interval.rc );
}

void Range::Get( const int index, Interval& interval )
{
  assert ( 0 <= index && index < noComponents );
  assert ( interval.start != NULL && interval.end != NULL );
 
  void *buf = malloc( size ); 
  intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ), size, buf );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  memcpy( interval.start, (StandardAttribute *)(algM->Cast(algebraId, typeId))( buf ), size );

  intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ) + size, size, buf );
  memcpy( interval.end, (StandardAttribute *)(algM->Cast(algebraId, typeId))( buf ), size );

  intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ) + ( 2 * size ), sizeof(bool), &interval.lc );
  intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ) + ( 2 * size ) + sizeof(bool), sizeof(bool), &interval.rc );
 
  free( buf ); 
  assert( interval.start->IsDefined() && interval.end->IsDefined() );
}

void Range::Get( const int index, const IntervalPosition pos, StandardAttribute *a, bool& closed )
{
  assert ( 0 <= index && index < noComponents );
  assert ( a != NULL );

  if( pos == Begin )
  {
    void *buf = malloc( size ); 
    intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ), size, buf );

    AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
    memcpy( a, (StandardAttribute *)(algM->Cast(algebraId, typeId))( buf ), size );
    free( buf );

    intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ) + size + sizeof(bool), sizeof(bool), &closed );
  }
  else
  {
    void *buf = malloc( size ); 
    intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ) + size, size, buf );

    AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
    a = (StandardAttribute *)(algM->Cast(algebraId, typeId))( buf );
    free( buf );

    intervals.Get( index * ( 2 * ( size + sizeof(bool) ) ) + size + 2 * sizeof(bool), sizeof(bool), &closed );
  }

  assert( a->IsDefined() );
}

void Range::Destroy()
{
  canDelete = true;
}

bool IntervalCompare( const Interval& a, const Interval& b )
{
  if( a.Before( b ) )
    return true;
  else
    return false;
}

void Range::Sort()
{
  if( noComponents > 1 )
  {
    vector<Interval> intervalArray;

    for( int i = 0; i < noComponents; i++ )
    {
      Interval interval( algebraId, typeId );
      Get( i, interval );
      intervalArray.push_back( interval );
    }

    sort( intervalArray.begin(), intervalArray.end(), IntervalCompare );

    for( size_t i = 0; i < intervalArray.size(); i++ )
      Put( i, intervalArray[i] );
  }
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
  assert( IsValid() );
}

bool Range::IsOrdered() const
{
  return ordered;
}

bool Range::operator==(Range& r)
{
  if( GetNoComponents() != r.GetNoComponents() )
    return false;

  bool result = true;
  Interval thisInterval( algebraId, typeId ), 
           interval( algebraId, typeId );

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, thisInterval );
    r.Get( i, interval );
   
    if( thisInterval != interval )
    {
      result = false;
      break;
    }
  }

  return result;
}

bool Range::operator!=(Range& r)
{
 return !( *this == r );
}

bool Range::Intersects(Range& r)
{
  if( IsEmpty() || r.IsEmpty() )
    return false;

  bool result = false;
  Interval thisInterval( algebraId, typeId ), 
           interval( algebraId, typeId );
  
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
      if( ++i == GetNoComponents() )
      {
        result = false;
        break;
      }
      Get( i, thisInterval );
    }
    
    if( interval.Before( thisInterval ) )
    {
      if( ++j == r.GetNoComponents() )
      {
        result = false;
        break;
      }
      r.Get( j, interval );
    }
  }

  return result;
}

bool Range::Inside(Range& r)
{
  if( IsEmpty() ) return true;
  if( r.IsEmpty() ) return false;

  bool result = true;
  Interval thisInterval( algebraId, typeId ), 
           interval( algebraId, typeId );

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  while( 1 )
  {
    if( interval.Before( thisInterval ) )
    {
      if( ++j == r.GetNoComponents() )
      {
        result = false;
        break;
      }
      r.Get( j, interval );
    }
    else if( thisInterval.Inside( interval ) )
    {
      if( ++i == GetNoComponents() )
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
      // Intersects but not inside.
      result = false;
      break;
    }
  }

  return result;
}

bool Range::Contains(StandardAttribute *a)
{
  assert( IsOrdered() && a->IsDefined() );

  if( IsEmpty() )
    return false;

  bool result = false;
  Interval midInterval( algebraId, typeId );

  int first = 0, last = GetNoComponents() - 1;

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

  return result;
}

bool Range::Before(Range& r)
{
  assert( !IsEmpty() && !r.IsEmpty() );

  bool result = true;
  Interval thisInterval( algebraId, typeId ), 
           interval( algebraId, typeId );

  Get( GetNoComponents() - 1, thisInterval );
  r.Get( 0, interval );
  result = thisInterval.Before( interval );

  return result;
}

bool Range::Before(StandardAttribute *a)
{
  assert( !IsEmpty() && a->IsDefined() );

  bool result = true;
  Interval thisInterval( algebraId, typeId );

  Get( GetNoComponents() - 1, thisInterval );
  result = thisInterval.Before( a );

  return result;
}

bool Range::After(StandardAttribute *a)
{
  assert( !IsEmpty() && a->IsDefined() );

  bool result = true;
  Interval thisInterval( algebraId, typeId );

  Get( 0, thisInterval );
  result = thisInterval.After( a );

  return result;
}

void Range::Intersection(Range& r, Range& result)
{
  assert( IsOrdered() && r.IsOrdered() && result.IsEmpty() );

  Interval thisInterval( algebraId, typeId ), 
           interval( algebraId, typeId );

  int i = 0, j = 0;
  Get( i, thisInterval );
  r.Get( j, interval );

  result.StartBulkLoad();
  while( i < GetNoComponents() && j < r.GetNoComponents() )
  {
    if( thisInterval.start->Compare( interval.start ) == 0 && 
        thisInterval.end->Compare( interval.end ) == 0 )
    {
      Interval newInterval( algebraId, typeId,
                            thisInterval.start, thisInterval.end,
                            thisInterval.lc && interval.lc, 
                            thisInterval.rc && interval.rc );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
    else if( thisInterval.Inside( interval ) )
    {
      Interval newInterval( thisInterval );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
    }
    else if( interval.Inside( thisInterval ) )
    {
      Interval newInterval( interval );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
    else if( thisInterval.Intersects( interval ) )
    {
      if( thisInterval.start->Compare( interval.end ) == 0 && ( thisInterval.lc && interval.rc ) )
      {
        Interval newInterval( algebraId, typeId, 
                              interval.end, interval.end, true, true );
        result.Add( newInterval );
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval.end->Compare( interval.start ) == 0 && ( thisInterval.rc && interval.lc ) )
      {
        Interval newInterval( algebraId, typeId, 
                              interval.start, interval.start, true, true );
        result.Add( newInterval );
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( thisInterval.start->Compare( interval.start ) < 0 )
      {
        Interval newInterval( algebraId, typeId, 
                              interval.start, thisInterval.end, interval.lc, thisInterval.rc );
        if( newInterval.IsValid() )
          result.Add( newInterval );
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( thisInterval.start->Compare( interval.start ) == 0 )
      {
        assert( !thisInterval.lc || !interval.lc );
        if( thisInterval.end->Compare( interval.end ) > 0 )
        {
          Interval newInterval( algebraId, typeId, 
                                interval.start, interval.end, interval.lc && thisInterval.lc, interval.rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else
        {
          assert( thisInterval.end->Compare( interval.end ) < 0 );
          Interval newInterval( algebraId, typeId, 
                                thisInterval.start, thisInterval.end, interval.lc && thisInterval.lc, thisInterval.rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
      }
      else
      {
        Interval newInterval( algebraId, typeId,
                              thisInterval.start, interval.end, thisInterval.lc, interval.rc );
        if( newInterval.IsValid() )
        result.Add( newInterval );
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
    else if( thisInterval.start->Compare( interval.start ) <= 0 )
    {
      if( ++i < GetNoComponents() )
        Get( i, thisInterval );
    }
    else
    {
      if( ++j < r.GetNoComponents() )
        r.Get( j, interval );
    }
  }
  result.EndBulkLoad( false );
}

void Range::Union(Range& r, Range& result)
{
  assert( IsOrdered() && r.IsOrdered() && result.IsEmpty() );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  Interval thisInterval( algebraId, typeId ), 
           interval( algebraId, typeId );

  result.StartBulkLoad();
  int i = 0, j = 0;

  if( !IsEmpty() )
    Get( i, thisInterval );
  if( !r.IsEmpty() )
    r.Get( j, interval );

  if( !IsEmpty() && !r.IsEmpty() )
  {
    StandardAttribute *start = NULL, *end = NULL;
    bool lc = false, rc = false;

    while( i < GetNoComponents() && j < r.GetNoComponents() )
    {
      if( thisInterval.start->Compare( interval.start ) == 0 &&
          thisInterval.end->Compare( interval.end ) == 0 )
      {
        Interval newInterval( algebraId, typeId,
                              thisInterval.start, thisInterval.end,
                              thisInterval.lc || interval.lc, 
                              thisInterval.rc || interval.rc );
        result.Add( newInterval );
  
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
        
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( interval.Inside( thisInterval ) )
      {
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval.Inside( interval ) )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( !thisInterval.Intersects( interval ) )
      {
        if( thisInterval.end->Compare( interval.start ) < 0 )
        {
          if( thisInterval.Adjacent( interval ) )
          {
            if( start != NULL && end != NULL )
            {
              Word e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( e );
            }
            else
            {
              start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.start ) ).addr;
              lc = thisInterval.lc;
            }
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
            rc = interval.rc;
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              Interval newInterval( algebraId, typeId,
                                    start, end, lc, rc );
              result.Add( newInterval );
              Word s = SetWord(start),
                   e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( s );
              (algM->DeleteObj(algebraId, typeId))( e );
              start = NULL; end   = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval newInterval( thisInterval );
              result.Add( newInterval );
            }
          }
    
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else if( thisInterval.start->Compare( interval.end ) > 0 )
        {
          if( thisInterval.Adjacent( interval ) )
          {
            if( start != NULL && end != NULL )
            {
              Word e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( e );
            }
            else
            {
              start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.start ) ).addr;
              lc = interval.lc;
            }
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
            rc = thisInterval.rc;
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              Interval newInterval( algebraId, typeId,
                                    start, end, lc, rc );
              result.Add( newInterval );
              Word s = SetWord(start),
                   e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( s );
              (algM->DeleteObj(algebraId, typeId))( e );
              start = NULL; end   = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval newInterval( interval );
              result.Add( newInterval );
            }
          }
    
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( thisInterval.start->Compare( interval.end ) == 0 )
        {
          if( !thisInterval.lc && !interval.rc )
          {
            if( start != NULL && end != NULL )
            {
              Interval newInterval( algebraId, typeId,
                                    start, end, lc, rc );
              result.Add( newInterval );
              Word s = SetWord(start),
                   e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( s );
              (algM->DeleteObj(algebraId, typeId))( e );
              start = NULL; end   = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval newInterval( interval );
              result.Add( newInterval );
            }
          }
          else
          {
            if( start != NULL && end != NULL ) 
            {
              if( end->Compare( thisInterval.end ) < 0 )
              {
                Word e = SetWord(end);
                (algM->DeleteObj(algebraId, typeId))( e );
                end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
                rc = thisInterval.rc;
              }
              else if( end->Compare( thisInterval.end ) == 0 )
              {
                rc = rc || thisInterval.rc;
              }
            }
            else
            {
              start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.start ) ).addr;
              lc = interval.lc;
              end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
              rc = thisInterval.rc;
            }
          }
   
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( interval.start->Compare( thisInterval.end ) == 0 )
        {
          if( !interval.lc && !thisInterval.rc )
          {
            if( start != NULL && end != NULL )
            {
              Interval newInterval( algebraId, typeId, start, end, lc, rc );
              result.Add( newInterval );
              Word s = SetWord(start),
                   e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( s );
              (algM->DeleteObj(algebraId, typeId))( e );
              start = NULL; end   = NULL;
              lc = false; rc = false;
            }
            else
            {
              Interval newInterval( thisInterval );
              result.Add( newInterval );
            }
          }
          else
          {
            if( start != NULL && end != NULL )
            {
              if( end->Compare( interval.end ) < 0 )
              {
                Word e = SetWord(end);
                (algM->DeleteObj(algebraId, typeId))( e );
                end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
                rc = interval.rc;
              }
              else if( end->Compare( interval.end ) == 0 )
              {
                rc = rc || interval.rc;
              }
            }
            else
            {
              start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.start ) ).addr;
              lc = thisInterval.lc;
              end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
              rc = interval.rc;
            }
          }
  
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
      }
      else if( thisInterval.start->Compare( interval.start ) < 0 )
      {
        if( start == NULL && end == NULL )
        {
          start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.start ) ).addr;
          lc = thisInterval.lc;
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
          rc = interval.rc;
        }
        else
        {
          if( end->Compare( interval.end ) < 0 )
          {
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
            rc = interval.rc;
          }
          if( end->Compare( interval.end ) == 0 )
          {
            rc = rc || interval.rc;
          }
        }
  
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      } 
      else if( interval.start->Compare( thisInterval.start ) < 0 )
      {
        if( start == NULL && end == NULL )
        {
          start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.start ) ).addr;
          lc = interval.lc;
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
          rc = thisInterval.rc;
        }
        else
        {
          if( end->Compare( thisInterval.end ) < 0 )
          {
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
            rc = thisInterval.rc;
          }
          if( end->Compare( thisInterval.end ) == 0 )
          {
            rc = rc || thisInterval.rc;
          }
        }

        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( thisInterval.start->Compare( interval.start ) == 0 )
      {
        assert( start == NULL && end == NULL );
        start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.start ) ).addr;
        lc = thisInterval.lc || interval.lc;
        if( thisInterval.end->Compare( interval.end ) < 0 )
        {
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
          rc = interval.rc;

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else
        {
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
          rc = thisInterval.rc;
  
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else if( thisInterval.end->Compare( interval.end ) == 0 )
      {
        assert( start != NULL && end != NULL );
        rc = thisInterval.rc || interval.rc;
  
        Interval newInterval( algebraId, typeId, start, end, lc, rc );
        result.Add( newInterval );
        Word s = SetWord(start),
             e = SetWord(end);
        (algM->DeleteObj(algebraId, typeId))( s );
        (algM->DeleteObj(algebraId, typeId))( e );
        start = NULL; end   = NULL;
        lc = false; rc = false;
  
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
  
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
  
    if( start != NULL && end != NULL )
    {
      Interval newInterval( algebraId, typeId, start, end, lc, rc );
      result.Add( newInterval );
      Word s = SetWord(start),
           e = SetWord(end);
      (algM->DeleteObj(algebraId, typeId))( s );
      (algM->DeleteObj(algebraId, typeId))( e );
      start = end = NULL;
      lc = rc = false;
  
      if( j >= r.GetNoComponents() )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( i >= GetNoComponents() )
      {
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
  }

  while( i < GetNoComponents() )
  {
    Interval newInterval( thisInterval );
    result.Add( newInterval );

    if( ++i < GetNoComponents() )
      Get( i, thisInterval );
  }

  while( j < r.GetNoComponents() )
  {
    Interval newInterval( interval );
    result.Add( newInterval );

    if( ++j < r.GetNoComponents() )
      r.Get( j, interval );
  }
  result.EndBulkLoad( false );
}

void Range::Minus(Range& r, Range& result)
{
  assert( IsOrdered() && r.IsOrdered() && result.IsEmpty() );
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  if( IsEmpty() ) 
    return;
  result.StartBulkLoad();

  Interval thisInterval( algebraId, typeId ), 
           interval( algebraId, typeId );

  int i = 0, j = 0;
  Get( i, thisInterval );

  if( !r.IsEmpty() )
  {
    r.Get( j, interval );

    StandardAttribute *start = NULL, *end = NULL;
    bool lc = false, rc = false;

    while( i < GetNoComponents() && j < r.GetNoComponents() )
    {
      if( thisInterval.start->Compare( interval.start ) == 0 &&
          thisInterval.end->Compare( interval.end ) == 0 )
      {
        if( thisInterval.lc && !interval.lc )
        {
          Interval newInterval( algebraId, typeId,
                                thisInterval.start, thisInterval.start, true, true );
          result.Add( newInterval );
        }
        if( thisInterval.rc && !interval.rc )
        {
          Interval newInterval( algebraId, typeId,
                                thisInterval.end, thisInterval.end, true, true );
          result.Add( newInterval );
        }
  
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
      else if( !thisInterval.Intersects( interval ) )
      {
        if( start != NULL && end != NULL )
        { 
          Interval newInterval( algebraId, typeId, start, end, lc, rc );
          if( newInterval.IsValid() )
            result.Add( newInterval );
          Word s = SetWord(start),
               e = SetWord(end);
          (algM->DeleteObj(algebraId, typeId))( s );
          (algM->DeleteObj(algebraId, typeId))( e );
          start = end = NULL;
          lc = rc = false;
        }
        else if( thisInterval.start->Compare( interval.start ) <= 0 )
        {
          Interval newInterval( thisInterval );
          result.Add( newInterval );
        }

        if( thisInterval.start->Compare( interval.start ) <= 0 )
        {
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
        }
        else
        {
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else if( thisInterval.Inside( interval ) )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( interval.Inside( thisInterval ) )
      {
        if( interval.start->Compare( thisInterval.start ) == 0 )
        {
          assert( start == NULL && end == NULL );
          if( thisInterval.lc && !interval.lc )
          {
            Interval newInterval( algebraId, typeId,
                                  thisInterval.start, thisInterval.start, true, true );
            result.Add( newInterval );
          }
          start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
          lc = !interval.rc;
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
          rc = thisInterval.rc;

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else if( interval.end->Compare( thisInterval.end ) == 0 )
        {
          if( start == NULL && end == NULL )
          {
            Interval newInterval( algebraId, typeId, 
                                  thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            Interval newInterval( algebraId, typeId, 
                                  start, interval.start, lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
            Word s = SetWord(start),
                 e = SetWord(end);
            (algM->DeleteObj(algebraId, typeId))( s );
            (algM->DeleteObj(algebraId, typeId))( e );
            start = NULL; end = NULL;
            lc = false; rc = false;
          }
  
          if( thisInterval.rc && !interval.rc )
          {
            Interval newInterval( algebraId, typeId, thisInterval.end, thisInterval.end, true, true );
            result.Add( newInterval );
          }
  
          if( ++i < GetNoComponents() )
            Get( i, thisInterval );
  
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
        else
        {
          assert( thisInterval.start->Compare( interval.start ) < 0 &&
                  thisInterval.end->Compare( interval.end ) > 0 );
          if( start == NULL && end == NULL )
          {
            Interval newInterval( algebraId, typeId,
                                  thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
          }
          else
          {
            assert( end->Compare( thisInterval.end ) == 0 && rc == thisInterval.rc );

            Interval newInterval( algebraId, typeId, 
                                  start, interval.start, lc, !interval.lc );
            if( newInterval.IsValid() )
              result.Add( newInterval );
            Word s = SetWord(start),
                 e = SetWord(end);
            (algM->DeleteObj(algebraId, typeId))( s );
            (algM->DeleteObj(algebraId, typeId))( e );
            start = end = NULL;
            lc = rc = false;
          }
            
          start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
          lc = !interval.rc;
          end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
          rc = thisInterval.rc;
  
          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
      else
      {
        assert( thisInterval.Intersects( interval ) );
        
        if( interval.start->Compare( thisInterval.start ) < 0 )
        {
          assert( start == NULL && end == NULL );
          
          if( interval.end->Compare( thisInterval.end ) == 0 )
          {
            if( thisInterval.rc && !interval.rc )
            {
              Interval newInterval( algebraId, typeId, 
                                    thisInterval.end, thisInterval.end, true, true );
              result.Add( newInterval );
            }
  
            if( ++i < GetNoComponents() )
              Get( i, thisInterval );
  
            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
          else
          {
            start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
            if( interval.end->Compare( thisInterval.start ) == 0 )
            {
              lc = thisInterval.lc && !interval.rc;
            }
            else
            {
              lc = !interval.rc;
            }
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
            rc = thisInterval.rc;
  
            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
        }
        else if( interval.start->Compare( thisInterval.start ) == 0 )
        {
          assert( start == NULL & end == NULL );

          if( thisInterval.lc && !interval.lc )
          {
            Interval newInterval( algebraId, typeId, thisInterval.start, thisInterval.start, true, true );
            result.Add( newInterval );
          }

          if( thisInterval.end->Compare( interval.end ) > 0 )
          {
            start = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.end ) ).addr;
            lc = !interval.rc;
            end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( thisInterval.end ) ).addr;
            rc = thisInterval.rc;

            if( ++j < r.GetNoComponents() )
              r.Get( j, interval );
          }
          else
          {
            assert( thisInterval.end->Compare( interval.end ) < 0 );
            if( ++i < GetNoComponents() )
              Get( i, thisInterval );
          }
        }
        else if( interval.end->Compare( thisInterval.end ) > 0 )
        {
          if( thisInterval.start->Compare( interval.start ) == 0 )
          {
            assert( start == NULL && end == NULL );
            cerr << "I think that there is an error here!!!" << endl;
          } 
          else
          {
            if( start != NULL && end != NULL )
            { 
              if( interval.start->Compare( start ) > 0 ||
                  ( interval.start->Compare( start ) == 0 && interval.lc && !lc ) )
              {
                Word e = SetWord(end);
                (algM->DeleteObj(algebraId, typeId))( e );
                end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.start ) ).addr;
                if( interval.start->Compare( thisInterval.end ) == 0 )
                  rc = thisInterval.rc && !interval.lc;
                else
                  rc = !interval.lc;
  
                Interval newInterval( algebraId, typeId, start, end, lc, rc );
                if( newInterval.IsValid() )
                  result.Add( newInterval );
              }
              Word s = SetWord(start),
                   e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( s );
              (algM->DeleteObj(algebraId, typeId))( e );
              start = end = NULL;
              lc = rc = false;
            }
            else
            {
              Interval newInterval( algebraId, typeId,
                                    thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
            }
            if( ++i < GetNoComponents() )
              Get( i, thisInterval );
          }
        }
        else
        {
          assert( interval.end->Compare( thisInterval.end ) == 0 );

          if( interval.start->Compare( thisInterval.start ) < 0 )
          {
            assert( start == NULL && end == NULL );
            if( thisInterval.rc && !interval.rc )
            {
              Interval newInterval( algebraId, typeId,
                                    interval.end, interval.end, true, true );
              result.Add( newInterval );
            }
          }
          else
          {
            assert( interval.start->Compare( thisInterval.start ) > 0 );

            if( start != NULL && end != NULL )
            {
              Word e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( e );
              end = (StandardAttribute *)(algM->CloneObj(algebraId, typeId))( SetWord( interval.start ) ).addr;
              rc = !interval.lc;

              Interval newInterval( algebraId, typeId, start, end, lc, rc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
              Word s = SetWord(start);
              e = SetWord(end);
              (algM->DeleteObj(algebraId, typeId))( s );
              (algM->DeleteObj(algebraId, typeId))( e );
              start = end = NULL;
              lc = rc = false;
            }
            else
            {
              Interval newInterval( algebraId, typeId, thisInterval.start, interval.start, thisInterval.lc, !interval.lc );
              if( newInterval.IsValid() )
                result.Add( newInterval );
            }
          }

          if( ++i < GetNoComponents() )
            Get( i, thisInterval );

          if( ++j < r.GetNoComponents() )
            r.Get( j, interval );
        }
      }
    }

    if( start != NULL && end != NULL )
    {
      Interval newInterval( algebraId, typeId, start, end, lc, rc );
      if( newInterval.IsValid() )
        result.Add( newInterval );
      Word s = SetWord(start),
           e = SetWord(end);
      (algM->DeleteObj(algebraId, typeId))( s );
      (algM->DeleteObj(algebraId, typeId))( e );
  
      if( j >= r.GetNoComponents() )
      {
        if( ++i < GetNoComponents() )
          Get( i, thisInterval );
      }
      else if( i >= GetNoComponents() )
      {
        if( ++j < r.GetNoComponents() )
          r.Get( j, interval );
      }
    }
  }

  while( i < GetNoComponents() )
  {
    Interval newInterval( thisInterval );
    result.Add( newInterval );

    if( ++i < GetNoComponents() )
      Get( i, thisInterval );
  }
  result.EndBulkLoad( false );
}

void Range::Maximum(StandardAttribute *result)
{
  assert( IsOrdered() && !IsEmpty() );
  assert( result != NULL );
  bool closed;
  Get( noComponents-1, End, result, closed );
}

void Range::Minimum(StandardAttribute *result)
{
  assert( IsOrdered() && !IsEmpty() );
  assert( result != NULL );
  bool closed;
  Get( 0, Begin, result, closed );
}

int Range::GetNoComponents() const
{
  return noComponents;
}

bool Range::IsValid()
{
  assert( IsOrdered() );

  if( IsEmpty() )
    return true;

  bool result = true;
  Interval lastInterval( algebraId, typeId ), 
           interval( algebraId, typeId );

  if( GetNoComponents() == 1 )
  {
    Get( 0, interval );
    return( interval.IsValid() );
  }

  for( int i = 1; i < GetNoComponents(); i++ )
  {
    Get( i-1, lastInterval );
    if( !lastInterval.IsValid() )
    {
      result = false; 
      break;
    }
    Get( i, interval );
    if( !interval.IsValid() )
    {
      result = false; 
      break;
    }
    if( !( lastInterval.Disjoint( interval ) && !lastInterval.Adjacent( interval ) ) )
    {
      result = false; 
      break;
    }
  }

  return result;
}

/*
3.3 List Representation

The list representation of a ~range($\alpha$)~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:
----    ( (1 5 TRUE FALSE) (6 9 FALSE FALSE) (11 11 TRUE TRUE) )
----

3.4 ~Out~-function

*/
ListExpr
OutRange( ListExpr typeInfo, Word value )
{
  Range* range = (Range*)(value.addr);

  if( range->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    assert( range->IsOrdered() );
    AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
    ListExpr alphaInfo = nl->Second( typeInfo );
    int algebraId = nl->IntValue( nl->First( alphaInfo ) ),
        typeId = nl->IntValue( nl->Second( alphaInfo ) );
    ListExpr l = nl->TheEmptyList(), lastElem, intervalList;

    for( int i = 0; i < range->GetNoComponents(); i++ )
    {
      Interval interval( algebraId, typeId );
      range->Get( i, interval );
      intervalList = nl->FourElemList( (algM->OutObj( algebraId, typeId ))( alphaInfo, SetWord(interval.start)),
                                       (algM->OutObj( algebraId, typeId ))( alphaInfo, SetWord(interval.end)),
                                       nl->BoolAtom( interval.lc ), nl->BoolAtom( interval.rc));
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
Word
InRange( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct )
{
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  ListExpr alphaInfo = nl->Second( typeInfo );

  int algebraId = nl->IntValue( nl->First( alphaInfo ) ),
      typeId = nl->IntValue( nl->Second( alphaInfo ) );
  int objSize = (algM->SizeOfObj(algebraId, typeId))();

  Range* range = new Range( algebraId, typeId, objSize );
  range->StartBulkLoad();

  ListExpr rest = instance;
  while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->First( first ) ) &&
        nl->IsAtom( nl->Second( first ) ) && 
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType ) 
    {
      StandardAttribute *start = 
                          (StandardAttribute *)
                          (algM->InObj(algebraId, typeId))( alphaInfo, nl->First( first ), errorPos, errorInfo, correct ).addr;

      if( correct == false )
      {
        return SetWord( Address(0) );
      }

      StandardAttribute *end = 
                          (StandardAttribute *)
                          (algM->InObj(algebraId, typeId))( alphaInfo, nl->Second( first ), errorPos, errorInfo, correct ).addr;

      if( correct == false )
      {
        Word wstart = SetWord( start );
        (algM->DeleteObj(algebraId, typeId))( wstart );
        return SetWord( Address(0) );
      }

      Interval interval( algebraId, typeId, 
                         start, end, 
                         nl->BoolValue( nl->Third( first ) ), 
                         nl->BoolValue( nl->Fourth( first ) ) );

      Word wstart = SetWord( start ),
           wend   = SetWord( end );
      (algM->DeleteObj(algebraId, typeId))( wstart );
      (algM->DeleteObj(algebraId, typeId))( wend );
      
      range->Add( interval );
    }
    else
    {
      correct = false;
      return SetWord( Address(0) );
    }
  }
  range->EndBulkLoad( true );
  assert( range->IsOrdered() );
  correct = true;

  return SetWord( range );
}

/*
3.6 ~Create~-function

*/
Word
CreateRange( const ListExpr typeInfo )
{
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  ListExpr alphaInfo = nl->Second( typeInfo );
  int algebraId = nl->IntValue( nl->First( alphaInfo ) ),
      typeId = nl->IntValue( nl->Second( alphaInfo ) );
  int objSize = (algM->SizeOfObj(algebraId, typeId))();
  return (SetWord( new Range( algebraId, typeId, objSize ) ));
}

/*
3.7 ~Delete~-function

*/
void
DeleteRange( Word& w )
{
  ((Range *)w.addr)->Destroy();
  delete (Range *)w.addr;
  w.addr = 0;
}

/*
3.8 ~Close~-function

*/
void
CloseRange( Word& w )
{
  delete (Range *)w.addr;
  w.addr = 0;
}

/*
3.9 ~Clone~-function

*/
Word
CloneRange( const Word& w )
{
  Range *r = (Range *)w.addr;
  return SetWord( r->Clone() );
}

/*
3.9 ~Sizeof~-function

*/
int
SizeOfRange()
{
  return sizeof(Range);
}

/*
3.12 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"BASE is either int, real, bool or string; "
  "lci means left closed interval, rci respectively right closed interval,"
  " e.g. (0 1 TRUE FALSE) means the range [0, 1[");
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("BASE -> RANGE"),
                             nl->StringAtom("(range <basetype>), e.g. "
                             "(range int)"),
                             nl->StringAtom("( (b1 e1 lci rci) ... "
                             "(bn en lci rci) )"),
                             nl->StringAtom("( (0 1 TRUE FALSE)"
                             "(2 5 TRUE TRUE) )"),
                             remarkslist)));
}

/*
3.13 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
bool
CheckRange( ListExpr type, ListExpr& errorInfo )
{
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

  if ((nl->ListLength(type) == 2) && nl->IsEqual(nl->First(type), "range"))
  {
    return (algMgr->CheckKind("BASE", nl->Second(type), errorInfo));
  }
  else
  {
    return false;
  }
}
/*
3.14 ~Cast~-function

*/
void* CastRange(void* addr)
{
  return new (addr) Range;
}
/*
3.15 Creation of the type constructor ~range~

*/
TypeConstructor range(
        "range",                        //name
        RangeProperty,                  //property function describing signature
        OutRange,       InRange,        //Out and In functions
        0,              0,              //SaveToList and RestoreFromList functions
        CreateRange,    DeleteRange,    //object creation and deletion
        0,              0,              // object open and save
        CloseRange,     CloneRange,     //object close and clone
        CastRange,                      //cast function
        SizeOfRange,                    //sizeof function
        CheckRange,                     //kind checking function
        0,                              //predef. pers. function for model
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
ListExpr
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
ListExpr
RangeRangeTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && 
          nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg1 ) ) == "range" && 
          nl->IsAtom( nl->First( arg2 ) ) && 
          nl->AtomType( nl->First( arg2 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg2 ) ) == "range" && 
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
ListExpr
RangeBaseTypeMapBool1( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && 
          nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          nl->IsAtom( nl->First( arg2 ) ) && 
          nl->AtomType( nl->First( arg2 ) ) == SymbolType &&  
          nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg1 ) ) && 
          nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( nl->Second( arg2 ) ) && 
          nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
        return (nl->SymbolAtom( "bool" ));
    }
    else if( nl->IsAtom( arg1 ) && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg2 ) ) && 
          nl->AtomType( nl->First( arg2 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
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
ListExpr
RangeBaseTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && 
          nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          nl->IsAtom( nl->First( arg2 ) ) && 
          nl->AtomType( nl->First( arg2 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg1 ) ) && 
          nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( nl->Second( arg2 ) ) && 
          nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
        return (nl->SymbolAtom( "bool" ));
    }
    else if( nl->IsAtom( arg1 ) && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg2 ) ) && 
          nl->AtomType( nl->First( arg2 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg2 ) ) && 
          nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
          nl->IsAtom( arg1 ) && 
          nl->AtomType( arg1 ) == SymbolType &&
          nl->SymbolValue( nl->Second( arg2 ) ) == nl->SymbolValue( arg1 ) )
        return (nl->SymbolAtom( "bool" ));
    }
    else if( nl->ListLength( arg1 ) == 2 && nl->IsAtom( arg2 ) == 1 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && 
          nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
          nl->IsAtom( nl->Second( arg1 ) ) && 
          nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
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
ListExpr
RangeRangeTypeMapRange( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 )
    {
      if( nl->IsAtom( nl->First( arg1 ) ) && 
          nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg1 ) ) == "range" && 
          nl->IsAtom( nl->First( arg2 ) ) && 
          nl->AtomType( nl->First( arg2 ) ) == SymbolType && 
          nl->SymbolValue( nl->First( arg2 ) ) == "range" && 
          nl->IsAtom( nl->Second( arg1 ) ) && 
          nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
          nl->IsAtom( nl->Second( arg2 ) ) && 
          nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
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
ListExpr
RangeTypeMapBase( ListExpr args )
{
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

  if ( nl->ListLength( args ) == 1 )
  {
    if( nl->ListLength( nl->First( args ) ) == 2 )
    {
      ListExpr arg1 = nl->First( args );
      if( nl->IsAtom( nl->First( arg1 ) ) && 
          nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
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
ListExpr
RangeTypeMapInt( ListExpr args )
{
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

  if ( nl->ListLength( args ) == 1 )
  {
    if( nl->ListLength( nl->First( args ) ) == 2 )
    {
      ListExpr arg1 = nl->First( args );
      if( nl->IsAtom( nl->First( arg1 ) ) && 
          nl->AtomType( nl->First( arg1 ) ) == SymbolType &&
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
Word
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

5.2.2 Selection function ~RangeSelectPredicates~

Is used for the ~inside~ and ~before~ operations.

*/
int
RangeSelectPredicates( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->ListLength( arg1 ) == 2 && nl->ListLength( arg2 ) == 2 &&
      nl->IsAtom( nl->First( arg1 ) ) && 
      nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
      nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
      nl->IsAtom( nl->First( arg2 ) ) && 
      nl->AtomType( nl->First( arg2 ) ) == SymbolType && 
      nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
      nl->IsAtom( nl->Second( arg1 ) ) && 
      nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
      nl->IsAtom( nl->Second( arg2 ) ) && 
      nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
      nl->SymbolValue( nl->Second( arg1 ) ) == nl->SymbolValue( nl->Second( arg2 ) ) )
    return (0);

  if( nl->IsAtom( arg1 ) && nl->ListLength( arg2 ) == 2 &&
      nl->IsAtom( nl->First( arg2 ) ) && 
      nl->AtomType( nl->First( arg2 ) ) == SymbolType && 
      nl->SymbolValue( nl->First( arg2 ) ) == "range" &&
      nl->IsAtom( nl->Second( arg2 ) ) && 
      nl->AtomType( nl->Second( arg2 ) ) == SymbolType &&
      nl->IsAtom( arg1 ) && 
      nl->AtomType( arg1 ) == SymbolType &&
      nl->SymbolValue( nl->Second( arg2 ) ) == nl->SymbolValue( arg1 ) )
    return (1);

  if( nl->ListLength( arg1 ) == 2 && nl->IsAtom( arg2 ) &&
      nl->IsAtom( nl->First( arg1 ) ) && 
      nl->AtomType( nl->First( arg1 ) ) == SymbolType && 
      nl->SymbolValue( nl->First( arg1 ) ) == "range" &&
      nl->IsAtom( nl->Second( arg1 ) ) && 
      nl->AtomType( nl->Second( arg1 ) ) == SymbolType &&
      nl->IsAtom( arg2 ) && 
      nl->AtomType( arg2 ) == SymbolType &&
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
int
RangeIsEmpty_r( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeNotEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeIntersects_rr( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeInside_rr( Word* args, Word& result, int message, Word& local, Supplier s )
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

int
RangeInside_ar( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeBefore_rr( Word* args, Word& result, int message, Word& local, Supplier s )
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

int
RangeBefore_ar( Word* args, Word& result, int message, Word& local, Supplier s )
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

int
RangeBefore_ra( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeIntersection_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Intersection( *((Range*)args[1].addr), (*(Range*)result.addr) );
  return (0);
}

/*
5.3.8 Value mapping functions of operator ~union~

*/
int
RangeUnion_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Union( *((Range*)args[1].addr), (*(Range*)result.addr) );
  return (0);
}

/*
5.3.9 Value mapping functions of operator ~minus~

*/
int
RangeMinus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Minus( *((Range*)args[1].addr), (*(Range*)result.addr) );
  return (0);
}

/*
5.3.10 Value mapping functions of operator ~min~

*/
int
RangeMinimum_r( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeMaximum_r( Word* args, Word& result, int message, Word& local, Supplier s )
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
int
RangeNoComponents_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)->Set( true, ((Range*)args[0].addr)->GetNoComponents() );
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
ValueMapping rangeisemptymap[] = { RangeIsEmpty_r };
ValueMapping rangeequalmap[] = { RangeEqual_rr };
ValueMapping rangenotequalmap[] = { RangeNotEqual_rr };
ValueMapping rangeintersectsmap[] = { RangeIntersects_rr };
ValueMapping rangeinsidemap[] = { RangeInside_rr, RangeInside_ar };
ValueMapping rangebeforemap[] = { RangeBefore_rr, RangeBefore_ar, RangeBefore_ra };
ValueMapping rangeintersectionmap[] = { RangeIntersection_rr };
ValueMapping rangeunionmap[] = { RangeUnion_rr };
ValueMapping rangeminusmap[] = { RangeMinus_rr };
ValueMapping rangeminmap[] = { RangeMinimum_r };
ValueMapping rangemaxmap[] = { RangeMaximum_r };
ValueMapping rangenocomponentsmap[] = { RangeNoComponents_r };

ModelMapping rangenomodelmap[] = { RangeNoModelMapping, RangeNoModelMapping,
                                   RangeNoModelMapping, RangeNoModelMapping,
                                   RangeNoModelMapping, RangeNoModelMapping };

const string RangeSpecIsEmpty  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                 "\"Example\" ) "
                             "( <text>(range x) -> bool</text--->"
                               "<text>isempty ( _ )</text--->"
                               "<text>Returns whether the range is empty or "
                               "not.</text--->"
                               "<text>query isempty ( range1 )</text--->"
                               ") )";

const string RangeSpecEqual  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                            "( <text>( (range x) (range x) ) -> bool</text--->"
                               "<text>_ = _</text--->"
                               "<text>Equal.</text--->"
                               "<text>query range1 = range2</text--->"
                              ") )";

const string RangeSpecNotEqual  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                  "\"Example\" ) "
                             "( <text>( (range x) (range x) ) -> bool</text--->"
                               "<text>_ # _</text--->"
                               "<text>Not equal.</text--->"
                               "<text>query range1 # range2</text--->"
                              ") )";

const string RangeSpecIntersects  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                    "\"Example\" ) "
                             "( <text>( (range x) (range x) ) -> bool</text--->"
                               "<text>_ intersects _</text--->"
                               "<text>Intersects.</text--->"
                               "<text>query range1 intersects range2</text--->"
                              ") )";

const string RangeSpecInside  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                             "( <text>( (range x) (range x) ) -> bool,"
                             "( x (range x) ) -> bool</text--->"
                               "<text>_ inside _</text--->"
                               "<text>Inside.</text--->"
                               "<text>query 5 inside range1</text--->"
                              ") )";

const string RangeSpecBefore  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                             "( <text>( (range x) (range x) ) -> bool, "
                             "( x (range x) ) -> bool, ( (range x) x ) -> "
                             "bool</text--->"
                               "<text>_ before _</text--->"
                               "<text>Before.</text--->"
                               "<text>query 5 before range1</text--->"
                              ") )";

const string RangeSpecIntersection  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                      "\"Example\" ) "
                         "( <text>( (range x) (range x) ) -> (range x)</text--->"
                               "<text>_ intersection _</text--->"
                               "<text>Intersection.</text--->"
                               "<text>query range1 intersection range2</text--->"
                              ") )";

const string RangeSpecUnion  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" ) "
                         "( <text>( (range x) (range x) ) -> (range x)</text--->"
                               "<text>_ union _</text--->"
                               "<text>Union.</text--->"
                               "<text>query range1 union range2</text--->"
                              ") )";

const string RangeSpecMinus  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" ) "
                          "( <text>( (range x) (range x) ) -> (range x)</text--->"
                               "<text>_ minus _</text--->"
                               "<text>Minus.</text--->"
                               "<text>query range1 minus range2</text--->"
                              ") )";

const string RangeSpecMinimum  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                 "\"Example\" ) "
                             "( <text>(range x) -> x</text--->"
                               "<text>minimum ( _ )</text--->"
                               "<text>Minimum.</text--->"
                               "<text>minimum ( range1 )</text--->"
                              ") )";

const string RangeSpecMaximum  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                 "\"Example\" ) "
                             "( <text>(range x) -> x</text--->"
                               "<text>maximum ( _ )</text--->"
                               "<text>Maximum.</text--->"
                               "<text>maximum ( range1 )</text--->"
                              ") )";

const string RangeSpecNoComponents  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                      "\"Example\" ) "
                             "( <text>(range x) -> int</text--->"
                               "<text>no_components ( _ )</text--->"
                               "<text>Number of components.</text--->"
                               "<text>no_components ( range1 )</text--->"
                              ") )";

Operator rangeisempty( "isempty", 
                       RangeSpecIsEmpty, 
                       1, 
                       rangeisemptymap, 
                       rangenomodelmap, 
                       Operator::SimpleSelect, 
                       RangeTypeMapBool1 );

Operator rangeequal( "=", 
                     RangeSpecEqual, 
                     1, 
                     rangeequalmap, 
                     rangenomodelmap, 
                     Operator::SimpleSelect, 
                     RangeRangeTypeMapBool );

Operator rangenotequal( "#", 
                        RangeSpecNotEqual, 
                        1, 
                        rangenotequalmap, 
                        rangenomodelmap, 
                        Operator::SimpleSelect, 
                        RangeRangeTypeMapBool );

Operator rangeintersects( "intersects", 
                          RangeSpecIntersects, 
                          1, 
                          rangeintersectsmap, 
                          rangenomodelmap, 
                          Operator::SimpleSelect, 
                          RangeRangeTypeMapBool );

Operator rangeinside( "inside", 
                      RangeSpecInside, 
                      2, 
                      rangeinsidemap, 
                      rangenomodelmap, 
                      RangeSelectPredicates, 
                      RangeBaseTypeMapBool1 );

Operator rangebefore( "before", 
                      RangeSpecBefore, 
                      3, 
                      rangebeforemap, 
                      rangenomodelmap, 
                      RangeSelectPredicates, 
                      RangeBaseTypeMapBool2 );

Operator rangeintersection( "intersection", 
                            RangeSpecIntersection, 
                            1, 
                            rangeintersectionmap, 
                            rangenomodelmap, 
                            Operator::SimpleSelect, 
                            RangeRangeTypeMapRange );

Operator rangeunion( "union", 
                     RangeSpecUnion, 
                     1, 
                     rangeunionmap, 
                     rangenomodelmap, 
                     Operator::SimpleSelect, 
                     RangeRangeTypeMapRange );

Operator rangeminus( "minus", 
                     RangeSpecMinus, 
                     1,
                     rangeminusmap, 
                     rangenomodelmap, 
                     Operator::SimpleSelect, 
                     RangeRangeTypeMapRange );

Operator rangemin( "min", 
                   RangeSpecMinimum, 
                   1, 
                   rangeminmap, 
                   rangenomodelmap, 
                   Operator::SimpleSelect, 
                   RangeTypeMapBase );

Operator rangemax( "max", 
                   RangeSpecMaximum, 
                   1, 
                   rangemaxmap, 
                   rangenomodelmap, 
                   Operator::SimpleSelect, 
                   RangeTypeMapBase );

Operator rangenocomponents( "no_components", 
                            RangeSpecNoComponents, 
                            1, 
                            rangenocomponentsmap, 
                            rangenomodelmap, 
                            Operator::SimpleSelect, 
                            RangeTypeMapInt );

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
    range.AssociateKind( "DATA" );

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

