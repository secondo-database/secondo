/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

{\Large \bf Anhang F: Hauptspeicher R-Baum }

[1] Header-File of PlugJoin-Algebra

October 2004, Herbert Schoenhammer,


[TOC]

1 Overview

This header file implements a main memory representation of a R-Tree. Setting some parameters the R-Tree-behaviour of Guttman or the R[*]-Tree of Kriegel et al. can be selected. Details of the implementation are chosen to satisfy the Plug\&Join  algorithm of Bercken, Schneider and Seeger: Plug\&Join: An Easy-To-Use Generic Algorithm for Efficiently Processing Equi and Non-equi Joins.

This algebra is derived from the RTreeAlgebra of Victor Almeida. Especially the node-splitting-algorithms are used without any change. Changes are made for storing the R-Tree in main memory, setting a miximum number of nodes and another behaviour in the case of overflows in leaves. The number of entries in a node can be choosen and is not depending on the blocksize of the disk.

2 Defines and Includes

*/

#ifndef __RTREEPNJ_ALGEBRA_H__
#define __RTREEPNJ_ALGEBRA_H__

#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#include <iostream>
#include <stack>
#include <vector>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"


extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle
#define ArrayIndex long

#ifndef DOUBLE_MAX
#define DOUBLE_MAX (1.7E308)
#endif


/*
2 Constants

*/

const int MAX_PATH_SIZE = 50;

/*
The maximum height of the R-Tree.

Below are a bunch of constants that will govern
some decisions that have to be made when inserting or
deleting a node into the R-Tree. By setting this flags
in a particular pattern, it should be possible to obtain
assorted flavors of R-Tree and R[*]-tree behaviours.

*/

const int minimize_leafnode_overlap = 1;
/*
Checked while choosing the node in which a new entry will be placed.
Makes the insertion algorithm behave differently when next to the
bottom-most level of the tree, in that it will try to minimize
leaf node overlap instead of minimizing area enlargement (as is done
with the upper levels).
Used in R[*]-trees.

*/

const int leafnode_subset_max = 32;
/*
If minimize\_leafnode\_overlap is set, this variable determines
how many of the leafnodes will actually be checked (Kriegel et al
recommend 32). This set is chosed amongst the leafnodes that
result in least area enlargement.

*/

int do_forced_reinsertion = 0;
/*
Checked while trying to insert an entry into a full leaf node. If set,
some of the entries of the leaf node (governed by variable
~forced\_reinsertion\_percent~ below) will be reinserted into the tree.
Used in R[*]-trees.

Reinsertion may cause further node-splittings. This further node-splittings
could result in creation of new nodes, which could have not enough space in
the array of the R-Tree. So ~do\_forced\_reinsertion~ is not a constant. If the
number of free components in the array is too small for taking place of
possible new nodes in the R-Tree, ~do\_forced\_reinsertion~ is disabled
(set to 0). (For details see Insert-method of the R-Tree.).

Building persistent R-Trees, do\_forced\_reinsertion causes a better structure
ot the R-Tree. But for R-Trees in main memory you should not use
~do\_forced\_reinsertion~.

*/

const int forced_reinsertion_percent = 30;
/*
A number between 0 and 100 that indicates the percentage full leaf node
entries that will be reinserted if the node overflows.
Used in R[*]-trees. Beckmann et al. suggest a value of 30.

Only one of the three next flags below should be set.

*/

const int do_linear_split = 0;
/*
If set, Guttman's linear split algorithm is performed.
Used in standard R-Trees

*/

const int do_quadratic_split = 0;
/*
If set, Guttman's quadratic split algorithm is performed.
Used in standard R-Trees

*/

const int do_axis_split = 1;
/*
If set, Beckmann et al's axis split algorithm is performed.

For R-Trees in main memory, you can use one of the split-algorithms. There is
no recognizable difference in speed. But for a better structure of the R-Tree
you should use ~do\_axis\_split~.

*/

const int page_size = 4000;
/*
The page size of external memory (harddisk). Needed for
storing the ~partitions~ (see Bercken et al.) in SMI-files
( Secondo ~S~torage ~M~anagement ~I~nterface).

*/

const int default_entries_per_node = 15; 
/*
The default number of entries for one node or leaf. A value of 32 is suggested
by Bercken et al. Tests for this implementation recognized a value of 15 as best
choice.

*/

const int min_entries_per_centage = 40;
/*
This constant controls the minimum number of entries in a node. It is
a per centage value (e.g. default\_entries\_per\_node [*]
min\_entries\_per\_centage / 100). Beckmann et al. suggest a value of 40.

Also necessary for estimating the maximum total number of nodes.

*/

const int max_leaves_of_rtree = 10000;
/*
The amount of main memory used by the R-Tree is controlled by the
maximum number of leaves of the R-Tree. This maximum number is not
to be recognized as a exact number. With this value, the maximum
number of nodes (incl. inner nodes) of the R-Tree is estimated (the
upper bound).

The actual growth of the R-Tree depends on the inserted rectangles (and
other aspects like the setting of ~do\_axis\_split~ or ~do\_linear\_split~).
Therefore the exact number of inner nodes and leaves can not be
guaranteed. Only the maximum number of nodes (the sum of inner nodes and
leaves) can be guaranteed.

*/

const float scalingFactor = 17;
/*
Following Bercken et al., the number of nodes to use is computed by the
formula: ~nodesToUse = scalingFactor * (numberOfTuples(R) + numberOfTuples(S))
 / ( default\_entries\_per\_node * max\_leaves\_of\_rtree )~.

Beckmann et al. suggest 1.5. For this implementation a value of 17 is tested
as best choice.

*/

const int min_leaves_of_rtree = 40;
/*
Computing number of nodes in a R-Tree with the above formula, the result could
be very small. Especially regarding to the "lowest" trees in the recursive
Plug\&Join-Algorithm. So the minimum number of nodes in a R-Tree must be set.

For this implementation a value of 40 is tested as best choice.

*/


/*
3 Struct ~R\_TreeEntryPnJ~

This struct will store an entry inside a node of the R\_Tree.

*/
template<unsigned dim>
struct R_TreeEntryPnJ
{
  BBox<dim> box;
/*
If it is a leaf entry, then the bounding box spatially constains the spatial
object. If it is an internal entry, the bounding box contains all bounding
boxes of the entries of its child node.

*/

  ArrayIndex pointer;
/*
Points to an ~SmiRecord~ in a file. If it is a leaf entry, this is the record
where the spatial object is stored, otherwise this is the pointer to its
child node.

*/

  R_TreeEntryPnJ() {}
/*
The simple constructor.

*/

  R_TreeEntryPnJ( const BBox<dim>& box, long pointer = 0 ) :
     box( box ), pointer( pointer )
    {}
/*
The second constructor passing a bounding box and a page.

*/
};

/*
5 Class ~R\_TreeNodePnJ~

This is a node in the R-Tree.

*/
template<unsigned dim>
class R_TreeNodePnJ
{
  public:
    R_TreeNodePnJ( const bool leaf, const int min, const int max );
/*
The constructor.

*/

    R_TreeNodePnJ( const R_TreeNodePnJ<dim>& n );
/*
The copy constructor.

*/

    ~R_TreeNodePnJ();
/*
The destructor.

*/


    int Size() const;
/*
Returns the maximum size in bytes of this node.

*/

    int EntryCount() const
      { return count; }
/*
Returns the number of entries in this node.

*/

    int MaxEntries() const
      { return maxEntries; }
/*
Returns the max number of entries this node supports.

*/

    int MinEntries() const
      { return minEntries; }
/*
Returns the max number of entries this node supports.

*/

    bool IsLeaf() const
      { return leaf; }
/*
Tells whether this node is a leaf node.

*/

    bool IsInsertOverflow() const
      { return insertOverflow; }
/*
Tells whether the leaf has had an insertion-overflow in static structure of
the R-Tree.

*/

    R_TreeEntryPnJ<dim>& operator[] ( int index ) const
      { assert( index >= 0 && index <= maxEntries ); return entry[ index ]; }
/*
Returns entry given by index.

*/

    BBox<dim> BoundingBox() const;
/*
Returns the bounding box of this node.

*/

    BBox<dim> BoundingBoxOfEntry (const ArrayIndex& pointer) const;
/*
Returns the bounding box of the entry corresponding to ~pointer~.

*/


    void Clear()
      { leaf = false; count = 0; }
/*
Clears all entries.

*/

    void Flush()
      { assert (leaf && (maxEntries == count));
        insertOverflow = true; }
/*
Clears all entries of a leave if the leaf has an overflow. For performance
reasons it is better not to delete the entries, but to set ~insertOverflow~ to
~true~.

*/

    R_TreeNodePnJ<dim>& operator = ( const R_TreeNodePnJ<dim>& );
/*
Assignment operator between nodes.

*/

    bool Remove( int );
/*
Removes the given entry from the node. Returns true if successful
or false if underflow (The entry is deleted regardless).

*/

    bool Insert( const R_TreeEntryPnJ<dim>& e );
/*
Adds ~e~ to this node if possible. Returns ~true~ if successful,
i.e., if there is enough room in the node,  or ~false~ if the insertion
caused an overflow. In the latter case, the entry is inserted,
but the node should be split by whoever called the insert method.

*/

    void Split( R_TreeNodePnJ<dim>& n1, R_TreeNodePnJ<dim>& n2 );
/*
Splits this node in two: ~n1~ and ~n2~, which should be empty nodes.

*/

    void UpdateBox( BBox<dim>& box, const ArrayIndex& pointer );
/*
Update entry corresponding to ~pointer~ to have bounding box ~box~.

*/

    ostream& Print( ostream &os );
/*
For debugging purposes only.

*/

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

    R_TreeEntryPnJ<dim>* const entry;
/*
Array of entries.

*/

    bool insertOverflow;
/*
In a new node this flag is initialized with false. It is set by ~Flush~-
Method. In a leave it indicates that an insert in a full leave had be done,
after the structure of the R-Tree is no more dynamic.

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
};

/*
5.1 First constructor

*/
template<unsigned dim>
R_TreeNodePnJ<dim>::R_TreeNodePnJ( const bool leaf, const int min, const int max ) :
leaf( leaf ),
minEntries( min ),
maxEntries( max ),
count( 0 ),
entry( new R_TreeEntryPnJ<dim>[ max + 1] ),
insertOverflow ( false )
{
}

/*
5.2 Second constructor

*/
template<unsigned dim>
R_TreeNodePnJ<dim>::R_TreeNodePnJ( const R_TreeNodePnJ<dim>& node ) :
leaf( node.leaf ),
minEntries( node.minEntries ),
maxEntries( node.maxEntries ),
count( node.count ),
entry( new R_TreeEntryPnJ<dim>[ node.maxEntries + 1 ] ),
insertOverflow ( node.insertOverflow )
{
  for( int i = 0; i < node.count; i++ )
    entry[ i ] = node.entry[ i ];
}

/*
5.3 Destructor

*/
template<unsigned dim>
R_TreeNodePnJ<dim>::~R_TreeNodePnJ()
{
  delete [] entry;
}

/*
5.5 Method Size

*/

template<unsigned dim>
int R_TreeNodePnJ<dim>::Size() const
{
  int size = sizeof ( leaf ) +
             sizeof ( minEntries ) +
             sizeof ( maxEntries ) +
             sizeof ( count ) +
             sizeof ( insertOverflow ) ;

  size += sizeof( R_TreeEntryPnJ<dim> ) * MaxEntries();

  return size;
}

/*
5.6 Operation =

*/

template<unsigned dim>
R_TreeNodePnJ<dim>& R_TreeNodePnJ<dim>::operator = ( const R_TreeNodePnJ<dim>& node )
{
  assert( minEntries == node.minEntries && maxEntries == node.maxEntries );
  assert( count >= 0 && count <= maxEntries + 1 );

  for( int i = 0; i < node.count; i++ )
    entry[ i ] = node.entry[ i ];

  leaf = node.leaf;
  count = node.count;
  insertOverflow = node.insertOverflow;

  return *this;
}

/*
5.7 Method Remove

*/

template<unsigned dim>
bool R_TreeNodePnJ<dim>::Remove( int index )
{
  assert( index >= 0 && index < count );

  entry[ index ] = entry[ count - 1 ];
  count -= 1;

  return count >= minEntries;
}

/*
5.8 Method Insert

*/
template<unsigned dim>
bool R_TreeNodePnJ<dim>::Insert( const R_TreeEntryPnJ<dim>& ent )
{
  assert( count <= maxEntries );
  entry[ count++ ] = ent;
  return count <= maxEntries;
}

/*
5.9 Method LinearPickSeeds

*/
template<unsigned dim>
void R_TreeNodePnJ<dim>::LinearPickSeeds( int& seed1, int& seed2 ) const
{
  assert( EntryCount() == MaxEntries() + 1 );
    // This should be called only if the node has an overflow

  double maxMinVal[ dim ];
  double minMaxVal[ dim ];
  double minVal[ dim ];
  double maxVal[ dim ];
  double sep[ dim ];
  double maxSep = -DOUBLE_MAX;
  int maxMinNode[ dim ];
  int minMaxNode[ dim ];
  int bestD = -1;

  for( unsigned i = 0; i < dim; i++ )
  {
    maxMinVal[i] = -DOUBLE_MAX;
    minMaxVal[i] = DOUBLE_MAX;
    minVal[i] = DOUBLE_MAX;
    maxVal[i] = -DOUBLE_MAX;
    maxMinNode[i] = -1;
    minMaxNode[i] = -1;
  }

  for( int i = 0; i < EntryCount(); i++ )
  {
    for( unsigned d = 0; d < dim; d++ )
    {
      if( entry[ i ].box.MinD( d ) > maxMinVal[ d ] )
      {
        maxMinVal[ d ] = entry[ i ].box.MinD( d );
        maxMinNode[ d ] = i;
      }

      if( entry[ i ].box.MinD( d ) < minVal[ d ] )
      minVal[ d ] = entry[ i ].box.MinD( d );

      if( entry[ i ].box.MaxD( d ) < minMaxVal[ d ] )
      {
        minMaxVal[ d ] = entry[ i ].box.MaxD( d );
        minMaxNode[ d ] = i;
      }

      if( entry[ i ].box.MaxD( d ) > maxVal[ d ] )
        maxVal[ d ] = entry[ i ].box.MaxD( d );
    }
  }

  for( unsigned d = 0; d < dim; d++ )
  {
    assert( maxMinNode[ d ] != -1 && minMaxNode[ d ] != -1 );
    assert( maxVal[ d ] > minVal[ d ] );
    sep[ d ] = double( maxMinVal[ d ] - minMaxVal[ d ] ) / (maxVal[ d ] - minVal[ d ]);
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
5.10 Method QuadraticPickSeeds

*/

template<unsigned dim>
void R_TreeNodePnJ<dim>::QuadraticPickSeeds( int& seed1, int& seed2 ) const
{
  assert( EntryCount() == MaxEntries() + 1 );
    // This should be called only if the node has an overflow

  double bestWaste = -DOUBLE_MAX;
  double *area = new double[ MaxEntries() + 1 ]; // Compute areas just once
  int i;

  for( i = 0; i < EntryCount(); i++ )
  {
    int j;

    area[ i ] = entry[ i ].box.Area();

    for( j = 0; j < i; ++j )
    {
      double totalArea = entry[ i ].box.Union( entry[ j ].box ).Area();
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
5.11 Method QuadraticPickNext

*/
template<unsigned dim>
int R_TreeNodePnJ<dim>::QuadraticPickNext( BBox<dim>& b1, BBox<dim>& b2 ) const
{
  double area1 = b1.Area();
  double area2 = b2.Area();
  double bestDiff = -1;
  int besti = -1;

  for( int i = 0; i < count; i++ )
  {
    double d1 = b1.Union( entry[ i ].box ).Area() - area1;
    double d2 = b2.Union( entry[ i ].box ).Area() - area2;
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

struct SortedArrayItem
{
  int index;
  double pri;
};

int myComparePnJ( const void* a, const void* b );

class SortedArray
{
  public:
    SortedArray( unsigned max ) : max( max ), i( 0 ), n( 0 ), sorted( 0 )
      { a = new SortedArrayItem[ max ]; }

    ~SortedArray ()
      { delete [] a; }

    int empty() const
      { return i == n; }

    void push( int index, double pri );

    int head()
      { if( !sorted ) sort(); return a[ i ].index; }

    double headPri()
      { if( !sorted ) sort(); return a[ i ].pri; }

    int pop()
      {  if( !sorted ) sort(); assert( i <= n ); return a[ i++ ].index; }

  private:
    int max;
    int i;
    int n;
    int sorted;
    SortedArrayItem* a;

    void sort()
    {
      assert( i == 0 && n > 0 && !sorted );

      qsort( a, n, sizeof( SortedArrayItem ), myComparePnJ );
      sorted = 1;
    }
};

inline void SortedArray::push( int index, double pri )
{
  if( i == n )
  {
    i = n = 0;
    sorted = 0;
  }

  assert( n < max && i == 0 );
  a[ n ].index = index;
  a[ n ].pri = pri;
  n++;
};

/*
5.12 Method Split

*/
template<unsigned dim>
void R_TreeNodePnJ<dim>::Split( R_TreeNodePnJ<dim>& n1, R_TreeNodePnJ<dim>& n2 )
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
    double minMarginSum = DOUBLE_MAX;
    int minMarginAxis = -1;

    for( unsigned d = 0; d < dim; d++ )
    { // Compute sorted lists. Sort entry numbers by minimum value of axis 'd'.
      int* psort = sortedEntry[ 2*d ] = new int[ MaxEntries() + 1 ];
      SortedArray sort( MaxEntries() + 1 );
      int i;

      for( i = 0; i <= MaxEntries(); i++ )
        sort.push( i, entry[ i ].box.MinD( d ) );

      for( i = 0; i <= MaxEntries(); i++ )
        *psort++ = sort.pop();

      assert( sort.empty() );

      // Sort entry numbers by maximum value of axis 'd'
      psort = sortedEntry[ 2*d + 1 ] = new int[ MaxEntries() + 1 ];
      for( i = 0; i <= MaxEntries(); i++ )
        sort.push( i, entry[ i ].box.MaxD( d ) );

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

        b1[ 0 ] = entry[ psort[ 0 ] ].box;
        b2[ 0 ] = entry[ psort[ MaxEntries() ] ].box;

        for( i = 1; i <= MaxEntries(); i++ )
        {
          b1[ i ] = b1[ i - 1 ].Union( entry[ psort[ i ] ].box );
          b2[ i ] = b2[ i - 1 ].Union( entry[ psort[ MaxEntries() - i ] ].box );
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

          assert( pstat - stat <= (int)(dim*dim*(MaxEntries() + 2 - 2*MinEntries())) );
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
      double minOverlap = DOUBLE_MAX;
      double minArea = DOUBLE_MAX;
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
              pstat->overlap == minOverlap && pstat->area < minArea )
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
        n1.Insert( entry[ sort[ i ] ] );

      for( int i = minSplitPoint + 1; i <= MaxEntries(); i++ )
        n2.Insert( entry[ sort[ i ] ] );

      assert( n1.BoundingBox().Intersection( n2.BoundingBox() ).Area() == minOverlap );

      // Deallocate the sortedEntry arrays
      for( unsigned i = 0; i < 2*dim; i++)
        delete sortedEntry[ i ];

      delete [] stat;
    }
  }
  else
  { // Do regular R-Tree split
    int seed1, seed2; // Pick seeds

    if( do_quadratic_split )
      QuadraticPickSeeds( seed1, seed2 );
    else
    {
      assert( do_linear_split );

      LinearPickSeeds( seed1, seed2 );
    }

    // Put the two seeds in n1 and n2 and mark them
    BBox<dim> box1 = entry[ seed1 ].box;
    BBox<dim> box2 = entry[ seed2 ].box;
    n1.Insert( entry[ seed1 ] );
    n2.Insert( entry[ seed2 ] );

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
          for( i = 0; i < EntryCount() ; i++, notAssigned-- )
            n1.Insert( entry[ i ] );

          count = 0;
          assert( notAssigned == 0 );
        }
        else if( n2.EntryCount() + notAssigned == n2.MinEntries() )
        { // Insert all remaining entries in n2
          for( i = 0; i < EntryCount(); ++i, notAssigned-- )
            n2.Insert( entry[ i ] );

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

          union1 = box1.Union( entry[ i ].box );
          union2 = box2.Union( entry[ i ].box );

          if( union1.Area() - box1.Area() < union2.Area() - box2.Area() )
          {
            n1.Insert( entry[ i ] );
            box1 = union1;
          }
          else
          {
            n2.Insert( entry[ i ] );
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
}

/*
5.13 Method BoundingBox

*/
template<unsigned dim>
BBox<dim> R_TreeNodePnJ<dim>::BoundingBox() const
{
  if( count == 0 )
    return BBox<dim>( false );
  else
  {
    BBox<dim> result = entry[ 0 ].box;
    int i;

    for( i = 1; i < count; i++ )
      result = result.Union( entry[ i ].box );

    return result;
  }
}

/*
5.14 Method BoundingBoxOfEntry

*/
template<unsigned dim>
BBox<dim> R_TreeNodePnJ<dim>::BoundingBoxOfEntry(const ArrayIndex& pointer) const
{
  if ( count == 0)
     return BBox<dim> ( false );
  else
  {

    for (int i = 0; i < count; i++)
    {
      if ( entry[ i ].pointer == pointer )
      {
        BBox<dim> result = entry[ i ].box;
        return result;
      }
    }
    //should never reach this point
    assert ( 0 );
  }
}

/*
5.14 Method UpdateBox

*/
template<unsigned dim>
void R_TreeNodePnJ<dim>::UpdateBox( BBox<dim>& b, const ArrayIndex& pointer )
{

  for( int i = 0; i < count; i++ )
    if( entry[ i ].pointer == pointer )
    {
      entry[ i ].box = b;

      return;
    }

  // Should never reach this point
  assert( 0 );
}

/*
5.15 Method Print

*/
template<unsigned dim>
ostream& R_TreeNodePnJ<dim>::Print( ostream &os )
{
  os <<  "  Leaf=" << leaf << "  Count=" << count <<
         "  insertOverflow=" << insertOverflow << endl;
  for (int i = 1; i <= count; i++)
    {
       os << "  Entry: " << i << endl;
       os << "    Box:" << endl;

       for ( unsigned int j = 1; j <= dim; j++)
         cout << "      Dim:" << j << "  " << entry[i-1].box.MinD(j-1)
              << "   " << entry[i-1].box.MaxD(j-1) << endl;

       os << "    Pointer=" << entry[i-1].pointer << "  " << endl;
    }
  return os;
}

/*
5 Class ~R\_TreePnJ~

This class implements the R-Tree.

*/
template <unsigned dim>
class R_TreePnJ
{
  public:

    R_TreePnJ( const int pageSize,          //needed for storing the
                                            //leaf-overflows
               const int maxEntries,        //max # of entries of inner nodes
                                            // and leaves
               const int minEntriesPerCent, //min # of entries (given in a per
                                            //centage value)
               const int estimatedLeaves ); //# of leaves in the tree, could be
                                            //some more or less leaves,
                                            //depending on the data inserted
/*
The constructor. Creates an empty R-Tree.

*/


    ~R_TreePnJ();
/*
The destructor.

*/

    int MinEntries() const
      { return header.minEntries; }
/*
Returns the minimum number of entries per node.

*/

    int MaxEntries() const
      { return header.maxEntries; }
/*
Returns the maximum number of entries per node.

*/

    int EntryCount() const
      { return header.entryCount; }
/*
Return the total number of (leaf) entries in this tree.

*/


    int NodeCount() const
      { return header.nodeCount; }
/*
Returns the total number of nodes in this tree.

*/

    int MaxNodeCount() const
      { return header.maxNodeCount; }
/*
Returns the maximum number of nodes in this tree.

*/

    int Height() const
      { return header.height; }
/*
Returns the height of this tree.

*/


    int LeavesCount() const
      { return header.leavesCount; }
/*
Returns the actual number of leaves in this tree.

*/

    int EstimatedLeaves() const
      { return header.estimatedLeaves; }
/*
Returns the estimated maximum number of leaves in this tree. Could be some
more or less leaves.

*/

    int SizeOfRTree() const;
/*
Returns the size of the R-Tree.

*/

    int SizeOfRTreeHeader() const;
/*
Returns the size of the R-Tree header.

*/

    BBox<dim> BoundingBox();
/*
Returns the bounding box of this R-Tree.

*/

    bool Insert( const R_TreeEntryPnJ<dim>& e, ArrayIndex& leaveNo );
/*
Trys to insert the given entry ~e~ somewhere in the tree. If success the
method returns ~true~, in ~leaveNo~ the value -1 is returned in this case.
If no success, the method returns ~false~ and in ~leaveNo~ is the number
of the leave, where the entry would be inserted.

*/

    bool First( const BBox<dim>& box, R_TreeEntryPnJ<dim>& result,
                vector <ArrayIndex>& leavesOverflowed, int replevel = -1);
    bool Next ( R_TreeEntryPnJ<dim>& result,
                vector <ArrayIndex>& leavesOverflowed );
/*
Sets ~result~ to the (leaf) entry corresponding to the first/next
object whose bounding box overlaps ~box~.

Returns ~true~ if a suitable entry was found and ~false~ if not.

Setting ~replevel~ to a value mot equal -1 forces the search to return
entries at that level of the tree regardless of whether they
are at leaf nodes or not.

~leavesOverflowed~ is the list of all leaves visited during search, for
which IsInsertOverflow() == true (that means they had an overflow when
the structure of the R-Tree was no more dynamic).

*/

    R_TreeNodePnJ<dim> *FlushLeave( const ArrayIndex& address );
/*
Reads a node at R-Tree-array position ~address~ and returns the copy of this
node. This node must be deleted somewhere. The node must be a leaf.
The flag ~insertOverflow~ of the node is set to true, indicating that the node
had an overflow during inserting tuples in the R-Tree.

*/

    void R_TreePnJ<dim>::Info();
/*
Prints some information about the generated R-Tree on the screen.
Perhaps for debugging or some other interesting aspects of the R-Tree.

*/


    void R_TreePnJ<dim>::DebugOutput(ArrayIndex nodeNo);
/*
For debugging purposes only.
If nodeNo == -1 the entire tree is printed on cout.
Otherwise the node rtree[address] with address==nodeNo is printed.

*/

  private:
    R_TreeNodePnJ<dim>* rtree;
/*
The array for the R-Tree.

*/

    struct Header
    {
      int minEntries;      	// min # of entries per node.
      int maxEntries;      	// max # of entries per node.
      int nodeCount;       	// number of nodes in this tree.
      int entryCount;      	// number of entries in this tree.
      int height;          	// actual height of the tree.
      int maxNodeCount;		// max # of nodes in this tree.
      int leavesCount;          // actual # of leaves
      int estimatedLeaves;      // estimated # of leaves in the tree

      Header() :
        minEntries( 0 ), maxEntries( 0 ),
        nodeCount( 0 ), entryCount( 0 ), height( 0 ),
        maxNodeCount ( 0 ), leavesCount ( 0 ), estimatedLeaves ( 0 )
        {}
      Header( int minEntries, int maxEntries, int nodeCount,
              int entryCount, int height, int maxNodeCount,
              int leavesCount, int estimatedLeaves) :
        minEntries( minEntries ), maxEntries( maxEntries ),
        nodeCount( nodeCount ), entryCount( entryCount ), height( height ),
        maxNodeCount ( maxNodeCount), leavesCount ( leavesCount ),
        estimatedLeaves ( estimatedLeaves )
        {}
    } header;
/*
The header of the R-Tree.

*/

    ArrayIndex path[ MAX_PATH_SIZE ];
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

    R_TreeNodePnJ<dim> *nodePtr;
/*
The current node of the R-Tree.

*/

    ArrayIndex nodePtrNo;
/*
The number of the node nodePtr points to (in the array of nodes).

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
/*
Bounding box for first/next.

*/

    bool scanFlag;
/*
A flag that tells whether we're in the middle of a First/Next
scan of the tree.

*/

    int maxNodesInInsert;
/*
Max number of new nodes, which could arise in one insert of a data tuple.
Note: This number is only evident, if forced\_reinsertion is NOT done.

*/

    int maxNodesInReinforcedInsertion;
/*
Max number of new nodes, which could arise in one insert of a data tuple.
Note: This number is only evident, if forced\_reinsertion is done. If the
number of free nodes in the array is bigger than ~maxNodesInReinforcedInsertion~
do\_forced\_reinsertion is deactivated.

*/


    void InsertEntry( const R_TreeEntryPnJ<dim>& );
/*
Inserts given entry in current node.

*/

    void LocateBestNode( const R_TreeEntryPnJ<dim>& ent, int level );
/*
Locates the "best" node of level ~level~ to insert ~ent~.

*/

    bool FindEntry( const R_TreeEntryPnJ<dim>& ent );
/*
Finds the given entry ~ent~ in the tree. If successful, ~true~ is
returned and ~currEntry~ and ~nodePtr~ are set to point to the
found entry. Otherwise, ~false~ is returned.

*/

    void GotoLevel( const int& level );
/*
Loads the node at the given ~level~ (must be in the current path).

*/

    void DownLevel( const int& entryno );
/*
Loads the child node of the current node given by ~entryno~.

*/

    void UpLevel();
/*
Loads the father node.

*/

    void UpdateBox();
/*
Updates "bottom-up" bounding boxes of nodes in path
(i.e., from leaf to root).

UpdateBox() is necessary, if an insertion of a data entry in a leaf
has really be done.

*/

    void UpdateBoxOfAncestors(const R_TreeEntryPnJ<dim>& entry,
                              const ArrayIndex& LeaveNo);
/*
Updates "bottom-up" bounding boxes of nodes in path
(i.e., from leaf to root).

UpdateBoxOfAncestors() is necessary, if an insertion of a data entry
in a full leaf would be done. The bounding box of the leaf is not changed,
but the corresponding boundig boxes of the father node (and all further
nodes up to the root) must be updated.

*/

    void ComputeMaxNodesInInsert();
/*
Computes the maximal number of nodes, possibly arising during one insert
of a data entry in tn R-Tree. The numbers are stored in the variables
~maxNodesInInsert~ and ~maxNodesInReinforcedInsertion~.

*/
};

/*
6.1 The constructor

*/
template <unsigned dim>
R_TreePnJ<dim>::R_TreePnJ( const int pageSize,
                           const int maxEntries,
                           const int minEntriesPerCent,
                           const int estimatedLeaves ) :
header(),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox( false ),
scanFlag( false )
{
  //max # of entries per node
  header.maxEntries = maxEntries;

  //min # of entries per node;
  header.minEntries = (int)(maxEntries * minEntriesPerCent / 100);

  //estimated # of nodes in the R-Tree
  int maxNodesInNextLevel = estimatedLeaves;
  header.maxNodeCount = estimatedLeaves;
  while ( maxNodesInNextLevel > header.minEntries )
  {
    maxNodesInNextLevel = (int) ( maxNodesInNextLevel / header.minEntries);
    header.maxNodeCount += maxNodesInNextLevel;
  }

  //given and estimated # of leaves in the tree
  header.estimatedLeaves = estimatedLeaves;

   // initialize arrays
  for( int i = 0; i < 1; i++ )
  {
    overflowFlag[ i ] = 0;
  }
  path[0] = 0;  //the root node

  // the array for the nodes
  rtree  =  new R_TreeNodePnJ<dim>[header.maxNodeCount](true, MinEntries(), MaxEntries() ) ;

  header.leavesCount = 1;

  //pointer to the first array-element
  nodePtr = rtree;
  nodePtrNo = 0;

  header.nodeCount = 1;
  currLevel = 0;

  //computing the max # of new nodes inserting one data entry
  ComputeMaxNodesInInsert();
}

/*
6.2 Destruction

*/
template <unsigned dim>
R_TreePnJ<dim>::~R_TreePnJ()
{
   delete [] rtree;
}

/*
6.3 Method FlushLeave

*/
template <unsigned dim>
R_TreeNodePnJ<dim>* R_TreePnJ<dim>::FlushLeave( const ArrayIndex& address )
{ // only fullLeaves may be flushed
  assert ( rtree[address].IsLeaf() &&
           (rtree[address].EntryCount() == rtree[address].MaxEntries()) );

  R_TreeNodePnJ<dim>* node = new R_TreeNodePnJ<dim>( rtree[address] );
  rtree[address].Flush();
  return node;
}

/*
6.4 Method SizeOfRTree

*/
template<unsigned dim>
int R_TreePnJ<dim>::SizeOfRTree() const
{
  return (rtree[0].Size() * header.maxNodeCount) +  //sizeof (root-node) * max#
         SizeOfRTreeHeader();                       //SizeOfHeader
}

/*
6.5 Method SizeOfRTreeHeader

*/
template<unsigned dim>
int R_TreePnJ<dim>::SizeOfRTreeHeader() const
{
  return sizeof( Header );  //sizeof (node) * max#
}

/*
6.6 Method BoundingBox

*/
template <unsigned dim>
BBox<dim> R_TreePnJ<dim>::BoundingBox()
  // Returns the bounding box of this R_Tree
{
  BBox<dim> result = rtree[0].BoundingBox();
  return result;
}

/*
6.7 Method Insert

*/
template <unsigned dim>
bool R_TreePnJ<dim>::Insert( const R_TreeEntryPnJ<dim>& entry, ArrayIndex& leaveNo )
{
  scanFlag = false;

  //search the best leaf to insert data entry
  LocateBestNode( entry, header.height );

  //The maximum size of the R\_Tree must not be enlarged ! so,
  //the number of maximal possible node-splittings must be smaller
  //than the number of free nodes in the array (of the R-Tree)

  int freeNodesInTree = header.maxNodeCount - header.nodeCount;
  ComputeMaxNodesInInsert();

  if ( freeNodesInTree < maxNodesInReinforcedInsertion )
    do_forced_reinsertion = 0;

  if ( (freeNodesInTree >=  maxNodesInInsert) ||
     ( (freeNodesInTree < maxNodesInInsert) &&
     ( nodePtr->EntryCount() < nodePtr->MaxEntries() ) ) )
  {
    //the R-Tree is dynamic at this point of time
    InsertEntry( entry );
    header.entryCount++;
    leaveNo = -1;
    return true;
  }
  else
  {
    //the R-Tree is not dynamic anymore
    //therefore inserts are allowed only in leaves, which are not full
    //leaveNo returns the number of the node, the entry would be inserted
    leaveNo = nodePtrNo;
    UpdateBoxOfAncestors (entry, leaveNo);
    return false;
  }

}

/*
6.8 Method LocateBestNode

*/
template <unsigned dim>
void R_TreePnJ<dim>::LocateBestNode( const R_TreeEntryPnJ<dim>& entry, int level )
{
  GotoLevel( 0 );

  // Top down search for a node of level 'level'
  while( currLevel < level )
  {
    int bestNode = -1;
    if( currLevel + 1 == header.height && minimize_leafnode_overlap )
    { // Best node is the one that gives minimum overlap. However,
      // we should only take into consideration the k nodes that
      // result in least enlargement, where k is given by
      // leafnode_subset_max.
      SortedArray enlargeList( header.maxEntries + 1 );
      int i, j, k;

      for( i = 0; i < nodePtr->EntryCount(); i++ )
      {
        R_TreeEntryPnJ<dim> &son = (*nodePtr)[ i ];
        enlargeList.push( i, son.box.Union( entry.box ).Area() - son.box.Area() );
      }

      if( enlargeList.headPri() == 0.0 )
        bestNode = enlargeList.pop(); // No need to do the overlap enlargement tests
      else
      {
        double bestEnlargement = DOUBLE_MAX,
               bestoverlap = DOUBLE_MAX;

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
              R_TreeEntryPnJ<dim> &son = (*nodePtr)[ j ];
              overlapBefore += boxBefore.Intersection( son.box ).Area();
              overlapAfter += boxAfter.Intersection( son.box ).Area();
            }

          overlap = overlapAfter - overlapBefore;

          if( overlap < bestoverlap ||
              overlap == bestoverlap && enlargement < bestEnlargement )
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
      double bestEnlargement = DOUBLE_MAX;
      int i;

      for( i = 0; i < nodePtr->EntryCount(); i++ )
      {
        R_TreeEntryPnJ<dim> &son = (*nodePtr)[ i ];
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
6.9 Method GotoLevel

*/
template <unsigned dim>
void R_TreePnJ<dim>::GotoLevel( const int& level )
{
  if( currLevel == level )
  {
    if( nodePtr == NULL )
      {
        nodePtrNo = path [currLevel];
        nodePtr = &rtree [nodePtrNo];
      }
  }
  else
  {
    assert( level >= 0 && level <= Height() );

    currLevel = level;
    nodePtrNo = path[currLevel];
    nodePtr = &rtree [nodePtrNo];
  }
}

/*
6.10 Method DownLevel

*/
template <unsigned dim>
void R_TreePnJ<dim>::DownLevel( const int& entryNo )
{
  assert( currLevel != header.height );
  assert( nodePtr != 0 );
  assert( entryNo >= 0 && entryNo < nodePtr->EntryCount() );

  pathEntry[ currLevel ] = entryNo;
  path[ currLevel+1 ] = (*nodePtr)[ entryNo ].pointer;
  currLevel += 1;
  nodePtrNo = path[ currLevel ];
  nodePtr = &rtree [ nodePtrNo ];
}

/*
6.11 Method InsertEntry

*/
template <unsigned dim>
void R_TreePnJ<dim>::InsertEntry( const R_TreeEntryPnJ<dim>& entry )
{

  if( nodePtr->Insert( entry ) )
    UpdateBox();
  else
  {

    if( !do_forced_reinsertion || currLevel == 0 ||
        overflowFlag[ header.height - currLevel ] )
    { // Node splitting is necessary
      R_TreeNodePnJ<dim> *n1 = new R_TreeNodePnJ<dim>( nodePtr->IsLeaf(), MinEntries(), MaxEntries() );
      R_TreeNodePnJ<dim> *n2 = new R_TreeNodePnJ<dim>( nodePtr->IsLeaf(), MinEntries(), MaxEntries() );

      if ( nodePtr->IsLeaf() )
        header.leavesCount += 1;

      nodePtr->Split( *n1, *n2 );

      // Write split nodes and update parent
      if( currLevel == 0)
      { // splitting root node

        //make sure there is enough room in the array
        assert ( header.nodeCount <= header.maxNodeCount - 2);

        nodePtr->Clear();

        BBox<dim> n1Box( n1->BoundingBox() );
        ArrayIndex node1recno = header.nodeCount;
        rtree[node1recno] = *n1;
        delete n1;
        assert( nodePtr->Insert( R_TreeEntryPnJ<dim>( n1Box, node1recno ) ) );

        BBox<dim> n2Box( n2->BoundingBox() );
        header.nodeCount++;
        ArrayIndex node2recno = header.nodeCount;
        rtree[node2recno] = *n2;;
        delete n2;
        assert( nodePtr->Insert( R_TreeEntryPnJ<dim>( n2Box, node2recno ) ) );

        header.nodeCount++;
        header.height += 1;
      }
      else
      { // splitting non-root node

        //make sure there is enough room in the array
        assert ( header.nodeCount < header.maxNodeCount);

        ArrayIndex newNoderecno = header.nodeCount;
        R_TreeEntryPnJ<dim> newEntry( n2->BoundingBox(), newNoderecno );
        rtree[newNoderecno] = *n2;
        delete n2;

        header.nodeCount++;

        // Copy all entries from n1 to nodePtr
        *nodePtr = *n1;
        delete n1;

        UpdateBox();
        UpLevel();

        InsertEntry( newEntry );
      }
    }
    else
    { // Do forced reinsertion instead of split
      int reinsertLevel = header.height - currLevel;

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
      SortedArray distSort( MaxEntries() + 1 );

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
        R_TreeEntryPnJ<dim> *tmp = new R_TreeEntryPnJ<dim>[ MaxEntries() + 1 ];
        int *keepFlag = new int[ MaxEntries() + 1 ];
        int deleteCount, n = 0;

        for( int i = 0; i <= MaxEntries(); i++ )
          keepFlag[ i ] = 0;

        deleteCount = (forced_reinsertion_percent * MaxEntries()) / 100;
        if (deleteCount == 0) deleteCount = 1;

/*
If you choose less entries (for example MaxEntries()=5) AND you choose a small
value for ~forced\_reinsertion\_percent~ (for example
~forced\_reinsertion\_percent~=10) then at least ONE entry has to be reinserted.
If you do NOT want forced reinsertion, you have to select
~do\_forced\_reinsertion~=0.

*/
        assert( MaxEntries() - deleteCount >= MinEntries() );

        for( int i = MaxEntries()-deleteCount; i >= 0; i-- )
          keepFlag[ distSort.pop() ] = 1;

        for( int i = MaxEntries(); i >= 0; i-- )
          if( !keepFlag[ i ] )
          {
            tmp[ i ] = (*nodePtr)[ i ];
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
          R_TreeEntryPnJ<dim> reinsertEntry = tmp[ entryNo ];

          assert( !keepFlag[ entryNo ] );

          LocateBestNode( reinsertEntry, header.height - reinsertLevel );
          InsertEntry( reinsertEntry );
        }

        // Reset flag so that other insertion operations may cause the
        // forced reinsertion process to take place
        overflowFlag[ reinsertLevel ] = 0;

        delete [] tmp;
        delete [] keepFlag;
      }
    }
  }
}

/*
6.12 Method UpLevel

*/
template <unsigned dim>
void R_TreePnJ<dim>::UpLevel()
{
  assert( currLevel > 0 );

  currLevel -= 1;
  nodePtrNo = path[currLevel];
  nodePtr = &rtree [nodePtrNo ];
}

/*
6.13 Method UpdateBox

*/
template <unsigned dim>
void R_TreePnJ<dim>::UpdateBox()
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
6.14 Method UpdateBoxOfAncestors

*/
template <unsigned dim>
void R_TreePnJ<dim>::UpdateBoxOfAncestors(const R_TreeEntryPnJ<dim>& entry,
                                          const ArrayIndex& leaveNo)
{
  //not inserted entries may only appear in leaves with maxEntries
  assert ( nodePtr->IsLeaf() );
  assert ( nodePtr->MaxEntries() == nodePtr->EntryCount() );

  //make sure, the R-Tree has at leat a height of one
  assert ( currLevel > 0 );

  // Save where we were before
  int formerlevel = currLevel;

  //goto father node
  UpLevel();

  //updating bounding box in father node
  //of the leave the entry would be inserted)
  BBox<dim> fatherBox = nodePtr->BoundingBoxOfEntry (leaveNo);
  BBox<dim> newBox = fatherBox.Union (entry.box);
  nodePtr->UpdateBox (newBox, leaveNo);

  //updating all bounding boxes up to the root
  UpdateBox();

  // Return to where we were before
  GotoLevel( formerlevel );
}

/*
6.14 Method FindEntry

*/
template <unsigned dim>
bool R_TreePnJ<dim>::FindEntry( const R_TreeEntryPnJ<dim>& entry )
{
  // First see if the current entry is the one that is being sought,
  // as is the case with many searches followed by Delete
  if( currLevel == header.height &&
      currEntry >= 0 && currEntry < header.maxEntries &&
      nodePtr != 0 &&
      (*nodePtr)[ currEntry ].box == entry.box &&
      (*nodePtr)[ currEntry ].pointer == entry.pointer )
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
      if( currLevel == header.height ) // This is a leaf node. Search all entries
        while( currEntry < nodePtr->EntryCount() )
        {
          if( (*nodePtr)[ currEntry ].box == entry.box &&
          (*nodePtr)[ currEntry ].pointer == entry.pointer )
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
6.15 Method Compute MaxNodesInInsert

*/
template <unsigned dim>
void R_TreePnJ<dim>::ComputeMaxNodesInInsert()
{
  int maxNodesInLevel, maxNodesReinforced;

  //the number of new nodes which could come into beeing inserting
  //one data entry without forced_reinsertion.
  maxNodesInInsert = header.height + 2;

  //the number of new nodes which could come into beeing inserting
  //one data entry with forced_reinsertion.
  maxNodesInReinforcedInsertion = 2;

  for (int l = 1; l <= header.height; l++)
  {
     maxNodesInLevel = (int) pow ((double) header.maxEntries, (double) l);
     maxNodesReinforced = (int) pow ((double) header.minEntries, (double) (header.height+1-l));

     if (maxNodesInLevel <= maxNodesReinforced)
       { maxNodesInReinforcedInsertion += maxNodesInLevel; }
     else
       { maxNodesInReinforcedInsertion += maxNodesReinforced; }
   }
}


/*
6.15 Method First

*/
template <unsigned dim>
bool R_TreePnJ<dim>::First (const BBox<dim>& box, R_TreeEntryPnJ<dim>& result,
                            vector <ArrayIndex>& leavesOverflowed, int replevel)
{
  //Remember that we have started a scan of the R-Tree
  scanFlag = true;

  //Init search parameters
  searchBox = box;
  path [ 0 ] = 0;
  pathEntry [ 0 ] = 0;
  reportLevel = replevel;

  //Load root node
  nodePtr = &rtree[0];
  nodePtrNo = 0;
  currLevel = 0;

  //Finish with the actual search;
  currEntry = -1;

  return Next ( result, leavesOverflowed);

}

/*
6.16 Method Next

*/
template <unsigned dim>
bool R_TreePnJ<dim>::Next (R_TreeEntryPnJ<dim>& result,
                           vector <ArrayIndex>& leavesOverflowed)
{
  //Next can be called only after a First or Next operation
  assert (scanFlag);

  bool retcode = false;

  while ( currEntry < nodePtr->EntryCount() || currLevel > 0)
  {

    if ( currEntry >= nodePtr->EntryCount() )
    {
      UpLevel();
      currEntry = pathEntry [ currLevel ];
    }
    else
    { // Search next entry / subtree in this node

    if ( nodePtr->IsInsertOverflow() )
    {
       leavesOverflowed.push_back ( nodePtrNo );
       UpLevel();
       currEntry = pathEntry [ currLevel ];
    }

    currEntry++;

    if ( currEntry < nodePtr->EntryCount() )
    {
      if ( (*nodePtr) [currEntry ].box.Intersects ( searchBox ) )
        if ( nodePtr->IsLeaf() || currLevel == reportLevel )
        { // Found an appropriate entry
            result = (*nodePtr) [ currEntry ];
            retcode = true;
            break;
        }
        else
        {  //Go down the tree
          DownLevel ( currEntry );
          currEntry = -1;
        }
      }

    }
  }  // while

  return retcode;
};

/*
6.17 Method Info

*/
template <unsigned dim>
void R_TreePnJ<dim>::Info()
{
  cout << "CurrentHeightOfTree: " << Height() << endl;
  cout << "Var maxFanOut:       " << MaxEntries() << endl << endl;

  cout << "CurrentNodesInTree:  " << NodeCount() << endl;
  cout << "MaxNodesInTree:      " << MaxNodeCount() << endl << endl;

  //collect information about nodes
  int innerNodesNo = 0;
  int fullLeavesNo = 0;
  int emptyLeavesNo = 0;

  for (int i = 0; i <= NodeCount() - 1; i++)
  {
    if  ( rtree[i].IsLeaf() )
    {
      if ( rtree[i].EntryCount() == rtree[i].MaxEntries() )
        { fullLeavesNo++; }

      if ( rtree[i].IsInsertOverflow() )
        { emptyLeavesNo++; }
    }
  else
    { innerNodesNo++; }
  }

  cout << "InnerNodes:          " << innerNodesNo << endl;

  cout << "CurrentLeaves:       " << LeavesCount() << endl;
  cout << "EstimatedLeaves:     " << EstimatedLeaves() << endl << endl;
  cout << "FullLeaves:          " << fullLeavesNo << endl;
  cout << "OverflowedLeaves:    " << emptyLeavesNo << endl << endl;

  cout << "CurrentEntriesInTree:" << EntryCount() -
                                         (MaxEntries() * emptyLeavesNo) << endl
                                                                        << endl;

  cout << "SplittingMethod:     ";
    if (do_linear_split) cout << "do_linear_split" << endl << endl;
    if (do_quadratic_split) cout << "do_quadratic_split" << endl << endl;
    if (do_axis_split) cout << "do_axis_split" << endl << endl;

  cout << "SizeOfRTreeEntry:    " << sizeof( R_TreeEntryPnJ<dim> ) << " Bytes" << endl;
  cout << "SizeOfRTreeNode:     " << rtree[0].Size() << " Bytes" << endl;
  cout << "SizeOfRTree:         " << SizeOfRTree ()
                                  <<" = "<<SizeOfRTree()/1024<<" kBytes"
                                  <<" = "<<SizeOfRTree()/1048576<<" MBytes"<<endl;
  cout << "SizeOfHeader:        " <<SizeOfRTreeHeader()<<" Bytes"<<endl<<endl;

}

template<unsigned dim>
void R_TreePnJ<dim>::DebugOutput(ArrayIndex nodeNo)
{
  assert ( (nodeNo >= -1) && (nodeNo <= NodeCount()) );

  ArrayIndex begin, end;

  if (nodeNo == -1)
    { begin = 0; end = NodeCount() - 1; }
  else
    { begin = nodeNo; end = nodeNo; };

  for (int j = begin; j <= end; j ++)
    {
     cout << endl << "Node:" << j << endl;
     (rtree[j]).Print(cout);
    }
}

#endif

