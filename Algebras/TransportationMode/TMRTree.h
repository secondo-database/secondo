/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

{\Large \bf Anhang D: RTree-Template }

[1] Header-File of TMRTree 

2012, June Jianqiu 

[TOC]

0 Overview

This header file implements a disk-resident representation of a R-Tree.
Setting some parameters the R-Tree-behaviour of Guttman or the R[*]-Tree
of Kriegel et al. can be selected.

The R-Tree is implemented as a template to satisfy the usage with various
dimensions. The desired dimensions are passed as a parameter to the template.

1 Defines and Includes

*/

#ifndef __TMRTREE_H__
#define __TMRTREE_H__

#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#include <iostream>
#include <stack>
#include <limits>
#include <string.h>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "BTreeAlgebra.h"
#include "TemporalAlgebra.h"
#include "AlmostEqual.h"
#include "RTreeAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle
#define BBoxSet RectangleSet


/*
4 Class TMTreeNode
This is a node in the TMR-Tree.
we can not simply derive this tmrtreenode from rtreenode class because 
of the following two functions: read() and write(). we have to store the tm
value after count and leaf. 

*/
template<unsigned dim, class LeafInfo>
class TM_RTreeNode
{
  public:

    TM_RTreeNode( const bool leaf, const int min, const int max );
    TM_RTreeNode( const TM_RTreeNode<dim, LeafInfo>& n );
    ~TM_RTreeNode();

    static int SizeOfEmptyNode() {
//      return sizeof( bool ) + sizeof( int ); 
        return sizeof(bool) + sizeof(int) + sizeof(long);// plus tm
    };

    int Size() const;

    int EntryCount() const { return count; }

    int MaxEntries() const { return maxEntries; }

    int MinEntries() const { return minEntries; }

     bool IsLeaf() const { return leaf; }


     R_TreeEntry<dim>& operator[] ( int index ) const {
     assert( index >= 0 );
     assert(index <= maxEntries );
     assert(index < count);
     return *entries[ index ];
    }
/*
Returns entry given by index.

*/
    R_TreeLeafEntry<dim,LeafInfo>* GetLeafEntry(const int index) const;
    R_TreeInternalEntry<dim>* GetInternalEntry(const int index) const;

    BBox<dim> BoundingBox() const;
  BBox<dim> BoundingBox(int entryid) const
  {
    assert(entryid >= 0 && entryid <= maxEntries);
    return entries[entryid]->box;
  }
  
  void Clear()
    {
      for( int i = 0; i <= maxEntries; i++ )
      {
        if(entries[i]){
          delete entries[ i ];
          entries[ i ] = 0;
        }
      }
      count = 0;
      modified = true;
    }
/*
Clears all entries.

*/

 TM_RTreeNode<dim, LeafInfo>& operator = ( const TM_RTreeNode<dim, LeafInfo>&);
/*
Assignment operator between nodes.

*/

    bool Remove( int );
/*
Removes the given entry from the node. Returns true if successful
or false if underflow (The entry is deleted regardless).

*/

    bool Insert( const R_TreeEntry<dim>& e );
/*
Adds ~e~ to this node if possible. Returns ~true~ if successful,
i.e., if there is enough room in the node,  or ~false~ if the insertion
caused an overflow. In the latter case, the entry is inserted,
but the node should be split by whoever called the insert method.

*/

    void Split(TM_RTreeNode<dim, LeafInfo>& n1, 
               TM_RTreeNode<dim, LeafInfo>& n2 );
/*
Splits this node in two: ~n1~ and ~n2~, which should be empty nodes.

*/

    void UpdateBox( BBox<dim>& box, SmiRecordId pointer );
/*
Update entry corresponding to ~pointer~ to have bounding box ~box~.

*/

    void Read( SmiRecordFile *file, const SmiRecordId pointer );
    void Read( SmiRecord& record );
/*
Reads this node from an ~SmiRecordFile~ at position ~id~.

*/

    void Write( SmiRecordFile *file, const SmiRecordId pointer );
    void Write( SmiRecord& record );
/*
Writes this node to an ~SmiRecordFile~ at position ~id~

*/
    void SetInternal( int minEntries, int maxEntries )
    {
      assert( count == 0 );
      leaf = false;
      this->minEntries = minEntries;
      this->maxEntries = maxEntries;
      modified = true;
    }
/*
Converts a leaf node to an internal one. The node must be empty.

*/

//////////////////////////////////////////////////
    void SetModified(){modified = true;}
//////////////////////////////////////////////////////

    long GetTMValue(){return tm;}
    void SetTMValue(long m){tm = m; modified = true;}

  private:
    bool leaf;
/*
Flag that tells whether this is a leaf node

*/

    int minEntries;
/*
Min \# of entries per node.

*/

    int maxEntries;
/*
Max \# of entries per node.

*/

    int count;
/*
Number of entries in this node.

*/

    R_TreeEntry<dim>** entries;
/*
Array of entries.

*/

    bool modified;
/*
A flag that indicates when a node is modified. This avoids
writing unnecessarily.

*/

    void LinearPickSeeds( int& seed1, int& seed2 ) const;
/*
Implements the linear ~PickSeeds~ algorithm of Guttman.
Linear algorithm that selects 2 seed entries to use as anchors
for the node splitting algorithm. The entry numbers are returned
in ~seed1~ and ~seed2~.

*/

    void QuadraticPickSeeds( int& seed1, int& seed2 ) const;
/*
Implementation of the quadratic 'PickSeeds' Algorithm of Guttman.
Quadratic algorithm that selects 2 seed entries to use as anchors
for the node splitting algorithm. The entry numbers are returned
in ~seed1~ and ~seed2~.

*/

    int QuadraticPickNext( BBox<dim>& b1, BBox<dim>& b2 ) const;
/*
Returns the entry position that should be assigned next to one of the
two groups with bounding boxes ~b1~ and ~b2~, respectively.
(Algorithm ~PickNext~ of Guttman)

*/

    long tm; ///////////transportation mode
};


/*
The constructors for TMRtree node
minentries:  24 maxentries: 62

*/
template<unsigned dim, class LeafInfo>
TM_RTreeNode<dim, LeafInfo>::TM_RTreeNode( const bool leaf,
                                       const int min,
                                       const int max ) :
  leaf( leaf ),
  minEntries( min ),
  maxEntries( max ),
  count( 0 ),
  entries( new R_TreeEntry<dim>*[ max + 1 ] ),
  modified( true ), tm(-1)
{
  for( int i = 0; i <= maxEntries; i++ ){
    entries[ i ] = 0;
  }
//  cout<<MinEntries()<<" "<<MaxEntries()<<endl;
}

template<unsigned dim, class LeafInfo>
TM_RTreeNode<dim, LeafInfo>::
TM_RTreeNode(const TM_RTreeNode<dim, LeafInfo>& node):
  leaf( node.leaf ),
  minEntries( node.minEntries ),
  maxEntries( node.maxEntries ),
  count( node.count ),
  entries( new R_TreeEntry<dim>*[ node.maxEntries + 1 ] ),
  modified( true ), tm(node.tm)
{
  int i;
  for( i = 0; i < node.EntryCount(); i++ )
  {
    if( leaf )
      entries[ i ] =
          new R_TreeLeafEntry<dim, LeafInfo>( (R_TreeLeafEntry<dim,
                                               LeafInfo>&)*node.entries[ i ] );
    else
      entries[ i ] =
          new R_TreeInternalEntry<dim>( (
          R_TreeInternalEntry<dim>&)*node.entries[ i ] );
  }
  for( ; i <= node.maxEntries; i++ ){
    entries[ i ] = NULL;
  }
}


/*
4.2 The destructor

*/
template<unsigned dim, class LeafInfo>
TM_RTreeNode<dim, LeafInfo>::~TM_RTreeNode()
{
  for( int i = 0; i <= count; i++ ){
    delete entries[ i ];
  }
  delete []entries;
}

template<unsigned dim, class LeafInfo>
int TM_RTreeNode<dim, LeafInfo>::Size() const
{
  int size = SizeOfEmptyNode();

  if( leaf )
    size += R_TreeLeafEntry<dim, LeafInfo>::Size() * maxEntries;
  else
    size += R_TreeInternalEntry<dim>::Size() * maxEntries;

  return size;
}

template<unsigned dim, class LeafInfo>
TM_RTreeNode<dim, LeafInfo>& TM_RTreeNode<dim, LeafInfo>::operator=
    (const TM_RTreeNode<dim, LeafInfo>& node )
{

  // delete old entries
  for(int i=0;i<=count; i++){
     delete entries[i];
     entries[i] = 0;
  }

  leaf = node.leaf;
  count = node.count;
  modified = true;

  // if there are a change in size, resize the entries array
  if(maxEntries!=node.maxEntries){
     delete[] entries;
     entries = new R_TreeEntry<dim>*[node.maxEntries +1];
     for(int i=node.count; i<=node.maxEntries; i++){
        entries[i] = 0;
     }
  }

  for( int i = 0; i < node.count; i++ ) {
    if( leaf ){
      entries[ i ] = new R_TreeLeafEntry<dim, LeafInfo>
          ( (R_TreeLeafEntry<dim, LeafInfo>&)*node.entries[ i ] );
    } else {
      entries[ i ] = new R_TreeInternalEntry<dim>
          ( (R_TreeInternalEntry<dim>&)*node.entries[ i ] );
    }
  }
  
  tm = node.tm;
  return *this;
}

template<unsigned dim, class LeafInfo>
bool TM_RTreeNode<dim, LeafInfo>::Remove( int index )
{
  assert( index >= 0 && index < count );

  delete entries[ index ];
  entries[ index ] = entries[ count - 1 ];
  entries[ count - 1 ] = 0;
  count -= 1;

  modified = true;
  return (count >= minEntries);
}

/*
4.3 Method Insert

*/
template<unsigned dim, class LeafInfo>
bool TM_RTreeNode<dim, LeafInfo>::Insert( const R_TreeEntry<dim>& ent )
{
  assert( count <= maxEntries );

  if( leaf )
    entries[ count++ ] = new R_TreeLeafEntry<dim, LeafInfo>
        ( (R_TreeLeafEntry<dim, LeafInfo>&)ent );
  else
    entries[ count++ ] = new R_TreeInternalEntry<dim>
        ( (R_TreeInternalEntry<dim>&)ent );
  modified = true;

  return (count <= maxEntries);
}

/*
4.3 Method LinearPickSeeds

*/
template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::LinearPickSeeds(int& seed1,int& seed2 ) const
{
  assert( EntryCount() == MaxEntries() + 1 );
    // This should be called only if the node has an overflow

  double maxMinVal[ dim ];
  double minMaxVal[ dim ];
  double minVal[ dim ];
  double maxVal[ dim ];
  double sep[ dim ];
  double maxSep = -numeric_limits<double>::max();
  int maxMinNode[ dim ];
  int minMaxNode[ dim ];
  int bestD = -1;

  for( unsigned i = 0; i < dim; i++ )
  {
    maxMinVal[i] = -numeric_limits<double>::max();
    minMaxVal[i] = numeric_limits<double>::max();
    minVal[i] = numeric_limits<double>::max();
    maxVal[i] = -numeric_limits<double>::max();
    maxMinNode[i] = -1;
    minMaxNode[i] = -1;
  }

  for( int i = 0; i < EntryCount(); i++ )
  {
    for( unsigned d = 0; d < dim; d++ )
    {
      if( entries[ i ]->box.MinD( d ) > maxMinVal[ d ] )
      {
        maxMinVal[ d ] = entries[ i ]->box.MinD( d );
        maxMinNode[ d ] = i;
      }

      if( entries[ i ]->box.MinD( d ) < minVal[ d ] )
      minVal[ d ] = entries[ i ]->box.MinD( d );

      if( entries[ i ]->box.MaxD( d ) < minMaxVal[ d ] )
      {
        minMaxVal[ d ] = entries[ i ]->box.MaxD( d );
        minMaxNode[ d ] = i;
      }

      if( entries[ i ]->box.MaxD( d ) > maxVal[ d ] )
        maxVal[ d ] = entries[ i ]->box.MaxD( d );
    }
  }

  for( unsigned d = 0; d < dim; d++ )
  {
    assert( maxMinNode[ d ] != -1 && minMaxNode[ d ] != -1 );
    assert( maxVal[ d ] > minVal[ d ] );
    sep[ d ] = double( maxMinVal[ d ] - minMaxVal[ d ] )
               / (maxVal[ d ] - minVal[ d ]);
    if( sep[ d ] > maxSep )
    {
      bestD = d;
      maxSep = sep[ d ];
    }
  }

  assert( bestD != -1 );
  seed1 = maxMinNode[ bestD ];
  seed2 = minMaxNode[ bestD ];

  if( seed1 == seed2 )
  {
    if( seed2 == 0 )
      seed2++;
    else
      seed2--;
  }
}

/*
4.4 Method QuadraticPickSeeds

*/
template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::QuadraticPickSeeds( int& seed1,
                                                    int& seed2 ) const
{
  assert( EntryCount() == MaxEntries() + 1 );
    // This should be called only if the node has an overflow

  double bestWaste = -numeric_limits<double>::max();
  double *area = new double[ MaxEntries() + 1 ]; // Compute areas just once
  int i;

  for( i = 0; i < EntryCount(); i++ )
  {
    int j;

    area[ i ] = entries[ i ]->box.Area();

    for( j = 0; j < i; ++j )
    {
      double totalArea = entries[ i ]->box.Union( entries[ j ]->box ).Area();
      double waste = totalArea - area[ i ] - area[ j ];

      if( waste > bestWaste )
      {
        seed1 = i;
        seed2 = j;
        bestWaste = waste;
      }
    }
  }

  delete [] area;
}

/*
4.5 Method QuadraticPickNext

*/
template<unsigned dim, class LeafInfo>
int TM_RTreeNode<dim, LeafInfo>::QuadraticPickNext( BBox<dim>& b1,
                                                  BBox<dim>& b2 ) const
{
  double area1 = b1.Area();
  double area2 = b2.Area();
  double bestDiff = -1;
  int besti = -1;

  for( int i = 0; i < count; i++ )
  {
    double d1 = b1.Union( entries[ i ]->box ).Area() - area1;
    double d2 = b2.Union( entries[ i ]->box ).Area() - area2;
    double diff = fabs( d1 - d2 );

    assert( d1 >= 0 && d2 >= 0 );

    if( diff > bestDiff )
    {
      bestDiff = diff;
      besti = i;
    }
  }

  assert( besti >= 0 );

  return besti;
}


/*
4.6 Method Split

*/
template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::Split( TM_RTreeNode<dim,
                                       LeafInfo>& n1,
                                       TM_RTreeNode<dim,
                                       LeafInfo>& n2 )
// Splits this node in two: n1 and n2, which should be empty nodes.
{
  assert( EntryCount() == MaxEntries() + 1 );
    // Make sure this node is ready to be split

  assert( n1.EntryCount() == 0 && n2.EntryCount() == 0  );
  assert( n1.MinEntries() == MinEntries() && n1.MaxEntries() == MaxEntries() );
  assert( n2.MinEntries() == MinEntries() && n2.MaxEntries() == MaxEntries() );
    // Make sure n1 and n2 are ok

  if( do_axis_split )
  { // Do R[*]-Tree style split
    int *sortedEntry[ 2*dim ] = { NULL }; // Arrays of sorted entries
    struct StatStruct
    {
      double margin;
      double overlap;
      double area;
    } *stat = new StatStruct[ dim*dim*(MaxEntries() + 2 - 2*MinEntries()) ],
      *pstat = stat; // Array of distribution statistics
    double minMarginSum = numeric_limits<double>::max();
    int minMarginAxis = -1;

    for( unsigned d = 0; d < dim; d++ )
    { // Compute sorted lists.
      // Sort entries numbers by minimum value of axis 'd'.
      int* psort = sortedEntry[ 2*d ] = new int[ MaxEntries() + 1 ];
      SortedArray sort( MaxEntries() + 1 );
      int i;

      for( i = 0; i <= MaxEntries(); i++ )
        sort.push( i, entries[ i ]->box.MinD( d ) );

      for( i = 0; i <= MaxEntries(); i++ )
        *psort++ = sort.pop();

      assert( sort.empty() );

      // Sort entries numbers by maximum value of axis 'd'
      psort = sortedEntry[ 2*d + 1 ] = new int[ MaxEntries() + 1 ];
      for( i = 0; i <= MaxEntries(); i++ )
        sort.push( i, entries[ i ]->box.MaxD( d ) );

      for( i = 0; i <= MaxEntries(); i++ )
        *psort++ = sort.pop();

      assert( sort.empty() );
    }

    // Compute statistics for the various distributions
    for( unsigned d = 0; d < dim; d++ )
    { // Sum margins over all distributions correspondig to one axis
      double marginSum = 0.0;

      for( unsigned minMax = 0; minMax < 2; minMax++ )
      { // psort points to one of the sorted arrays of entries
        int* psort = sortedEntry[ 2*d + minMax ];
        // Start by computing the cummulative bounding boxes of the
        // 'MaxEntries()-MinEntries()+1' entries of each end of the scale
        BBox<dim> *b1 = new BBox<dim>[ MaxEntries() + 1 ];
        BBox<dim> *b2 = new BBox<dim>[ MaxEntries() + 1 ];
        int i, splitPoint;

        b1[ 0 ] = entries[ psort[ 0 ] ]->box;
        b2[ 0 ] = entries[ psort[ MaxEntries() ] ]->box;

        for( i = 1; i <= MaxEntries(); i++ )
        {
          b1[i] = b1[i-1].Union(entries[psort[i]]->box );
          b2[i] = b2[i-1].Union(entries[psort[MaxEntries()-i]]->box);
        }

        // Now compute the statistics for the
        // MaxEntries() - 2*MinEntries() + 2 distributions
        for( splitPoint = MinEntries() - 1;
             splitPoint <= MaxEntries() - MinEntries();
             splitPoint++ )
        {
          BBox<dim>& box1 = b1[ splitPoint ];
          BBox<dim>& box2 = b2[ MaxEntries() - splitPoint - 1 ];

          pstat->margin = box1.Perimeter() + box2.Perimeter();
          pstat->overlap = box1.Intersection( box2 ).Area();
          pstat->area = box1.Area() + box2.Area();
          marginSum += pstat->margin;
          pstat += 1;

          assert( pstat - stat <=
                  (int)(dim*dim*(MaxEntries() + 2 - 2*MinEntries())) );
        }

        delete [] b2;
        delete [] b1;
      }

      if( marginSum < minMarginSum )
      {
        minMarginSum = marginSum;
        minMarginAxis = d;
      }
    }

    assert( pstat - stat == 2*(int)dim*(MaxEntries() + 2 - 2*MinEntries()));

    // At this point we have in minMarginAxis the axis on which we will
    // split. Choose the distribution with  minimum overlap,
    // breaking ties by choosing the distribution with minimum Area
    {
      double minOverlap = numeric_limits<double>::max();
      double minArea = numeric_limits<double>::max();
      int minSplitPoint = -1;
      int *sort = 0;
      int d = minMarginAxis;

      pstat = &stat[ 2*d*(MaxEntries() + 2 - 2*MinEntries()) ];
      for( unsigned minMax = 0; minMax < 2; minMax++ )
      {
        int *psort = sortedEntry[ 2*d + minMax ];
        int splitPoint;

        for( splitPoint = MinEntries() - 1;
             splitPoint <= MaxEntries()-MinEntries();
             splitPoint++, pstat++ )
          if( pstat->overlap < minOverlap ||
              (pstat->overlap == minOverlap && pstat->area < minArea) )
          {
            minOverlap = pstat->overlap;
            minArea = pstat->area;
            minSplitPoint = splitPoint;
            sort = psort;
          }
      }

      assert( sort != 0 );
      assert( pstat - stat == (d + 1 )*2*(MaxEntries() + 2 - 2*MinEntries()) );

      // Picked distribution; now put the corresponding entries in the
      // two split blocks
      for( int i = 0; i <= minSplitPoint; i++ )
        n1.Insert( *entries[ sort[ i ] ] );

      for( int i = minSplitPoint + 1; i <= MaxEntries(); i++ )
        n2.Insert( *entries[ sort[ i ] ] );

      assert( AlmostEqual(
               n1.BoundingBox().Intersection( n2.BoundingBox() ).Area(),
               minOverlap ));

      // Deallocate the sortedEntry arrays
      for( unsigned i = 0; i < 2*dim; i++)
        delete [] sortedEntry[ i ];

      delete [] stat;
    }
  }
  else
  { // Do regular R-tree split
    int seed1, seed2; // Pick seeds

    if( do_quadratic_split )
      QuadraticPickSeeds( seed1, seed2 );
    else
    {
      assert( do_linear_split );

      LinearPickSeeds( seed1, seed2 );
    }

    // Put the two seeds in n1 and n2 and mark them
    BBox<dim> box1 = entries[ seed1 ]->box;
    BBox<dim> box2 = entries[ seed2 ]->box;
    n1.Insert( *entries[ seed1 ] );
    n2.Insert( *entries[ seed2 ] );

    // Make sure that we delete entries from end of the array first
    if( seed1 > seed2 )
    {
      Remove( seed1 );
      Remove( seed2 );
    }
    else
    {
      Remove( seed2 );
      Remove( seed1 );
    }

    { // Successively choose a not yet assigned entry and put it into n1 or n2
      int i = 0;
      int notAssigned = EntryCount();

      assert( notAssigned == MaxEntries() - 1 );
      while( notAssigned > 0 )
      {
        if( n1.EntryCount() + notAssigned == n1.MinEntries() )
        { // Insert all remaining entries in n1
          for( i = 0; i < EntryCount() ; i++, notAssigned-- ){
            n1.Insert( *entries[ i ] );
            delete entries[i];
            entries[i] = 0;
          }
          count = 0;
          assert( notAssigned == 0 );
        }
        else if( n2.EntryCount() + notAssigned == n2.MinEntries() )
        { // Insert all remaining entries in n2
          for( i = 0; i < EntryCount(); ++i, notAssigned-- ){
            n2.Insert( *entries[ i ] );
            delete entries[i];
            entries[i] = 0;
          }
          count = 0;
          assert( notAssigned == 0 );
        }
        else
        {
          BBox<dim> union1, union2;
          if( do_quadratic_split )
            i = QuadraticPickNext( box1, box2 );
          else
          {
            assert( do_linear_split );
            i = 0;
          }

          union1 = box1.Union( entries[ i ]->box );
          union2 = box2.Union( entries[ i ]->box );

          if( union1.Area() - box1.Area() < union2.Area() - box2.Area() )
          {
            n1.Insert( *entries[ i ] );
            box1 = union1;
          }
          else
          {
            n2.Insert( *entries[ i ] );
            box2 = union2;
          }

          Remove( i );
          notAssigned--;
        }
      }

      assert( notAssigned == 0 );
      assert( EntryCount() == 0 );
    }
  }

  assert( n1.EntryCount() + n2.EntryCount() == MaxEntries() + 1 );
  assert( n1.EntryCount() >= MinEntries() && n1.EntryCount() <= MaxEntries() );
  assert( n2.EntryCount() >= MinEntries() && n2.EntryCount() <= MaxEntries() );
  modified = true;
}

/*
4.7 Method BoundingBox

*/
template<unsigned dim, class LeafInfo>
BBox<dim> TM_RTreeNode<dim, LeafInfo>::BoundingBox() const
{
  if( count == 0 )
    return BBox<dim>( false );
  else
  {
    BBox<dim> result = entries[ 0 ]->box;
    int i;

    for( i = 1; i < count; i++ )
      result = result.Union( entries[ i ]->box );

    return result;
  }
}

/*
4.8 Method UpdateBox

*/
template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::UpdateBox( BBox<dim>& b, SmiRecordId pointer)
{
  assert( !leaf );
  modified = true;

  for( int i = 0; i < count; i++ )
    if( ((R_TreeInternalEntry<dim>*)entries[ i ])->pointer == pointer )
    {
      entries[ i ]->box = b;

      return;
    }

  // Should never reach this point
  assert( 0 );
}

template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::Read( SmiRecordFile *file,
                                      const SmiRecordId pointer )
{
  SmiRecord record;
  int RecordSelected = file->SelectRecord( pointer, record, SmiFile::ReadOnly );
  assert( RecordSelected );
  Read( record );
}

template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::Read( SmiRecord& record )
{
  int offset = 0;
  char buffer[Size() + 1];
  memset( buffer, 0, Size() + 1 );

  SmiSize readed = record.Read( buffer, Size(), offset );

  // Reads leaf, count
  memcpy( &leaf, buffer + offset, sizeof( leaf ) );
  offset += sizeof( leaf );
  memcpy( &count, buffer + offset, sizeof( count ) );
  offset += sizeof( count );

  ///////////////read tm ///////////////////
  memcpy( &tm, buffer + offset, sizeof( tm ) );
  offset += sizeof( tm );


  assert( count <= maxEntries );

  // Now read the entry array.
  for( int i = 0; i < count; i++ )
  {
    if( leaf )
      entries[ i ] = new R_TreeLeafEntry<dim, LeafInfo>();
    else
      entries[ i ] = new R_TreeInternalEntry<dim>();

    entries[ i ]->Read( buffer, offset );
  }

  assert(offset<=(int)readed); // otherwise some entries will be uninitialized
  modified = false;
  
  
}

template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::Write( SmiRecordFile *file,
                                       const SmiRecordId pointer )
{

  if( modified )
  {

    SmiRecord record;
    int RecordSelected = file->SelectRecord( pointer, record, SmiFile::Update );
    assert( RecordSelected );
    Write( record );
  }
}

template<unsigned dim, class LeafInfo>
void TM_RTreeNode<dim, LeafInfo>::Write( SmiRecord& record )
{
  if( modified )
  {
    int offset = 0;
    char buffer[Size() + 1];
    memset( buffer, 0, Size() + 1 );

    // Writes leaf, count
    memcpy( buffer + offset, &leaf, sizeof( leaf ) );
    offset += sizeof( leaf );
    memcpy( buffer + offset, &count, sizeof( count ) );
    offset += sizeof( count );

    //////////////////////write tm ////////////////////////
    memcpy( buffer + offset, &tm, sizeof( tm ) );
    offset += sizeof( tm );

    //cout << "TM_RTreeNode<dim, LeafInfo>::Write(): count/maxEntries = "
    //     << count << "/" << maxEntries << "." << endl;
    assert( count <= maxEntries );

    // Now write the entry array.
    for( int i = 0; i < count; i++ )
      entries[i]->Write( buffer, offset );

    int RecordWritten = record.Write( buffer, Size(), 0 );
    assert( RecordWritten );
    modified = false;
  }
}

template <unsigned dim, class LeafInfo>
R_TreeLeafEntry<dim,LeafInfo>* 
TM_RTreeNode<dim,LeafInfo>::GetLeafEntry(const int index) const
{
  assert(index >= 0 && index <= maxEntries);
  return (R_TreeLeafEntry<dim,LeafInfo>*)entries[index];
}

template <unsigned dim, class LeafInfo>
R_TreeInternalEntry<dim>*
     TM_RTreeNode<dim,LeafInfo>::GetInternalEntry(const int index) const
{
  assert(index >= 0 && index < count);
  return (R_TreeInternalEntry<dim>*)entries[index];
}


/*
Class tmbulkload
This class implements the TMR-Tree.

*/

template<unsigned dim, class LeafInfo>
class TM_BulkLoadInfo
{
  public:
    TM_RTreeNode<dim, LeafInfo> *node[MAX_PATH_SIZE];
    bool skipLeaf;
    int currentLevel;
    int currentHeight;
    int nodeCount;
    int entryCount;
    long levelEntryCounter[MAX_PATH_SIZE];
    double levelDistanceSum[MAX_PATH_SIZE];
    BBox<dim> levelLastBox[MAX_PATH_SIZE];

    TM_BulkLoadInfo(const bool &leafSkipping = false) :
      skipLeaf( leafSkipping ),
      currentLevel( 0 ),
      currentHeight( 0 ),
      nodeCount( 0 ),
      entryCount( 0 )
    {
      for(int i=0; i < MAX_PATH_SIZE; i++)
      {
        node[i] = NULL;
        levelEntryCounter[i] = 0;
        levelDistanceSum[i] = 0.0;
        levelLastBox[i] = BBox<dim>(false);
      }
    };

    virtual ~TM_BulkLoadInfo()
    {
      for(int i=0; i < MAX_PATH_SIZE; i++)
        if(node[i] != NULL)
          delete node[i];
    };
};

///////////////////////////////////////////////////////////////////////////
///////////////////// TMRtree implementation /////////////////////////////
////////////////////////////////////////////////////////////////////////////

template <unsigned dim, class LeafInfo>
class TM_RTree
{
  public:
/*
The first constructor. Creates an empty R-tree.

*/
    TM_RTree( const int pageSize );

/*
Opens an existing R-tree.

*/
    TM_RTree( SmiRecordFile *file );


    TM_RTree( const SmiFileId fileId );
    TM_RTree( SmiRecordFile *file,
            const SmiRecordId headerRecordId );
    TM_RTree( const SmiFileId fileId, bool update );
/////////////////////////////////////////////////////////////////////////////
    TM_RTree( const SmiFileId fileid,const int);
    void OpenFile(const SmiFileId fileid){file->Open(fileid);}
    void CloseFile(){file->Close();}

////////////////////////////////////////////////////////////////////////////
    void Clone(TM_RTree<dim,LeafInfo>*);
    SmiRecordId DFVisit_Rtree(TM_RTree<dim,LeafInfo>*,
                              TM_RTreeNode<dim,LeafInfo>*);


/*
The destructor.

*/
    ~TM_RTree();

/*
Open and Save are used by NetworkAlgebra to save and open the rtree of network.

*/
    bool  Open( SmiRecord& valueRecord,
                size_t& offset,
                string typeInfo,
                Word &value);

    bool Save(SmiRecord& valueRecord,
                size_t& offset);
    inline static const string BasicType(){///!!! "tm-rtree" does not work
        return "tmrtree";
    }

   static const bool checkType(ListExpr type){
     return nl->IsEqual(type, "tmrtree" );
   }  

/*
Deletes the file of the R-Tree.

*/
    inline void DeleteFile()
    {
      if(nodePtr){
        delete nodePtr;
        nodePtr=0;
      }
      file->Close();
      file->Drop();
    }

/*
Returns the ~SmiFileId~ of the R-Tree database file.

*/
    inline SmiFileId FileId()
    {
      return file->GetFileId();
    }

    inline void GetNode(SmiRecordId id, TM_RTreeNode<dim, LeafInfo>& result) {
        result.Read(file,id);
    }


/*
Returns the ~SmiRecordId~ of the root node.

*/
    inline SmiRecordId RootRecordId() const
    {
      return header.rootRecordId;
    }

/*
Returns the ~SmiRecordId~ of the header node.

*/
    inline SmiRecordId HeaderRecordId() const
    {
      return header.headerRecordId;
    }

    inline int MinEntries( int level ) const
    {
      return level == Height() ? header.minLeafEntries
                               : header.minInternalEntries;
    }

    inline int MinLeafEntries() const{
        return header.minLeafEntries;
    }
    inline int MinInternalEntries() const{
        return header.minInternalEntries;
    }

/*
Returns the minimum number of entries per node.

*/

    inline int MaxEntries( int level ) const
    {
      return level == Height() ? header.maxLeafEntries
                               : header.maxInternalEntries;
    }
    inline int MaxLeafEntries() const{
        return header.maxLeafEntries;
    }
    inline int MaxInternalEntries() const{
        return header.maxInternalEntries;
    }
/*
Returns the maximum number of entries per node.

*/

    inline int EntryCount() const
    {
      return header.entryCount;
    }
/*
Return the total number of (leaf) entries in this tree.

*/

    inline int NodeCount() const
    {
      return header.nodeCount;
    }
/*
Returns the total number of nodes in this tree.

*/

    inline int Height() const
    {
      return header.height;
    }
/*
Returns the height of this tree.

*/

    BBox<dim> BoundingBox();
/*
Returns the bounding box of this rtree.

*/

    void Insert( const R_TreeLeafEntry<dim, LeafInfo>& );
/*
Inserts the given entry somewhere in the tree.

*/

    bool Remove( const R_TreeLeafEntry<dim, LeafInfo>& );
/*
Deletes the given entry from the tree. Returns ~true~ if entry found
and successfully deleted, and ~false~ otherwise.

*/

    bool First( const BBox<dim>& bx,
                R_TreeLeafEntry<dim, LeafInfo>& result,
                int replevel = -1 );
    bool First( const BBoxSet<dim>& searchBoxSet,
                R_TreeLeafEntry<dim, LeafInfo>& result );
    bool Next( R_TreeLeafEntry<dim, LeafInfo>& result );
/*
Sets ~result~ to the (leaf) entry corresponding to the first/next
object whose bounding box overlaps ~bx~.
Returns ~true~ if a suitable entry was found and ~false~ if not.
Setting ~replevel~ to a value != -1 forces the search to return
entries at that level of the tree regardless of whether they
are at leaf nodes or not.

*/

    TM_RTreeNode<dim, LeafInfo>& Root();
/*
Loads ~nodePtr~ with the root node and returns it.

*/


    bool checkRecordId(SmiRecordId nodeId);

    bool InitializeBulkLoad(const bool &leafSkipping = BULKLOAD_LEAF_SKIPPING);

/*
Verifies, that the R-tree is empty. Use this before calling
~InsertBulkLoad()~.
Data structures used during bulk loading will be initialized.

*/


/*
build the r-tree in a way for genmo units

*/
    void TM_BulkLoad(const R_TreeEntry<dim>& entry, int , int);
    void BulkLoad(const R_TreeEntry<dim>& entry);
/*
Insert an entry in the bulk loading mode. The R-tree will be
constructed bottom up.

*/

    bool FinalizeBulkLoad();
/*
Call this after inserting the last entry in the bulk loading mode.
The root will be changed to point to the constructed R-tree.
Data structures used during bulk loading will be deleted.

*/

    bool IntrospectFirst(IntrospectResult<dim>& result);
    bool IntrospectNext(IntrospectResult<dim>& result);
  bool IntrospectNextE(unsigned long& nodeid, BBox<dim>& box,unsigned long&
                        tupleid);
  SmiRecordId SmiNodeId(int currlevel)
  {
    assert(currlevel >= 0 && currlevel <= Height());
    return path[currlevel];
  }
/*
The last two methods are used to produce a sequence of node decriptions, that
can be used to inspect the R-tree structure.

*/

    void FirstDistanceScan( const BBox<dim>& box );
/*
FirstDistanceScan initializes the priority queue.

Implemented by NearestNeighborAlgebra.

*/

    void LastDistanceScan(  );
/*
LastDistanceScan deletes the priority queue of the distancescan

Implemented by NearestNeighborAlgebra.

*/

    bool NextDistanceScan( const BBox<dim>& box, LeafInfo& result );
/*
NextDistanceScan returns true and fills the result with the
ID of the next tuple if there is a next tuple else it returns false

*/


   TM_RTreeNode<dim, LeafInfo> *GetMyNode(SmiRecordId& address,
                                        const bool leaf,
                                        const int min, const int max )
  {
    return GetNode(address,leaf,min,max);
  }

  bool getFileStats( SmiStatResultType &result );

  bool InitializeBLI(const bool& leafSkipping=BULKLOAD_LEAF_SKIPPING);

  bool CalculateTM(Relation* rel, int);
  long CalculateNodeTM(SmiRecordId nodeid, Relation* rel, int attr_pos);
///////////
  private:
    bool fileOwner;
    SmiRecordFile *file;
/*
The record file of the R-Tree.

*/
    struct Header
    {
      SmiRecordId headerRecordId; // Header node address.
      SmiRecordId rootRecordId;   // Root node address (Path[ 0 ]).
      int minLeafEntries;         // min # of entries per leaf node.
      int maxLeafEntries;         // max # of entries per leaf node.
      int minInternalEntries;     // min # of entries per internal node.
      int maxInternalEntries;     // max # of entries per internal node.
      int nodeCount;              // number of nodes in this tree.
      int entryCount;             // number of entries in this tree.
      int height;                 // height of the tree.

      Header() :
        headerRecordId( 0 ), rootRecordId( 0 ),
        minLeafEntries( 0 ), maxLeafEntries( 0 ),
        minInternalEntries( 0 ), maxInternalEntries( 0 ),
        nodeCount( 0 ), entryCount( 0 ), height( 0 )
        {}
      Header( SmiRecordId headerRecordId, SmiRecordId rootRecordId = 0,
              int minEntries = 0, int maxEntries = 0,
              int minInternalEntries = 0, int maxInternalEntries = 0,
              int nodeCount = 0, int entryCount = 0,
              int nodeSize = 0, int height = 0) :
        headerRecordId( headerRecordId ),
        rootRecordId( rootRecordId ),
        minLeafEntries( minLeafEntries ),
        maxLeafEntries( maxLeafEntries ),
        minInternalEntries( minInternalEntries ),
        maxInternalEntries( maxInternalEntries ),
        nodeCount( nodeCount ),
        entryCount( entryCount ),
        height( height )
        {}
    } header;

/*
The header of the R-Tree which will be written (read) to (from) the file.

*/

    SmiRecordId path[ MAX_PATH_SIZE ];
/*
Addresses of all nodes in the current path.

*/

    int pathEntry[ MAX_PATH_SIZE ];
/*
Indices of entries down the current path.

*/

    int overflowFlag[ MAX_PATH_SIZE ];
/*
Flags used in Insert which control the forced reinsertion process.

*/

    TM_RTreeNode<dim, LeafInfo> *nodePtr;
/*
The current node of the R-tree.

*/

    int currLevel;
/*
Current level (of ~nodePtr~).

*/

    int currEntry;
/*
Current entry within ~nodePtr~.

*/

    int reportLevel;
/*
Report level for first/next.

*/

    BBox<dim> searchBox;
    BBoxSet<dim> searchBoxSet;
/*
Bounding box for first/next.

*/

    enum SearchType { NoSearch, BoxSearch, BoxSetSearch };
    SearchType searchType;
/*
A flag that tells whether we're in the middle of a First/Next
scan of the tree.

*/

  void PutNode( const SmiRecordId address, TM_RTreeNode<dim, LeafInfo> **node);
    void PutNode( SmiRecord& record, TM_RTreeNode<dim, LeafInfo> **node );
/*
Writes the node ~node~ at file position ~address~.
Also deletes the node.

*/

    TM_RTreeNode<dim, LeafInfo> *GetNode( const SmiRecordId address,
                                        const bool leaf,
                                        const int min, const int max );
    TM_RTreeNode<dim, LeafInfo> *GetNode( SmiRecord& record, const bool leaf,
                                        const int min, const int max );
/*
Reads a node at file position ~address~. This function creates a new
node that must be deleted somewhere.

*/

    void WriteHeader();
/*
Writes the header of this tree.

*/

    void ReadHeader();
/*
Reads the header of this rtree.

*/

    void InsertEntry( const R_TreeEntry<dim>& );
/*
Inserts given entry in current node.

*/

    void LocateBestNode( const R_TreeEntry<dim>& ent, int level );
/*
Locates the "best" node of level ~level~ to insert ~ent~.

*/

    bool FindEntry( const R_TreeLeafEntry<dim, LeafInfo>& ent );
/*
Finds the given entry ~ent~ in the tree. If successful, ~true~ is
returned and ~currEntry~ and ~nodePtr~ are set to point to the
found entry. Otherwise, ~false~ is returned.

*/

    void UpdateBox();
/*
Updates "bottom-up" bounding boxes of nodes in path
(i.e., from leaf to root).

*/

    void GotoLevel( int level );
/*
Loads the node at the given ~level~ (must be in the current path).

*/

    void DownLevel( int entryno );
/*
Loads the child node of the current node given by ~entryno~.

*/

    void UpLevel();
/*
Loads the father node.

*/

/*
Private InsertBulkLoad method. Additionally gets the TreeNode to insert into.

*/
    void InsertBulkLoad(TM_RTreeNode<dim, LeafInfo> *node,
                        const R_TreeEntry<dim>& entry);

    void TM_InsertBulkLoad(TM_RTreeNode<dim, LeafInfo> *node,
                        const R_TreeEntry<dim>& entry, int, int);
    bool bulkMode;

/*
true, iff in bulk loading mode

*/

    TM_BulkLoadInfo<dim, LeafInfo> *bli;
/*
Info maintained during bulk loading

*/

    long nodeIdCounter;
/*
A counter used in ~Introspect~ routines

*/

    long nodeId[ MAX_PATH_SIZE ];
/*
An array to save the nodeIds of the path during the ~Introspect~ routines

*/


    bool distanceFlag;
/*
true, after a call of FirstDistanceScan
Used by NearestNeighborAlgebra.

*/
    unsigned long noentrynode;//noentryin currnode
};

/*
5.1 The constructors

*/

template <unsigned dim, class LeafInfo>
TM_RTree<dim, LeafInfo>::TM_RTree( const int pageSize ) :
  fileOwner( true ),
  file( new SmiRecordFile( true, pageSize ) ),
  header(),
  nodePtr( NULL ),
  currLevel( -1 ),
  currEntry( -1 ),
  reportLevel( -1 ),
  searchBox( false ),
  searchBoxSet(),
  searchType( NoSearch ),
  bulkMode( false ),
  bli( NULL ),
  nodeIdCounter( 0 )
{

  file->Create();

  // Calculating maxEntries e minEntries
  int nodeEmptySize = TM_RTreeNode<dim, LeafInfo>::SizeOfEmptyNode();
  int leafEntrySize = sizeof( R_TreeLeafEntry<dim, LeafInfo> ),
      internalEntrySize = sizeof( R_TreeInternalEntry<dim> );

  int maxLeaf = ( pageSize - nodeEmptySize ) / leafEntrySize,
      maxInternal = ( pageSize - nodeEmptySize ) / internalEntrySize;

  header.maxLeafEntries = maxLeaf;
  header.minLeafEntries = (int)(maxLeaf * 0.4);

  header.maxInternalEntries = maxInternal;
  header.minInternalEntries = (int)(maxInternal * 0.4);

  assert( header.maxLeafEntries >= 2*header.minLeafEntries &&
          header.minLeafEntries > 0 );
  assert( header.maxInternalEntries >= 2*header.minInternalEntries &&
          header.minInternalEntries > 0 );

  // initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
  {
    overflowFlag[ i ] = 0;
    nodeId[ i ] = 0;
  }

  nodePtr = new TM_RTreeNode<dim, LeafInfo>( true,
                                           MinEntries( 0 ),
                                           MaxEntries( 0 ) );

  // Creating a new page for the R-Tree header.
  SmiRecord headerRecord;
  int AppendedRecord = file->AppendRecord( header.headerRecordId,
                                           headerRecord );

  assert( AppendedRecord );
  assert( header.headerRecordId == 1 );

  // Creating the root node.
  SmiRecordId rootRecno;
  SmiRecord rootRecord;
  AppendedRecord = file->AppendRecord( rootRecno, rootRecord );
  assert( AppendedRecord );
  header.rootRecordId = path[ 0 ] = rootRecno;
  header.nodeCount = 1;
  nodePtr->Write( rootRecord );

  currLevel = 0;
}

template <unsigned dim, class LeafInfo>
TM_RTree<dim, LeafInfo>::TM_RTree( SmiRecordFile *file ) :
  fileOwner( false ),
  file( file ),
  header(),
  nodePtr( NULL ),
  currLevel( -1 ),
  currEntry( -1 ),
  reportLevel( -1 ),
  searchBox( false ),
  searchBoxSet(),
  searchType( NoSearch ),
  bulkMode( false ),
  bli( NULL ),
  nodeIdCounter( 0 )
{

  // Calculating maxEntries e minEntries
  int nodeEmptySize = TM_RTreeNode<dim, LeafInfo>::SizeOfEmptyNode();
  int leafEntrySize = sizeof( R_TreeLeafEntry<dim, LeafInfo> ),
      internalEntrySize = sizeof( R_TreeInternalEntry<dim> );

  int maxLeaf = ( file->GetRecordLength() - nodeEmptySize ) /
                leafEntrySize,
      maxInternal = ( file->GetRecordLength() - nodeEmptySize ) /
                    internalEntrySize;

  header.maxLeafEntries = maxLeaf;
  header.minLeafEntries = (int)(maxLeaf * 0.4);

  header.maxInternalEntries = maxInternal;
  header.minInternalEntries = (int)(maxInternal * 0.4);

  assert( header.maxLeafEntries >= 2*header.minLeafEntries &&
          header.minLeafEntries > 0 );
  assert( header.maxInternalEntries >= 2*header.minInternalEntries &&
          header.minInternalEntries > 0 );


  // initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
  {
    overflowFlag[ i ] = 0;
    nodeId[ i ] = 0;
  }

  nodePtr = new TM_RTreeNode<dim, LeafInfo>( true,
                                           MinEntries( 0 ),
                                           MaxEntries( 0 ) );

  // Creating a new page for the R-Tree header.
  SmiRecord headerRecord;
  int AppendedRecord =
    file->AppendRecord( header.headerRecordId, headerRecord );
  assert( AppendedRecord );


  // Creating the root node.
  SmiRecordId rootRecno;
  SmiRecord rootRecord;

  AppendedRecord = file->AppendRecord( rootRecno, rootRecord );
  assert( AppendedRecord );
  header.rootRecordId = path[ 0 ] = rootRecno;
  header.nodeCount = 1;

  nodePtr->Write( rootRecord );

  currLevel = 0;
}



template <unsigned dim, class LeafInfo>
TM_RTree<dim, LeafInfo>::TM_RTree( const SmiFileId fileid ) :
fileOwner( true ),
file( new SmiRecordFile( true ) ),
header( 1 ),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox( false ),
searchBoxSet(),
searchType( NoSearch ),
nodeIdCounter( 0 )
{

  file->Open( fileid );

  // initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
  {
    overflowFlag[ i ] = 0;
    nodeId[ i ] = 0;
  }

  ReadHeader();
  assert( header.maxLeafEntries >= 2*header.minLeafEntries &&
          header.minLeafEntries > 0 );
  assert( header.maxInternalEntries >= 2*header.minInternalEntries &&
          header.minInternalEntries > 0 );

  currLevel = 0;

  nodePtr = GetNode( RootRecordId(),
                     currLevel == Height(),
                     MinEntries( currLevel ),
                     MaxEntries( currLevel ) );
  path[ 0 ] = header.rootRecordId;
}

template <unsigned dim, class LeafInfo>
TM_RTree<dim, LeafInfo>::TM_RTree( SmiRecordFile *file,
                               const SmiRecordId headerRecordId ) :
fileOwner( false ),
file( file ),
header( headerRecordId ),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox( false ),
searchBoxSet(),
searchType( NoSearch ),
nodeIdCounter( 0 )
{
//  cout<<"222"<<endl;
  // initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
  {
    overflowFlag[ i ] = 0;
    nodeId[ i ] = 0;
  }

  ReadHeader();
  assert( header.maxLeafEntries >= 2*header.minLeafEntries &&
          header.minLeafEntries > 0 );
  assert( header.maxInternalEntries >= 2*header.minInternalEntries &&
          header.minInternalEntries > 0 );

  currLevel = 0;

  nodePtr = GetNode( RootRecordId(),
                     currLevel == Height(),
                     MinEntries( currLevel ),
                     MaxEntries( currLevel ) );
  path[ 0 ] = header.rootRecordId;
}

/*
Open an existing R-Tree file and create a new R-Tree on it

*/

template <unsigned dim, class LeafInfo>
TM_RTree<dim, LeafInfo>::TM_RTree( const SmiFileId fileid,const int pageSize ) :
fileOwner( true ),
file( new SmiRecordFile( true )  ),
header(),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox( false ),
searchBoxSet(),
searchType( NoSearch ),
bulkMode(false),
bli(NULL),
nodeIdCounter( 0 )
{
//  cout<<"333"<<endl;
  file->Open(fileid);
  // Calculating maxEntries e minEntries
  int nodeEmptySize = TM_RTreeNode<dim, LeafInfo>::SizeOfEmptyNode();
  int leafEntrySize = sizeof( R_TreeLeafEntry<dim, LeafInfo> ),
      internalEntrySize = sizeof( R_TreeInternalEntry<dim> );

  int maxLeaf = ( pageSize - nodeEmptySize ) / leafEntrySize,
      maxInternal = ( pageSize - nodeEmptySize ) / internalEntrySize;

  header.maxLeafEntries = maxLeaf;
  header.minLeafEntries = (int)(maxLeaf * 0.4);

  header.maxInternalEntries = maxInternal;
  header.minInternalEntries = (int)(maxInternal * 0.4);

  assert( header.maxLeafEntries >= 2*header.minLeafEntries &&
          header.minLeafEntries > 0 );
  assert( header.maxInternalEntries >= 2*header.minInternalEntries &&
          header.minInternalEntries > 0 );

  // initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
  {
    overflowFlag[ i ] = 0;
    nodeId[ i ] = 0;
  }

  nodePtr = new TM_RTreeNode<dim, LeafInfo>( true,
                                           MinEntries( 0 ),
                                           MaxEntries( 0 ) );

  // Creating a new page for the R-Tree header.
  SmiRecord headerRecord;
  int AppendedRecord = file->AppendRecord( header.headerRecordId,
                                           headerRecord );
  assert( AppendedRecord );

  // Creating the root node.
  SmiRecordId rootRecno;
  SmiRecord rootRecord;
  AppendedRecord = file->AppendRecord( rootRecno, rootRecord );
  assert( AppendedRecord );
  header.rootRecordId = path[ 0 ] = rootRecno;
  header.nodeCount = 1;
  nodePtr->Write( rootRecord );

  currLevel = 0;

}


template <unsigned dim, class LeafInfo>
TM_RTree<dim, LeafInfo>::TM_RTree( const SmiFileId fileid, bool update ) :
fileOwner( true ),
file( new SmiRecordFile( true ) ),
header(1),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox( false ),
searchBoxSet(),
searchType( NoSearch ),
bulkMode(false),
bli(NULL),
nodeIdCounter( 0 )
{
//  cout<<"444"<<endl;
  file->Open( fileid );
// initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
  {
    overflowFlag[ i ] = 0;
    nodeId[ i ] = 0;
  }

  ReadHeader();
  assert( header.maxLeafEntries >= 2*header.minLeafEntries &&
          header.minLeafEntries > 0 );
  assert( header.maxInternalEntries >= 2*header.minInternalEntries &&
          header.minInternalEntries > 0 );

  currLevel = 0;

  nodePtr = GetNode( RootRecordId(),
                     currLevel == Height(),
                     MinEntries( currLevel ),
                     MaxEntries( currLevel ) );
  path[ 0 ] = header.rootRecordId;

}

/*
5.2 The destructor

*/
template <unsigned dim, class LeafInfo>
TM_RTree<dim, LeafInfo>::~TM_RTree()
{
//  cout<<"~R_Tree()"<<endl;

  if( file->IsOpen() )
  {
    if( nodePtr != NULL )
      PutNode( path[ currLevel ], &nodePtr );

    WriteHeader();

    if( fileOwner )
      file->Close();
  }
  if( fileOwner )
    delete file;
}

/*
5.3 Reading and writing the header

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::ReadHeader()
{

  SmiRecord record;

  int RecordSelected =
    file->SelectRecord( header.headerRecordId,
                        record,
                        SmiFile::ReadOnly );
  assert( RecordSelected );

  int RecordRead = record.Read( &header, sizeof( Header ), 0 )
      == sizeof( Header );
  assert( RecordRead );
}

template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::WriteHeader()
{
  SmiRecord record;
  int RecordSelected =
      file->SelectRecord( header.headerRecordId,
                          record,
                          SmiFile::Update );
  assert( RecordSelected );
  int RecordWritten =
      record.Write( &header, sizeof( Header ), 0 ) == sizeof( Header );
  assert( RecordWritten );
}

/*
5.4 Method PutNode: Putting node to disk

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::PutNode( const SmiRecordId recno,
                                     TM_RTreeNode<dim, LeafInfo> **node )
{
  assert( file->IsOpen() );
  (*node)->Write( file, recno );
  delete *node;
  *node = NULL;
}

template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::PutNode( SmiRecord& record,
                                     TM_RTreeNode<dim, LeafInfo> **node )
{
  (*node)->Write( record );
  delete *node;
  *node = NULL;
}

/*
5.5 Method GetNode: Getting node from disk

*/
template <unsigned dim, class LeafInfo>
TM_RTreeNode<dim, LeafInfo> *TM_RTree<dim, LeafInfo>::GetNode(
    const SmiRecordId recno,
    const bool leaf,
    const int min,
    const int max )
{
  assert( file->IsOpen() );
  TM_RTreeNode<dim, LeafInfo> *node =
      new TM_RTreeNode<dim, LeafInfo>( leaf, min, max );
  node->Read( file, recno );
  return node;
}

template <unsigned dim, class LeafInfo>
TM_RTreeNode<dim, LeafInfo> *TM_RTree<dim, LeafInfo>::GetNode(
    SmiRecord& record,
    const bool leaf,
    const int min,
    const int max )
{
  TM_RTreeNode<dim, LeafInfo> *node =
      new TM_RTreeNode<dim, LeafInfo>( leaf, min, max );
  node->Read( record );
  return node;
}

/*
return BoundingBox

*/
template <unsigned dim, class LeafInfo>
BBox<dim> TM_RTree<dim, LeafInfo>::BoundingBox()
{
  if( currLevel == 0 )
    return nodePtr->BoundingBox();
  else
  {
    TM_RTreeNode<dim, LeafInfo> *tmp =
        GetNode( RootRecordId(),
                 0,
                 MinEntries(currLevel),
                 MaxEntries(currLevel) );
    BBox<dim> result = tmp->BoundingBox();
    delete tmp;

    return result;
  }
}

/*
Method Insert

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim,LeafInfo>::Insert(const R_TreeLeafEntry<dim,LeafInfo>& entry)
{
  searchType = NoSearch;

  LocateBestNode( entry, Height() );
  InsertEntry( entry );
  header.entryCount++;
}

/*
Method LocateBestNode

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::LocateBestNode( const R_TreeEntry<dim>& entry,
                                            int level )
{ GotoLevel( 0 );

  // Top down search for a node of level 'level'
  while( currLevel < level )
  {
    int bestNode = -1;
    if( currLevel + 1 == Height() && minimize_leafnode_overlap )
    { // Best node is the one that gives minimum overlap. However,
      // we should only take into consideration the k nodes that
      // result in least enlargement, where k is given by
      // leafnode_subset_max.
      SortedArray enlargeList( MaxEntries(currLevel) + 1 );
      int i, j, k;

      for( i = 0; i < nodePtr->EntryCount(); i++ )
      {
        R_TreeEntry<dim> &son = (*nodePtr)[ i ];
        enlargeList.push( i,
            son.box.Union( entry.box ).Area() - son.box.Area() );
      }

      if( enlargeList.headPri() == 0.0 )
        bestNode = enlargeList.pop();
        // ...No need to do the overlap enlargement tests
      else
      {
        double bestEnlargement = numeric_limits<double>::max(),
               bestoverlap = numeric_limits<double>::max();

        // Now compute the overlap enlargements
        for( k = 0; !enlargeList.empty() && k < leafnode_subset_max; k++ )
        {
          double enlargement = enlargeList.headPri(),
                 overlapBefore = 0.0,
                 overlapAfter = 0.0,
                 overlap;
          i = enlargeList.pop();
          BBox<dim> boxBefore = (*nodePtr)[ i ].box;
          BBox<dim> boxAfter = boxBefore.Union( entry.box );

          for( j = 0; j < nodePtr->EntryCount(); ++j )
            if( j == i )
              continue;
            else
            {
              R_TreeEntry<dim> &son = (*nodePtr)[ j ];
              overlapBefore += boxBefore.Intersection( son.box ).Area();
              overlapAfter += boxAfter.Intersection( son.box ).Area();
            }

          overlap = overlapAfter - overlapBefore;

          if( overlap < bestoverlap ||
              (overlap == bestoverlap && enlargement < bestEnlargement) )
          {
            bestoverlap = overlap;
            bestEnlargement = enlargement;
            bestNode = i;
          }
        }
      }
    }
    else
    {
      double bestEnlargement = numeric_limits<double>::max();
      int i;

      for( i = 0; i < nodePtr->EntryCount(); i++ )
      {
        R_TreeEntry<dim> &son = (*nodePtr)[ i ];
        double enlargement = son.box.Union( entry.box ).Area() - son.box.Area();

        if( enlargement < bestEnlargement )
        {
          bestNode = i;
          bestEnlargement = enlargement;
        }
      }
    }

    assert( bestNode != -1 );
    DownLevel( bestNode );
  }
}

/*
5.9 Method GotoLevel

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::GotoLevel( int level )
{
  if( currLevel == level )
  {
    if( nodePtr == NULL )
      nodePtr = GetNode( path[ currLevel ],
                         Height() == level,
                         MinEntries(currLevel),
                         MaxEntries(currLevel) );
  }
  else
  {
    assert( level >= 0 && level <= Height() );

    if( nodePtr != NULL )
      PutNode( path[ currLevel ], &nodePtr );

    currLevel = level;
    nodePtr = GetNode( path[ currLevel ],
                       Height() == level,
                       MinEntries(currLevel),
                       MaxEntries(currLevel) );
  }
}


/*
Method DownLevel

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::DownLevel( int entryNo )
{
  assert( currLevel != Height() );
  assert( nodePtr != 0 );
  assert( entryNo >= 0 && entryNo < nodePtr->EntryCount() );

  pathEntry[ currLevel ] = entryNo;
  path[ currLevel+1 ] =
      ((R_TreeInternalEntry<dim>&)(*nodePtr)[ entryNo ]).pointer;
  PutNode( path[ currLevel ], &nodePtr );
  currLevel += 1;
  nodePtr = GetNode( path[ currLevel ],
                     Height() == currLevel,
                     MinEntries(currLevel),
                     MaxEntries(currLevel) );
}


/*
InsertEntry

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::InsertEntry( const R_TreeEntry<dim>& entry )
{
  assert( file->IsOpen() );

  if( nodePtr->Insert( entry ) ){
    UpdateBox();
  } else {
    if( !do_forced_reinsertion || currLevel == 0 ||
        overflowFlag[ Height() - currLevel ] )
    { // Node splitting is necessary
      TM_RTreeNode<dim, LeafInfo> *n1 =
        new TM_RTreeNode<dim, LeafInfo>
            (nodePtr->IsLeaf(), MinEntries(currLevel), MaxEntries(currLevel) );
      TM_RTreeNode<dim, LeafInfo> *n2 =
        new TM_RTreeNode<dim, LeafInfo>
            (nodePtr->IsLeaf(), MinEntries(currLevel), MaxEntries(currLevel) );

      nodePtr->Split( *n1, *n2 );

      // Write split nodes and update parent
      if( currLevel == 0)
      { // splitting root node
        nodePtr->Clear();
        nodePtr->SetInternal( header.minInternalEntries,
                              header.maxInternalEntries );

        BBox<dim> n1Box( n1->BoundingBox() );
        SmiRecordId node1recno;
        SmiRecord node1record;
        int RecordAppended = file->AppendRecord( node1recno, node1record );
        assert( RecordAppended );
        PutNode( node1record, &n1 ); // deletes n1
        int EntryInserted =
            nodePtr->Insert( R_TreeInternalEntry<dim>(n1Box,node1recno));
        assert(EntryInserted);

        BBox<dim> n2Box( n2->BoundingBox() );
        SmiRecordId node2recno;
        SmiRecord node2record;
        RecordAppended = file->AppendRecord( node2recno, node2record );
        assert( RecordAppended );
        PutNode( node2record, &n2 ); // deletes n2
        EntryInserted =
            nodePtr->Insert(R_TreeInternalEntry<dim>(n2Box,node2recno));
        assert(EntryInserted);
        header.height += 1;
        header.nodeCount += 2;
      }
      else
      { // splitting non-root node
        SmiRecordId newNoderecno;
        SmiRecord newNoderecord;
        int RecordAppended = file->AppendRecord( newNoderecno, newNoderecord );
        assert( RecordAppended );
        R_TreeInternalEntry<dim> newEntry( n2->BoundingBox(), newNoderecno );
        PutNode( newNoderecord, &n2 ); // deletes n2

        header.nodeCount++;

        // Copy all entries from n1 to nodePtr
        nodePtr->Clear();
        *nodePtr = *n1;
        delete n1;

        UpdateBox();
        UpLevel();
        InsertEntry( newEntry );
      }
    }
    else
    { // Do forced reinsertion instead of split
      int reinsertLevel = Height() - currLevel;

      // Avoid reinserting at this level
      overflowFlag[ reinsertLevel ] = 1;

      // Compute the center of the node
      BBox<dim> nodeBox = nodePtr->BoundingBox();
      double nodeCenter[ dim ];

      for( unsigned i = 0; i < dim; i++ )
        nodeCenter[ i ] = (nodeBox.MinD( i ) + nodeBox.MaxD( i ))/2;

      // Make list sorted by distance from the center of each
      // entry bounding box to the center of the bounding box
      // of all entries.
      // NOTE: We use CHESSBOARD metric for the distance
      SortedArray distSort( MaxEntries(currLevel) + 1 );

      for( int i = 0; i < nodePtr->EntryCount(); i++ )
      {
        double entryDistance = 0.0;

        for( unsigned j = 0; j < dim; j++ )
        {
          double centerEntry = ((*nodePtr)[ i ].box.MinD( j ) +
                                (*nodePtr)[ i ].box.MaxD( j ))/2;
          double dist = fabs( centerEntry - nodeCenter[ j ] );

          if( dist > entryDistance )
            entryDistance = dist;
        }

        distSort.push( i, entryDistance );
      }

      { // Write node with the entries that will stay
        int maxEntries = MaxEntries(currLevel),
            minEntries = MinEntries(currLevel);

        R_TreeEntry<dim> **tmp = new R_TreeEntry<dim>*[ maxEntries + 1 ];
        int *keepFlag = new int[ maxEntries + 1 ];
        bool leaf = nodePtr->IsLeaf();
        int deleteCount, n = 0;

        for( int i = 0; i <= maxEntries; i++ )
        {
          keepFlag[ i ] = 0;
          tmp[i] = 0;
        }

        deleteCount = (forced_reinsertion_percent * maxEntries) / 100;

        assert( maxEntries - deleteCount >= minEntries );

        for( int i = maxEntries-deleteCount; i >= 0; i-- )
          keepFlag[ distSort.pop() ] = 1;

        for( int i = maxEntries; i >= 0; i-- )
          if( !keepFlag[ i ] )
          {
            if( leaf )
              tmp[ i ] = new R_TreeLeafEntry<dim, LeafInfo>
                  ( (R_TreeLeafEntry<dim, LeafInfo>&)(*nodePtr)[ i ] );
            else
              tmp[ i ] = new R_TreeInternalEntry<dim>
                  ( (R_TreeInternalEntry<dim>&)(*nodePtr)[ i ] );
            n++;
            nodePtr->Remove( i );
          }

        assert( n == deleteCount );
        UpdateBox();

        // Reinsert remaining entries( using "close reinsert" --
        // see R*tree paper, pg 327)
        while( !distSort.empty() )
        {
          int entryNo = distSort.pop();
          R_TreeEntry<dim> *reinsertEntry = tmp[ entryNo ];

          assert( !keepFlag[ entryNo ] );

          LocateBestNode( *reinsertEntry, Height() - reinsertLevel );
          InsertEntry( *reinsertEntry );
        }

        // Reset flag so that other insertion operations may cause the
        // forced reinsertion process to take place
        overflowFlag[ reinsertLevel ] = 0;

        for( int i = 0; i <= maxEntries; i++ )
          delete tmp[i];
        delete [] tmp;
        delete [] keepFlag;
      }
    }
  }
}

/*
5.12 Method UpLevel

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::UpLevel()
{
  assert( currLevel > 0 );

  if( nodePtr != NULL )
    PutNode( path[ currLevel ], &nodePtr );

  currLevel -= 1;
  nodePtr = GetNode( path[ currLevel ],
                     Height() == currLevel,
                     MinEntries(currLevel),
                     MaxEntries(currLevel) );
}


/*
5.13 Method UpdateBox

*/
template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::UpdateBox()
{
  // Save where we were before
  int formerlevel = currLevel;

  for( int l = currLevel; l > 0; l-- )
  {
    // Compute bounding box of child
    BBox<dim> box = nodePtr->BoundingBox();

    // Update 'father' node
    UpLevel();
    nodePtr->UpdateBox( box, path[ l ] );
  }

  // Return to where we were before
  GotoLevel( formerlevel );
}

/*
5.14 Method FindEntry

*/
template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::FindEntry(const R_TreeLeafEntry<dim, 
                                        LeafInfo>& entry )
{

  // First see if the current entry is the one that is being sought,
  // as is the case with many searches followed by Delete
  if( currLevel == Height() &&
      currEntry >= 0 && currEntry < nodePtr->EntryCount() &&
      nodePtr != 0 &&
      (*nodePtr)[ currEntry ].box == entry.box &&
      ((R_TreeLeafEntry<dim, LeafInfo>&)(*nodePtr)[ currEntry ]).info
      == entry.info )
    return true;

  // Load root node
  GotoLevel( 0 );

  // Init search params
  searchBox = entry.box;
  currEntry = 0;

  // Search the tree until no more subtrees are found in the root
  while( currEntry < nodePtr->EntryCount() || currLevel > 0 )
  {
    if( currEntry >= nodePtr->EntryCount())
    { // All entries or subtrees of the current node were
      // examined. Go up on the tree
      UpLevel();
      currEntry = pathEntry[ currLevel ];
      currEntry++;
    }
    else // Try another subtree or entry
      if( currLevel == Height() ) // This is a leaf node. Search all entries
        while( currEntry < nodePtr->EntryCount() )
        {
          if( (*nodePtr)[ currEntry ].box == entry.box &&
              ((R_TreeLeafEntry<dim, LeafInfo>&)(*nodePtr)[ currEntry ]).
                info == entry.info )
            return true; // Found it

          currEntry++;
        }
      else // This is an internal node. Search all subtrees
        while( currEntry < nodePtr->EntryCount() )
          if( (*nodePtr)[ currEntry ].box.Intersects( searchBox ) )
          { // Found a possible subtree. Go down
            DownLevel( currEntry );
            currEntry = 0;

            break;
          }
        else
          currEntry++;
          // No subtree was found. Go up
  }

  // Entry Not found
  return false;
}

/*
5.15 Method First

*/
template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::First( const BBox<dim>& box,
                                   R_TreeLeafEntry<dim,
                                   LeafInfo>& result,
                                   int replevel )
{
  // Remember that we have started a scan of the R_Tree
  searchType = BoxSearch;

  // Init search params
  searchBox = box;
  reportLevel = replevel;

  // Load root node
  GotoLevel( 0 );

  // Finish with the actual search
  currEntry = -1;
  return Next( result );
}

template<unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::First( const BBoxSet<dim>& boxSet,
                                   R_TreeLeafEntry<dim, LeafInfo>& result )
{
  // Remember that we have started a scan of the R_Tree
  searchType = BoxSetSearch;

  // Init search params
  searchBoxSet = boxSet;

  // Load root node
  GotoLevel( 0 );

  // Finish with the actual search
  currEntry = -1;
  return Next( result );
}

/*
5.16 Method Next

*/
template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::Next( R_TreeLeafEntry<dim, LeafInfo>& result )
{
  // Next can be called only after a 'First' or a 'Next' operation
  assert( searchType != NoSearch );

  bool retcode = false;

  while( currEntry < nodePtr->EntryCount() || currLevel > 0 )
    if( currEntry >= nodePtr->EntryCount())
    { // All entries in this node were examined. Go up the tree
      // Find entry in father node corresponding to this node.
      UpLevel();
      assert( pathEntry[ currLevel ] < nodePtr->EntryCount() );
      currEntry = pathEntry[ currLevel ];
    }
    else
    { // Search next entry / subtree in this node
      currEntry++;

      if( currEntry < nodePtr->EntryCount() )
      {
        if( searchType == BoxSearch ?
              searchBox.Intersects( (*nodePtr)[ currEntry ].box ) :
              searchBoxSet.Intersects( (*nodePtr)[ currEntry ].box ) )
        {
          if( nodePtr->IsLeaf() || currLevel == reportLevel)
          { // Found an appropriate entry
            result = (R_TreeLeafEntry<dim, LeafInfo>&)(*nodePtr)[ currEntry ];
            retcode = true;
            break;
          }
          else
          { // Go down the tree
            DownLevel( currEntry );
            currEntry = -1;
          }
        }
      }
    }

  return retcode;
}



/*
Variants of ~First~ and ~Next~ for introspection into the R-tree.

The complete r-tree will be iterated, but only all entries on replevel
will be returned, unless replevel == -1.

*/

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::IntrospectFirst( IntrospectResult<dim>& result)
{
  // create result for root
  result = IntrospectResult<dim>
    (
      0,
      0,
      BoundingBox(),
      -1,
      nodePtr->IsLeaf(),
      nodePtr->MinEntries(),
      nodePtr->MaxEntries(),
      nodePtr->EntryCount()
    );

  // Load root node
  GotoLevel( 0 );
  nodeIdCounter = 0;
  nodeId[currLevel] = 0;


  // Remember that we have started a scan of the R_Tree
  searchType = BoxSearch;
  currEntry = -1;
  for(int i = 0; i < MAX_PATH_SIZE; i++)
  {
    pathEntry[i] = -1;
    nodeId[i]    = -1;
  }
  return true;
}

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::IntrospectNextE(unsigned long& nodeid,BBox<dim>&
                                                  box,unsigned long& tupleid  )
{
  // Next can be called only after a 'IntrospectFirst'
  // or a 'IntrospectNext' operation
  assert( searchType == BoxSearch );

  bool retcode = false;

  pathEntry[currLevel]++;
//  printf("pathEntry %d\n",pathEntry[currLevel]);
//  printf("EntryCount() %d\n",nodePtr->EntryCount());
//  printf("currLevel %d\n",currLevel);
  while( pathEntry[currLevel] < nodePtr->EntryCount() || currLevel > 0 )
  {
    if( pathEntry[currLevel] >= nodePtr->EntryCount())
    { // All entries in this node were examined. Go up the tree
        // Find entry in father node corresponding to this node.
      nodeId[currLevel] = -1;
      pathEntry[ currLevel ] = -1;
      UpLevel();
      assert( pathEntry[ currLevel ] < nodePtr->EntryCount() );
      pathEntry[currLevel]++;
    }
    else
    { // Search next entry / subtree in this node
//      pathEntry[currLevel]++;

      if( nodeId[ currLevel ] < 0)
        nodeId[ currLevel ] = nodeIdCounter;
      if( pathEntry[currLevel] < nodePtr->EntryCount() )
      { // produce result
          if( nodePtr->IsLeaf() )
          { // Found leaf node
            // get complete node
//      printf("leaf\n");
            R_TreeLeafEntry<dim, LeafInfo> entry =
                (R_TreeLeafEntry<dim, LeafInfo>&)
                    (*nodePtr)[ pathEntry[ currLevel ] ];
//
            box = entry.box;
      tupleid = entry.info;
      nodeid = path[currLevel];
      retcode = true;
            break;
         }
          else // internal node
          { // Found internal node
//      printf("internal node\n");
            R_TreeInternalEntry<dim> entry =
                (R_TreeInternalEntry<dim>&) (*nodePtr)[pathEntry[ currLevel]];
//      printf("pathEntry currLevel %d, %d\n",currLevel,pathEntry[currLevel]);
      DownLevel( pathEntry[currLevel] );
            nodeId[currLevel] = ++nodeIdCounter; // set nodeId
//      printf("pathEntry currLevel %d, %d\n",currLevel,pathEntry[currLevel]);
      pathEntry[currLevel]++;
//             pathEntry[currLevel] = -1; // reset for next iteration
          } // end else

        } //end if
    } // end else
  } // end while
//  pathEntry[currLevel] += nodePtr->EntryCount();
  return retcode;
}


template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::IntrospectNext( IntrospectResult<dim>& result )
{
  // Next can be called only after a 'IntrospectFirst'
  // or a 'IntrospectNext' operation
  assert( searchType == BoxSearch );

  bool retcode = false;

  pathEntry[currLevel]++;
  while( pathEntry[currLevel] < nodePtr->EntryCount() || currLevel > 0 )
  {
    if( pathEntry[currLevel] >= nodePtr->EntryCount())
    { // All entries in this node were examined. Go up the tree
        // Find entry in father node corresponding to this node.
      nodeId[currLevel] = -1;
      pathEntry[ currLevel ] = -1;
      UpLevel();
      assert( pathEntry[ currLevel ] < nodePtr->EntryCount() );
      pathEntry[currLevel]++;
    }
    else
    { // Search next entry / subtree in this node
//      pathEntry[currLevel]++;
      if( nodeId[ currLevel ] < 0)
        nodeId[ currLevel ] = nodeIdCounter;
      if( pathEntry[currLevel] < nodePtr->EntryCount() )
      { // produce result
          if( nodePtr->IsLeaf() )
          { // Found leaf node
            // get complete node
            R_TreeLeafEntry<dim, LeafInfo> entry =
                (R_TreeLeafEntry<dim, LeafInfo>&)
                    (*nodePtr)[ pathEntry[ currLevel ] ];
            result = IntrospectResult<dim>
                (
                  currLevel+1,       // the entries shall have bigger levels
                  ++nodeIdCounter,   // and get their own node numbers
                  entry.box,
                  nodeId[currLevel],
                  true,              // use some
                  1,                 // resonable standard
                  1,                 // values for
                  1                  // these attributes
                );
          }
          else // internal node
          { // Found internal node
            DownLevel( pathEntry[currLevel] );
            nodeId[currLevel] = ++nodeIdCounter; // set nodeId
            R_TreeInternalEntry<dim> entry =
                (R_TreeInternalEntry<dim>&) (*nodePtr)[ 0 ];
            result = IntrospectResult<dim>
                (
                  currLevel,
                  nodeIdCounter,
                  nodePtr->BoundingBox(),
                  nodeId[currLevel-1], // currLevel >= 1 (DownLevel)
                  nodePtr->IsLeaf(),
                  nodePtr->MinEntries(),
                  nodePtr->MaxEntries(),
                  nodePtr->EntryCount()
                );
//             pathEntry[currLevel] = -1; // reset for next iteration
          } // end else
          retcode = true;
          break;
        } //end if
    } // end else
  } // end while
  return retcode;
}



/*
Method Remove

*/
template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::Remove( const R_TreeLeafEntry<dim,
                                    LeafInfo>& entry )
{
  assert( file->IsOpen() );

  searchType = NoSearch;

  // First locate the entry in the tree
  if( !FindEntry( entry ) )
    return false;
  else
  { // Create a list of nodes whose entries must be reinserted
    stack<int> reinsertLevelList;
    stack<TM_RTreeNode<dim, LeafInfo>*> reinsertNodeList;
    BBox<dim> sonBox( false );

    // remove leaf node entry
    nodePtr->Remove( currEntry );
    header.entryCount -= 1;

    while( currLevel > 0 )
    {
      int underflow = nodePtr->EntryCount() < MinEntries( currEntry );

      if( underflow )
      { // Current node has underflow. Save it for later reinsertion
     TM_RTreeNode<dim, LeafInfo>* nodePtrcopy = new TM_RTreeNode<dim, LeafInfo>
            ( *nodePtr );

        reinsertNodeList.push( nodePtrcopy );
        reinsertLevelList.push( currLevel );

        // Remove node from the tree
        nodePtr->Clear();
        int RecordDeleted = file->DeleteRecord( path[ currLevel ] );
        assert( RecordDeleted );
      }
      else
        sonBox = nodePtr->BoundingBox();

      // Find entry corresponding to this node in father node
      UpLevel();
      currEntry = pathEntry[ currLevel ];

      // Adjust father node
      if( underflow ) // remove corresponding entry in father node
        nodePtr->Remove( currEntry );
      else // Adjust father node entry bounding box
        (*nodePtr)[ currEntry ].box = sonBox;
    }

    // Reinsert entries in every node of reinsertNodeList
    while( !reinsertNodeList.empty() )
    {
      TM_RTreeNode<dim, LeafInfo>* tmp = reinsertNodeList.top();
      int level = reinsertLevelList.top(), i;
      reinsertNodeList.pop();
      reinsertLevelList.pop();

      for( i = 0; i < tmp->EntryCount(); i++ )
      {
        assert( level <= Height() );

        LocateBestNode( (*tmp)[ i ], level );
        InsertEntry( (*tmp)[ i ] );
      }

      delete tmp;
    }

    // See if root node now has only one son
    if( Height() == 0 ) // we are done
      return true;

    // Load root node
    GotoLevel( 0 );

    assert( !nodePtr->IsLeaf());

    if( nodePtr->EntryCount() == 1 )
    { // root node has only one son
      long newRoot = ((R_TreeInternalEntry<dim>&)(*nodePtr)[ 0 ]).pointer;

      // Remove former root node from the tree
      file->DeleteRecord( RootRecordId() );
      delete nodePtr; nodePtr = NULL;

      // Retrieve new root
      header.rootRecordId = path[ 0 ] = newRoot;
      GotoLevel( 0 );
      // Tree has diminished in height()
      header.height -= 1;
    }

    return true;
  }
}

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::InitializeBulkLoad(const bool &leafSkipping)
{
  assert( NodeCount() == 1 );

  if( bulkMode || bli != NULL )
  {
    return false;
  }
  bulkMode = true;
  bli = new TM_BulkLoadInfo<dim, LeafInfo>(leafSkipping);
  return true;
};

template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::TM_BulkLoad(const R_TreeEntry<dim>& entry, int m,
                                        int last_m)
{
  if( bli->node[0] != NULL ) {
    assert(bulkMode == true);
  }
  bli->currentLevel = 0;
  if( bli->node[0] == NULL ) { // create a fresh leaf node
    bli->node[0] = new TM_RTreeNode<dim,LeafInfo>(true,
                                                header.minLeafEntries,
                                                header.maxLeafEntries);
    bli->nodeCount++;
  }

  TM_InsertBulkLoad(bli->node[0], entry, m, last_m);
  bli->entryCount++;
  return;
}

template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::BulkLoad(const R_TreeEntry<dim>& entry)
{
  if( bli->node[0] != NULL ) {
    assert(bulkMode == true);
  }
  bli->currentLevel = 0;
  if( bli->node[0] == NULL ) { // create a fresh leaf node
    bli->node[0] = new TM_RTreeNode<dim,LeafInfo>(true,
                                                header.minLeafEntries,
                                                header.maxLeafEntries);
    bli->nodeCount++;
  }

  InsertBulkLoad(bli->node[0], entry);
  bli->entryCount++;
  return;
}

template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::InsertBulkLoad(TM_RTreeNode<dim, LeafInfo> *node,
                                           const R_TreeEntry<dim>& entry)
{
  assert( bulkMode == true );
  assert( node != NULL );

  if( !bli->levelLastBox[bli->currentLevel].IsDefined() ) {
    // initialize when called for the first time
    bli->levelLastBox[bli->currentLevel] = entry.box;
  }
  // trace the distance to the last entry inserted into this node:
  double dist = entry.box.Distance(bli->levelLastBox[bli->currentLevel]);
  bli->levelEntryCounter[bli->currentLevel]++;
  bli->levelDistanceSum[bli->currentLevel] += dist;
  bli->levelLastBox[bli->currentLevel] = entry.box;
  // update average distance of all entries on this level
  double avgDist =   bli->levelDistanceSum[bli->currentLevel]
                   / (MAX(bli->levelEntryCounter[bli->currentLevel],2) - 1);

//   cout << "Level = " << bli->currentLevel << endl
//       << "  dist = " << dist
//       << "  avgDist = " << avgDist
//       << "  #Entries =" << bli->node[bli->currentLevel]->EntryCount()
//       << "/" << bli->node[bli->currentLevel]->MaxEntries()
//       << endl;

  if(  ( bli->node[bli->currentLevel]->EntryCount() <
         bli->node[bli->currentLevel]->MaxEntries()         // fits into node
       )
       &&
       (    !bli->skipLeaf                                  // standard case
         || (   dist <= (avgDist * BULKLOAD_TOLERANCE) )    // distance OK
         || (   bli->node[bli->currentLevel]->EntryCount() <=
                bli->node[bli->currentLevel]->MinEntries() *
                              BULKLOAD_MIN_ENTRIES_FACTOR ) // too few entries
         || ( bli->levelEntryCounter[bli->currentLevel] <=
                               BULKLOAD_INITIAL_SEQ_LENGTH)
       )
    )
  { // insert the entry into this (already existing) node
//     cout << "  --> Inserting here!" << endl;
    bli->node[bli->currentLevel]->Insert(entry);
    return;
  } // else: node is already full (or distance to large)...

//   cout << "  --> Passing upwards..." << endl;
  //  Write node[currentLevel] to disk
  // Request SMI for a fresh record (and its Id):
  assert(file->IsOpen());
  SmiRecordId recId;
  SmiRecord rec;
  bool RecordAppended = file->AppendRecord(recId, rec);
  assert(RecordAppended);
  // write the current node into the fresh record
  bli->node[bli->currentLevel]->Write(file, recId);

  // if no father node exists, create one
  if( bli->node[bli->currentLevel+1] == NULL ) {
    assert (bli->currentHeight == bli->currentLevel);
    bli->currentHeight++; // increase height reached
    bli->node[bli->currentLevel+1] =
        new TM_RTreeNode<dim,LeafInfo>(false,
                                     header.minInternalEntries,
                                     header.maxInternalEntries);
    bli->nodeCount++;
  }

  // change to father
  bli->currentLevel++;

  // Insert son into father (by recursive call)
  InsertBulkLoad(
        bli->node[bli->currentLevel],
        R_TreeInternalEntry<dim>(
            bli->node[bli->currentLevel-1]->BoundingBox(),
            recId ) );

  // delete node and create a new node instead
  bli->currentLevel--; // change back to original level
  delete bli->node[bli->currentLevel];
  bli->node[bli->currentLevel] = NULL;
  if(bli->currentLevel == 0) { // create a leaf node
    bli->node[bli->currentLevel] =
        new TM_RTreeNode<dim,LeafInfo>(true,
                                     header.minLeafEntries,
                                     header.maxLeafEntries);
  } else { // create an internal node
    bli->node[bli->currentLevel] =
        new TM_RTreeNode<dim,LeafInfo>(false,
                                     header.minInternalEntries,
                                     header.maxInternalEntries);
  }

  bli->nodeCount++;

  // finally, insert the original entry passed as argument
  bli->node[bli->currentLevel]->Insert(entry);
  return;
};

/*
for each leaf node, only insert tuples with the same transportation mode

*/

template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::TM_InsertBulkLoad(
                                          TM_RTreeNode<dim, LeafInfo> *node,
                                           const R_TreeEntry<dim>& entry,
                                              int m, int last_m)
{
  assert( bulkMode == true );
  assert( node != NULL );

  if( !bli->levelLastBox[bli->currentLevel].IsDefined() ) {
    // initialize when called for the first time
    bli->levelLastBox[bli->currentLevel] = entry.box;
  }
  // trace the distance to the last entry inserted into this node:
  double dist = entry.box.Distance(bli->levelLastBox[bli->currentLevel]);
  bli->levelEntryCounter[bli->currentLevel]++;
  bli->levelDistanceSum[bli->currentLevel] += dist;
  bli->levelLastBox[bli->currentLevel] = entry.box;

  if( bli->node[bli->currentLevel]->EntryCount() <
      bli->node[bli->currentLevel]->MaxEntries() && m == last_m)

  { // insert the entry into this (already existing) node
//     cout << "  --> Inserting here!" << endl;
    bli->node[bli->currentLevel]->Insert(entry);
    return;
  } // else: node is already full (or distance to large)...

//   cout << "  --> Passing upwards..." << endl;
  //  Write node[currentLevel] to disk
  // Request SMI for a fresh record (and its Id):


  assert(file->IsOpen());
  SmiRecordId recId;
  SmiRecord rec;
  bool RecordAppended = file->AppendRecord(recId, rec);
  assert(RecordAppended);
  // write the current node into the fresh record
  bli->node[bli->currentLevel]->Write(file, recId);

  // if no father node exists, create one
  if( bli->node[bli->currentLevel+1] == NULL ) {
    assert (bli->currentHeight == bli->currentLevel);
    bli->currentHeight++; // increase height reached
    bli->node[bli->currentLevel+1] =
        new TM_RTreeNode<dim,LeafInfo>(false,
                                     header.minInternalEntries,
                                     header.maxInternalEntries);
    bli->nodeCount++;
  }

  // change to father
  bli->currentLevel++;

  // Insert son into father (by recursive call)
  InsertBulkLoad(
        bli->node[bli->currentLevel],
        R_TreeInternalEntry<dim>(
            bli->node[bli->currentLevel-1]->BoundingBox(),
            recId ) );

  // delete node and create a new node instead
  bli->currentLevel--; // change back to original level
  delete bli->node[bli->currentLevel];
  bli->node[bli->currentLevel] = NULL;
  if(bli->currentLevel == 0) { // create a leaf node
    bli->node[bli->currentLevel] =
        new TM_RTreeNode<dim,LeafInfo>(true,
                                     header.minLeafEntries,
                                     header.maxLeafEntries);
  } else { // create an internal node
    bli->node[bli->currentLevel] =
        new TM_RTreeNode<dim,LeafInfo>(false,
                                     header.minInternalEntries,
                                     header.maxInternalEntries);
  }
  bli->nodeCount++;

  // finally, insert the original entry passed as argument
  bli->node[bli->currentLevel]->Insert(entry);
  return;
};

template <unsigned dim, class LeafInfo>
SmiRecordId TM_RTree<dim, LeafInfo>::DFVisit_Rtree(
                                        TM_RTree<dim,LeafInfo>* rtree_in,
                                        TM_RTreeNode<dim,LeafInfo>* node)
{
  if(node->IsLeaf()){
    SmiRecordId recordid;
    SmiRecord record;
    int AppendRecord =
      file->AppendRecord(recordid,record);
    assert(AppendRecord);
    node->SetModified();
    node->Write(record);
    return recordid;
  }else{
    TM_RTreeNode<dim,LeafInfo>* new_n =
    new TM_RTreeNode<dim,LeafInfo>(node->IsLeaf(),MinEntries(0),MaxEntries(0));

    for(int i = 0;i < node->EntryCount();i++){
      R_TreeInternalEntry<dim> e =
          (R_TreeInternalEntry<dim>&)(*node)[i];
      TM_RTreeNode<dim,LeafInfo>* n = rtree_in->GetMyNode(
        e.pointer,false,rtree_in->MinEntries(0),rtree_in->MaxEntries(0));
      SmiRecordId recid;
      recid = DFVisit_Rtree(rtree_in,n);
      new_n->Insert(R_TreeInternalEntry<dim>(e.box,recid));
      delete n;
    }
    SmiRecordId recordid;
    SmiRecord record;
    int AppendRecord =
      file->AppendRecord(recordid,record);
    assert(AppendRecord);
    new_n->SetModified();
    new_n->Write(record);
    delete new_n;
    return recordid;
  }
}

/*
Copy an R-Tree, 1) Using BulkLoad 2) DF Traversal
The key issue is for different files, the recordId is different and can't be
controled (which is returned by berkeleydb library function)

*/

template <unsigned dim, class LeafInfo>
void TM_RTree<dim, LeafInfo>::Clone(TM_RTree<dim,LeafInfo>* rtree_in)
{
/*  assert(InitializeBulkLoad());
    BBox<dim> searchbox(rtree_in->BoundingBox());
    R_TreeLeafEntry<dim,LeafInfo> e;
    if(rtree_in->First(searchbox,e)){
      InsertBulkLoad(e);
      while(rtree_in->Next(e)){
        InsertBulkLoad(e);
      }
    }
  assert(FinalizeBulkLoad());*/

 ////////////////////// root /////////////////////////////
  SmiRecordId root_id = rtree_in->RootRecordId();

  TM_RTreeNode<dim,LeafInfo>* rootnode = rtree_in->GetMyNode(
        root_id,false,rtree_in->MinEntries(0),rtree_in->MaxEntries(0));

  root_id = DFVisit_Rtree(rtree_in,rootnode);

  header.nodeCount = rtree_in->NodeCount();
  header.entryCount = rtree_in->EntryCount();
  header.height = rtree_in->Height();
  header.rootRecordId = root_id;
  delete rootnode;
}

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::FinalizeBulkLoad()
{
  if ( bulkMode != true ) {
    return false;
  }
  // write all nodes to disk and delete them from memory

  // the array bli->node[i] contains all nodes, that need to be flushed outputs
  // onto disk. This is done bottom-up (starting at the leaf-level): Each node
  // is written to disk and then inserted into the node one level above.
  // During insertion, ancestor nodes may be changed, written to disk or being
  // replaced by other nodes.
  bool finished = false;
  for(int i=0; i < MAX_PATH_SIZE; i++) { // level == 0 means the leaf level
    if(    !finished
        && i <= bli->currentHeight
        && bli->node[i] != NULL) { // need to write the node
      // request SMI for a fresh record
      assert(file->IsOpen());
      SmiRecordId recId;
      SmiRecord rec;
      int RecordAppended = file->AppendRecord(recId, rec);
      assert(RecordAppended);
      // write current node to that record
      bli->node[i]->Write(rec);

      if( i < bli->currentHeight ) { // Insert a non-root node into its father
        assert( i+1 <= bli->currentHeight );
        assert( bli->node[i+1] != NULL );
        if(i == 0) {// leaf level
          assert( bli->node[i]->IsLeaf() == true );
        } else { // inner level
          assert( bli->node[i]->IsLeaf() == false );
          // create an entry for this node to insert into the father
        }
        // create an entry for this node to insert into the father
        R_TreeInternalEntry<dim> entry( bli->node[i]->BoundingBox(),
                                        recId );
        bli->currentLevel = i+1;
        InsertBulkLoad(bli->node[i+1],entry);
      } else {// ( i == bli->currentHeight): replace the root node
        assert( i == bli->currentHeight );

        // Verify, that the current rtree is empty:
        assert( NodeCount() == 1 );
        assert( EntryCount() == 0 );
        SmiRecordId newRootId =  recId;

        // Remove old root node from the tree
        file->DeleteRecord( RootRecordId() );
        delete nodePtr;
        nodePtr      = NULL;
        currLevel    = 0;
        currEntry    = -1;
        reportLevel  = -1;
        searchBox    = false;
        searchType   = NoSearch;

        // Set new root to topmost node from bulkloading
        header.nodeCount    = bli->nodeCount;
        header.entryCount   = bli->entryCount;
        header.height       = bli->currentHeight;
        header.rootRecordId = newRootId;
        path[ 0 ]           = newRootId;

        WriteHeader();
        finished = true;
      }
    }
    // ALWAYS: delete the current node from memory
    if( bli->node[i] != NULL ) {
      delete bli->node[i];
      bli->node[i] = NULL;
    }
  } // end for
  delete bli;
  bli = NULL;


  if(!finished){
     // happens if no things are do do above
       WriteHeader();
  } 
  // re-initialize the rtree
  ReadHeader();
  assert( header.maxLeafEntries >= 2*header.minLeafEntries &&
          header.minLeafEntries > 0 );
  assert( header.maxInternalEntries >= 2*header.minInternalEntries &&
          header.minInternalEntries > 0 );
  currLevel = 0;
  //assert(nodePtr==0);
  nodePtr = GetNode( RootRecordId(),
                     currLevel == Height(),
                     MinEntries( currLevel ),
                     MaxEntries( currLevel ) );
  path[ 0 ] = header.rootRecordId;

  return true;
};

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::getFileStats( SmiStatResultType &result )
{
  result = file->GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << file->GetFileId();
  result.push_back(pair<string,string>("FilePurpose",
            "SecondaryRtreeIndexFile"));
  result.push_back(pair<string,string>("FileId",fileid.str()));
  return true;
}

/*
6 Template functions for the type constructors

6.1 ~Out~-function

It does not make sense to have an R-Tree as an independent value
since the record ids stored in it become obsolete as soon as
the underlying relation is deleted. Therefore this function
outputs will show only some statistics about the tree.

*/
template <unsigned dim>
ListExpr OutTMRTree(ListExpr typeInfo, Word value)
{
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    ListExpr bboxList, appendList;
    TM_RTree<dim, TwoLayerLeafInfo> *rtree =
        (TM_RTree<dim, TwoLayerLeafInfo> *)value.addr;

    bboxList = nl->OneElemList(
                 nl->RealAtom( rtree->BoundingBox().MinD( 0 ) ));
    appendList = bboxList;
    appendList = nl->Append(appendList,
                            nl->RealAtom( rtree->BoundingBox().MaxD( 0 ) ));

    for( unsigned i = 1; i < dim; i++)
    {
      appendList = nl->Append(appendList,
                              nl->RealAtom( rtree->BoundingBox().MinD( i ) ));
      appendList = nl->Append(appendList,
                              nl->RealAtom( rtree->BoundingBox().MaxD( i ) ));
    }

    return nl->FiveElemList(
             nl->StringAtom( "TM-RTree statistics" ),
             nl->TwoElemList( nl->StringAtom( "Height" ),
                              nl->IntAtom( rtree->Height() ) ),
             nl->TwoElemList( nl->StringAtom( "# of (leaf) entries" ),
                              nl->IntAtom( rtree->EntryCount() ) ),
             nl->TwoElemList( nl->StringAtom( "# of nodes" ),
                              nl->IntAtom( rtree->NodeCount() ) ),
             nl->TwoElemList( nl->StringAtom( "Bounding Box" ), bboxList ) );
  }
  else
  {
    ListExpr bboxList, appendList;
    TM_RTree<dim, TupleId> *rtree = (TM_RTree<dim, TupleId> *)value.addr;

    bboxList = nl->OneElemList(
                 nl->RealAtom( rtree->BoundingBox().MinD( 0 ) ));
    appendList = bboxList;
    appendList = nl->Append(appendList,
                            nl->RealAtom( rtree->BoundingBox().MaxD( 0 ) ));

    for( unsigned i = 1; i < dim; i++)
    {
      appendList = nl->Append(appendList,
                              nl->RealAtom( rtree->BoundingBox().MinD( i ) ));
      appendList = nl->Append(appendList,
                              nl->RealAtom( rtree->BoundingBox().MaxD( i ) ));
    }

    return nl->FiveElemList(
             nl->StringAtom( "TM-RTree statistics" ),
             nl->TwoElemList( nl->StringAtom( "Height" ),
                              nl->IntAtom( rtree->Height() ) ),
             nl->TwoElemList( nl->StringAtom( "# of (leaf) entries" ),
                              nl->IntAtom( rtree->EntryCount() ) ),
             nl->TwoElemList( nl->StringAtom( "# of nodes" ),
                              nl->IntAtom( rtree->NodeCount() ) ),
             nl->TwoElemList( nl->StringAtom( "Bounding Box" ), bboxList ) );
  }

}

/*
6.2 ~In~-function

Reading an R-Tree from a list does not make sense because an R-Tree
is not an independent value. Therefore calling this function leads
to program abort.

*/
template <unsigned dim>
Word InTMRTree( ListExpr typeInfo, ListExpr value,
              int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  return SetWord(Address(0));
}

/*
6.3 ~Create~-function

*/
template <unsigned dim>
Word CreateTMRTree( const ListExpr typeInfo )
{

  if( nl->BoolValue(nl->Fourth(typeInfo)) == true ){
     return SetWord( new TM_RTree<dim, TwoLayerLeafInfo>( 4000 ) );
  }else{
    return SetWord( new TM_RTree<dim, TupleId>( 4000 ) );
  }
}

/*
6.4 ~Close~-function

*/
template <unsigned dim>
void CloseTMRTree( const ListExpr typeInfo, Word& w )
{
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    TM_RTree<dim, TwoLayerLeafInfo>* rtree = (TM_RTree<dim,
                                            TwoLayerLeafInfo>*)w.addr;
    delete rtree;
  }
  else
  {
    TM_RTree<dim, TupleId>* rtree = (TM_RTree<dim, TupleId>*)w.addr;
    delete rtree;
  }
}

/*
6.5 ~Clone~-function

Not implemented yet.

implemented by Jianqiu xu --- 2009.11.30

*/
template <unsigned dim>
Word CloneTMRTree( const ListExpr typeInfo, const Word& w )
{
/////////////// new implementation ////////////////////////////
  TM_RTree<dim,TupleId>* rtree = (TM_RTree<dim,TupleId>*)w.addr;
  TM_RTree<dim,TupleId>* newrtree =
                  new TM_RTree<dim,TupleId>(4000);

  newrtree->Clone(rtree);
  return SetWord( newrtree);

////////////////  original version ////////////////////////////////////////
//  return SetWord( Address(0) );
}

/*
6.6 ~Delete~-function

*/
template <unsigned dim>
void DeleteTMRTree( const ListExpr typeInfo, Word& w )
{

  if (nl->ListLength(typeInfo) == 4)
  {
    if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
      {
        TM_RTree<dim, TwoLayerLeafInfo>* rtree = (TM_RTree<dim,
                                                TwoLayerLeafInfo>*)w.addr;
        rtree->DeleteFile();
        delete rtree;
        return;
      }
  }

  TM_RTree<dim, TupleId>* rtree = (TM_RTree<dim, TupleId>*)w.addr;
  rtree->DeleteFile();
  delete rtree;

}

/*
6.7 ~Cast~-function

*/
template <unsigned dim>
void* CastTMRTree( void* addr)
{
  return ( 0 );
}

/*
6.8 ~Open~-function

*/
template <unsigned dim>
bool OpenTMRTree( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{

  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    TM_RTree<dim, TwoLayerLeafInfo> *rtree =
        new TM_RTree<dim, TwoLayerLeafInfo>( fileid );
    value = SetWord( rtree );
  }
  else
  {
    TM_RTree<dim, TupleId> *rtree = new TM_RTree<dim, TupleId>( fileid );
    value = SetWord( rtree );
  }

  return true;
}

/*
6.9 ~Save~-function

*/

template <unsigned dim>
bool SaveTMRTree( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{

  SmiFileId fileId;

  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    TM_RTree<dim, TwoLayerLeafInfo> *rtree =
        (TM_RTree<dim, TwoLayerLeafInfo> *)value.addr;
    fileId = rtree->FileId();
  }
  else
  {
    assert(value.addr);
    TM_RTree<dim, TupleId> *rtree = (TM_RTree<dim, TupleId> *)value.addr;
    fileId = rtree->FileId();

  }
  valueRecord.Write( &fileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  return true;
}

/*
6.10 ~SizeOf~-function

*/
template <unsigned dim>
int SizeOfTMRTree()
{
  return 0;
}

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::Open(SmiRecord& valueRecord,
                                  size_t& offset,
                                  string typeInfo,
                                  Word &value)
{
  SmiFileId fileId;
  size_t n = sizeof(SmiFileId);
  valueRecord.Read(&fileId, n, offset);
  offset += n;
  value = SetWord(new TM_RTree<dim, LeafInfo> (fileId));
  return true;
};

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim, LeafInfo>::Save(SmiRecord& valueRecord,
                                 size_t& offset)
{
  const size_t n=sizeof(SmiFileId);
  SmiFileId fileId= this->FileId();
  if (valueRecord.Write(&fileId, n, offset) != n) return false;
  offset += n;
  return true;
};

template <unsigned dim, class LeafInfo>
bool TM_RTree<dim,LeafInfo>::InitializeBLI(const bool& leafSkipping)
{
    if(bulkMode)cout<<"bulkMode"<<endl;
    if(bli)cout<<"bli"<<endl;
//    if(bulkMode || bli != NULL)
//      return false;
    bulkMode = true;
    bli = new TM_BulkLoadInfo<dim,LeafInfo>(leafSkipping);
    return true;
}

/*
calculate the transportation mode for each node
from bottom to up

*/
template <unsigned dim, class LeafInfo>
bool TM_RTree<dim,LeafInfo>::CalculateTM(Relation* rel, int attr_pos)
{

  SmiRecordId node_id = RootRecordId();
  long tm = CalculateNodeTM(node_id, rel, attr_pos);

  TM_RTreeNode<3, TupleId>* node = 
                     GetMyNode(node_id,false, MinEntries(0), MaxEntries(0));

  ////////////write the value ///////////////
  node->SetTMValue(tm);
  node->Write(file, node_id);

  delete node;

  return true;
}

/*
recursively calling the son node 

*/
template <unsigned dim, class LeafInfo>
long TM_RTree<dim,LeafInfo>::CalculateNodeTM(SmiRecordId nodeid, 
                                             Relation* rel, int attr_pos)
{
   TM_RTreeNode<3, TupleId>* node = GetMyNode(nodeid,false,
                                    MinEntries(0), MaxEntries(0));

   if(node->IsLeaf()){

       int pos = -1;
       bitset<ARR_SIZE(str_tm)> modebits;
       modebits.reset();

      for(int j = 0;j < node->EntryCount();j++){

       R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];
       Tuple* tuple = rel->GetTuple(e.info, false);
       int m = ((CcInt*)tuple->GetAttribute(attr_pos))->GetIntval();//bit index
       tuple->DeleteIfAllowed();
// //      cout<<"j "<<j<<" tm "<<GetTMStr(m)<<endl;
       pos = (int)ARR_SIZE(str_tm) - 1 - m;
//        if(pos < 0) pos = (int)(ARR_SIZE(str_tm) - 1 - m);
//        else assert(pos == (int)(ARR_SIZE(str_tm) - 1 - m));
       assert(0 <= pos && pos <= (int)(ARR_SIZE(str_tm) - 1 - m));

       modebits.set(pos, 1);//set the value for each entry:general method
      }
      /////////checking for leaf node, mode should be the same///////////////

/*      bitset<ARR_SIZE(str_tm)> modebits;
      modebits.reset();
      modebits.set(pos, 1);*/
      long tm = modebits.to_ulong();
//       cout<<"leaf node "<<nodeid<<endl;
//       cout<<tm<<" "<<modebits.to_string()<<" "<<GetModeString(tm)<<endl;

      node->SetTMValue(tm);
      node->Write(file, nodeid);
      delete node;

      return tm;

   }else{
      bitset<ARR_SIZE(str_tm)> modebits;
      modebits.reset();
        for(int j = 0;j < node->EntryCount();j++){

          R_TreeInternalEntry<3> e =
                (R_TreeInternalEntry<3>&)(*node)[j];
         int son_tm = CalculateNodeTM(e.pointer, rel, attr_pos);

         bitset<ARR_SIZE(str_tm)> m_bit(son_tm);
//         ///////////// union value of each son tm to tm//////////////
// /*        cout<<"new one "<<m_bit.to_string()
//             <<" before "<<modebits.to_string()<<endl;*/
         modebits = modebits | m_bit;
// //        cout<<"after"<<modebits.to_string()<<endl;
// 
       }

       long tm = modebits.to_ulong();
//     cout<<"non leaf node "<<nodeid<<endl;
//       cout<<modebits.to_ulong()<<" "<<modebits.to_string()
//            <<" "<<GetModeString(tm)<<endl;
       node->SetTMValue(tm);
       node->Write(file, nodeid);
       delete node;
       return tm;
   }

}

/*
after the TMRtree declaration

*/

template <unsigned dim, class LeafInfo>
TM_RTreeNode<dim, LeafInfo>& TM_RTree<dim, LeafInfo>::Root()
// Loads nodeptr with the root node
{
  GotoLevel( 0 );

  return *nodePtr;
}

#endif