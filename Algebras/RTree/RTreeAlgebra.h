/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

{\Large \bf Anhang D: RTree-Template }

[1] Header-File of R-Tree Algebra

1996, Original code from Claudio Esperanca

October 1997, Geraldo Zimbrao made some adaptions.

July 2003, Victor Almeida.

October 2003, Victor Almeida changed the R-Tree class to be a template
on the number of dimensions.

October 2004, Herbert Schoenhammer, tested and divided in Header-File and
Implementation File. Some few corrections in SplitAlgorithms LinearSplit and
AxisSplit were done.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

February 2007, Christian Duentgen added operator for bulk loading
R-trees.

[TOC]

0 Overview

This header file implements a disk-resident representation of a R-Tree.
Setting some parameters the R-Tree-behaviour of Guttman or the R[*]-Tree
of Kriegel et al. can be selected.

The R-Tree is implemented as a template to satisfy the usage with various
dimensions. The desired dimensions are passed as a parameter to the template.

1 Defines and Includes

*/
#ifndef __RTREEPNJ_ALGEBRA_H__
#define __RTREEPNJ_ALGEBRA_H__

#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#include <iostream>
#include <stack>
#include <limits>

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

/*
2 Constants

*/

const int MAX_PATH_SIZE = 50;
/*
The maximum height of the R-Tree.

Below are a bunch of constants that will govern
some decisions that have to be made when inserting or
deleting a node into the R-tree. By setting this flags
in a particular pattern, it should be possible to obtain
assorted flavors of R-tree and R[*]-tree behaviours.

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

const int do_forced_reinsertion = 1;
/*
Checked while trying to insert an entry into a full leaf node. If set,
some of the entries of the leaf node (governed by variable
forced\_reinsertion\_percent below) will be reinserted into the tree.
Used in R[*]-trees.

*/

const int forced_reinsertion_percent = 30;
/*
A number between 0 and 100 that indicates the percentage full leaf node
entries that will be reinserted if the node overflows.
Used in R[*]-trees.

Only one of the three next flags below should be set

*/
const int do_linear_split = 0;
/*
If set, Guttman's linear split algorithm is performed.
Used in standard R-trees

*/
const int do_quadratic_split = 0;
/*
If set, Guttman's quadratic split algorithm is performed.
Used in standard R-trees

*/
const int do_axis_split = 1;
/*
If set, Krigel et al's axis split algorithm is performed.

*/

const double BULKLOAD_TOLERANCE = 4.0;
/*
Tolerance for leaf stripping in bulkload mechanism.
The tolerance specifies, which multiple of the average distance
of bounding boxes is acceptable within a single node.

Value must be >0.

*/

const double BULKLOAD_MIN_ENTRIES_FACTOR = 1.0;
/*
Specifies a multiple of MinEntries, that must be reached, before
leaf stripping is performed during a bulkload.

Value should be between 0.0 and 1.0.

*/

/*
3 Struct ~R\_TreeEntry~

*/
template<unsigned dim>
struct R_TreeEntry
{
  BBox<dim> box;
/*
If it is a leaf entry, then the bounding box spatially constains the spatial
object. If it is an internal entry, the bounding box contains all bounding
boxes of the entries of its child node.

*/

  virtual ~R_TreeEntry()
  {}
/*
The virtual destructor.

*/
  virtual void Read( char *buffer, int& offset ) = 0;
/*
Reads an entry from the buffer. Offset is increased.

*/
  virtual void Write( char *buffer, int& offset ) = 0;
/*
Writes an entry to the buffer. Offset is increased.

*/
};

/*
3 Struct ~R\_TreeInternalEntry~

This struct will store an entry inside an internal node of the R\_Tree.

*/
template<unsigned dim>
struct R_TreeInternalEntry : public R_TreeEntry<dim>
{
  SmiRecordId pointer;
/*
Points to the child node.

*/

  inline R_TreeInternalEntry() {}
/*
The simple constructor.

*/

  inline R_TreeInternalEntry( const BBox<dim>& box, SmiRecordId pointer = 0 ) :
  pointer( pointer )
  {
    this->box = box;
  }
/*
The second constructor passing a bounding box and a page.

*/
  inline R_TreeInternalEntry( const R_TreeInternalEntry<dim>& e ):
  pointer( e.pointer )
  {
    this->box = e.box;
  }
/*
The copy constructor.

*/
  inline R_TreeInternalEntry<dim>& operator=
      ( const R_TreeInternalEntry<dim>& entry )
  {
    this->box = entry.box;
    this->pointer = entry.pointer;
    return *this;
  }
/*
Redefinition of the assignement operator.

*/

  static int Size()
  {
    return sizeof( BBox<dim> ) + sizeof( SmiRecordId );
  }
/*
Returns the size of the entry in disk.

*/

  void Read( char *buffer, int& offset )
  {
    memcpy( &this->box, buffer+offset, sizeof(BBox<dim>) );
    offset += sizeof(BBox<dim>);
    memcpy( &pointer, buffer+offset, sizeof(SmiRecordId) );
    offset += sizeof(SmiRecordId);
  }
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char *buffer, int& offset )
  {
    memcpy( buffer+offset, &this->box, sizeof(BBox<dim>) );
    offset += sizeof(BBox<dim>);
    memcpy( buffer+offset, &pointer, sizeof(SmiRecordId) );
    offset += sizeof(SmiRecordId);
  }
/*
Writes an entry to the buffer. Offset is increased.

*/
};

/*
4 Struct ~R\_TreeLeafEntry~

This struct will store an entry inside a leaf node of the R\_Tree.

*/
template<unsigned dim, class Info>
struct R_TreeLeafEntry : public R_TreeEntry<dim>
{
  Info info;
/*
Stores the leaf entry information.

*/

  inline R_TreeLeafEntry() {}
/*
The simple constructor.

*/

  inline R_TreeLeafEntry( const BBox<dim>& box, const Info& info ) :
  info( info )
  {
    this->box = box;
  }
/*
The second constructor passing a bounding box and the info.

*/
  inline R_TreeLeafEntry( const R_TreeLeafEntry<dim, Info>& e ):
  info( e.info )
  {
    this->box = e.box;
  }
/*
The copy constructor.

*/
  inline R_TreeLeafEntry<dim, Info>& operator=
      ( const R_TreeLeafEntry<dim, Info>& entry )
  {
    this->box = entry.box;
    this->info = entry.info;
    return *this;
  }
/*
Redefinition of the assignement operator.

*/

  static int Size()
  {
    return sizeof( BBox<dim> ) + sizeof( Info );
  }
/*
Returns the size of the entry in disk.

*/

  void Read( char *buffer, int& offset )
  {
    memcpy( &this->box, buffer+offset, sizeof(BBox<dim>) );
    offset += sizeof(BBox<dim>);
    memcpy( &info, buffer+offset, sizeof(Info) );
    offset += sizeof(Info);
  }
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char *buffer, int& offset )
  {
    memcpy( buffer+offset, &this->box, sizeof(BBox<dim>) );
    offset += sizeof(BBox<dim>);
    memcpy( buffer+offset, &info, sizeof(Info) );
    offset += sizeof(Info);
  }
/*
Writes an entry to the buffer. Offset is increased.

*/

};

template<unsigned dim>
class IntrospectResult
{
  public:

    int level;
    long nodeId;
    BBox<dim> MBR;
    long fatherId;
    bool isLeaf;
    int minEntries;
    int maxEntries;
    int countEntries;

    IntrospectResult():
      level( -1 ),
      nodeId( -1 ),
      fatherId( -1 ),
      isLeaf( true ),
      minEntries( -1 ),
      maxEntries( -1 ),
      countEntries( -1 )
      {
        double dmin[dim], dmax[dim];
        for(unsigned int i=0; i < dim; i++)
        {
          dmin[i] = 0.0;
          dmax[i] = 0.0;
        }
        MBR = Rectangle<dim>(true, dmin, dmax);
      }

    IntrospectResult( int lev,  int node, BBox<dim> box, 
                      long father, bool leaf, 
                      int minE, int maxE, int countE ):
      level( lev ),
      nodeId( node ),
      MBR( box ),
      fatherId( father ),
      isLeaf( leaf ),
      minEntries( minE ),
      maxEntries( maxE ),
      countEntries( countE )
    {}

    virtual ~IntrospectResult()
    {}
};

/*
4 Class ~R\_TreeNode~

This is a node in the R-Tree.

*/

template<unsigned dim, class LeafInfo>
class R_TreeNode
{
  public:
    R_TreeNode( const bool leaf, const int min, const int max );
/*
The constructor.

*/

    R_TreeNode( const R_TreeNode<dim, LeafInfo>& n );
/*
The copy constructor.

*/

    ~R_TreeNode();
/*
The destructor.

*/

    static int SizeOfEmptyNode();
/*
This is a class function that returns the size in bytes of an empty node.
It will be used to calculate how many entries fit in a node, given a page size.

*/

    int Size() const;
/*
Returns the maximum size in bytes of this node, i.e., how many bytes in disk
this node would occupy at most if written with method ~Write~.

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

    R_TreeEntry<dim>& operator[] ( int index ) const
      { assert( index >= 0 && index <= maxEntries ); return *entries[ index ]; }
/*
Returns entry given by index.

*/

    BBox<dim> BoundingBox() const;
/*
Returns the bounding box of this node.

*/

    void Clear()
    {
      for( int i = 0; i < maxEntries; i++ )
      {
        delete entries[ i ];
        entries[ i ] = 0;
      }
      count = 0;
      modified = true;
    }
/*
Clears all entries.

*/

    R_TreeNode<dim, LeafInfo>& operator = ( const R_TreeNode<dim, LeafInfo>& );
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

    void Split( R_TreeNode<dim, LeafInfo>& n1, R_TreeNode<dim, LeafInfo>& n2 );
/*
Splits this node in two: ~n1~ and ~n2~, which should be empty nodes.

*/

    void UpdateBox( BBox<dim>& box, SmiRecordId pointer );
/*
Update entry corresponding to ~pointer~ to have bounding box ~box~.

*/

    void Read( SmiRecordFile& file, const SmiRecordId pointer );
    void Read( SmiRecord& record );
/*
Reads this node from an ~SmiRecordFile~ at position ~id~.

*/

    void Write( SmiRecordFile& file, const SmiRecordId pointer );
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

};


/*
4.1 The constructors

*/
template<unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo>::R_TreeNode( const bool leaf, 
                                       const int min, 
                                       const int max ) :
  leaf( leaf ),
  minEntries( min ),
  maxEntries( max ),
  count( 0 ),
  entries( new R_TreeEntry<dim>*[ max + 1 ] ),
  modified( true )
{
  for( int i = 0; i <= maxEntries; i++ )
    entries[ i ] = 0;
}

template<unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo>::R_TreeNode( const R_TreeNode<dim, LeafInfo>& node ) :
  leaf( node.leaf ),
  minEntries( node.minEntries ),
  maxEntries( node.maxEntries ),
  count( node.count ),
  entries( new R_TreeEntry<dim>*[ node.maxEntries + 1 ] ),
  modified( true )
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
  for( ; i <= node.maxEntries; i++ )
    entries[ i ] = NULL;
}

/*
4.2 The destructor

*/
template<unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo>::~R_TreeNode()
{
  for( int i = 0; i <= count; i++ )
    delete entries[ i ];
  delete []entries;
}

template<unsigned dim, class LeafInfo>
int R_TreeNode<dim, LeafInfo>::SizeOfEmptyNode()
{
  return sizeof( bool ) + // leaf
         sizeof( int );  // count
}

template<unsigned dim, class LeafInfo>
int R_TreeNode<dim, LeafInfo>::Size() const
{
  int size = SizeOfEmptyNode();

  if( leaf )
    size += R_TreeLeafEntry<dim, LeafInfo>::Size() * maxEntries;
  else
    size += R_TreeInternalEntry<dim>::Size() * maxEntries;

  return size;
}

template<unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo>& R_TreeNode<dim, LeafInfo>::operator = 
    ( const R_TreeNode<dim, LeafInfo>& node )
{
  assert( minEntries == node.minEntries && maxEntries == node.maxEntries );
  assert( count >= 0 && count <= maxEntries + 1 );

  for( int i = 0; i < node.count; i++ )
  {
    if( leaf )
      entries[ i ] = new R_TreeLeafEntry<dim, LeafInfo>
          ( (R_TreeLeafEntry<dim, LeafInfo>&)*node.entries[ i ] );
    else
      entries[ i ] = new R_TreeInternalEntry<dim>
          ( (R_TreeInternalEntry<dim>&)*node.entries[ i ] );
  }

  leaf = node.leaf;
  count = node.count;
  modified = true;

  return *this;
}

template<unsigned dim, class LeafInfo>
bool R_TreeNode<dim, LeafInfo>::Remove( int index )
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
bool R_TreeNode<dim, LeafInfo>::Insert( const R_TreeEntry<dim>& ent )
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
void R_TreeNode<dim, LeafInfo>::LinearPickSeeds( int& seed1, int& seed2 ) const
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
void R_TreeNode<dim, LeafInfo>::QuadraticPickSeeds( int& seed1, 
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
int R_TreeNode<dim, LeafInfo>::QuadraticPickNext( BBox<dim>& b1, 
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

struct SortedArrayItem
{
  int index;
  double pri;
};

int myCompare( const void* a, const void* b );

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

      qsort( a, n, sizeof( SortedArrayItem ), myCompare );
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
4.6 Method Split

*/
template<unsigned dim, class LeafInfo>
void R_TreeNode<dim, LeafInfo>::Split( R_TreeNode<dim, 
                                       LeafInfo>& n1, 
                                       R_TreeNode<dim, 
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
        n1.Insert( *entries[ sort[ i ] ] );

      for( int i = minSplitPoint + 1; i <= MaxEntries(); i++ )
        n2.Insert( *entries[ sort[ i ] ] );

      assert( n1.BoundingBox().Intersection( n2.BoundingBox() ).Area() 
              == minOverlap );

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
          for( i = 0; i < EntryCount() ; i++, notAssigned-- )
            n1.Insert( *entries[ i ] );

          count = 0;
          assert( notAssigned == 0 );
        }
        else if( n2.EntryCount() + notAssigned == n2.MinEntries() )
        { // Insert all remaining entries in n2
          for( i = 0; i < EntryCount(); ++i, notAssigned-- )
            n2.Insert( *entries[ i ] );

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
BBox<dim> R_TreeNode<dim, LeafInfo>::BoundingBox() const
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
void R_TreeNode<dim, LeafInfo>::UpdateBox( BBox<dim>& b, SmiRecordId pointer )
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
void R_TreeNode<dim, LeafInfo>::Read( SmiRecordFile& file, 
                                      const SmiRecordId pointer )
{
  SmiRecord record;
  assert( file.SelectRecord( pointer, record, SmiFile::ReadOnly ) );
  Read( record );
}

template<unsigned dim, class LeafInfo>
void R_TreeNode<dim, LeafInfo>::Read( SmiRecord& record )
{
  int offset = 0;
  char buffer[Size() + 1];
  memset( buffer, 0, Size() + 1 );

  assert( record.Read( buffer, Size(), offset ) );

  // Reads leaf, count
  memcpy( &leaf, buffer + offset, sizeof( leaf ) );
  offset += sizeof( leaf );
  memcpy( &count, buffer + offset, sizeof( count ) );
  offset += sizeof( count );

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
  modified = false;
}

template<unsigned dim, class LeafInfo>
void R_TreeNode<dim, LeafInfo>::Write( SmiRecordFile& file, 
                                       const SmiRecordId pointer )
{
  if( modified )
  {
    SmiRecord record;
    assert( file.SelectRecord( pointer, record, SmiFile::Update ) );
    Write( record );
  }
}

template<unsigned dim, class LeafInfo>
void R_TreeNode<dim, LeafInfo>::Write( SmiRecord& record )
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

    //cout << "R_TreeNode<dim, LeafInfo>::Write(): count/maxEntries = " 
    //     << count << "/" << maxEntries << "." << endl;
    assert( count <= maxEntries );

    // Now write the entry array.
    for( int i = 0; i < count; i++ )
      entries[i]->Write( buffer, offset );

    assert( record.Write( buffer, Size(), 0 ) );
    modified = false;
  }
}

/*
5 Class ~R\_Tree~

This class implements the R-Tree.

*/

template<unsigned dim, class LeafInfo>
class BulkLoadInfo
{
  public:
    R_TreeNode<dim, LeafInfo> *node[MAX_PATH_SIZE];
    bool skipLeaf; 
    int currentLevel;
    int currentHeight;
    int nodeCount;
    int entryCount;
    long levelEntryCounter[MAX_PATH_SIZE];
    double levelDistanceSum[MAX_PATH_SIZE];
    BBox<dim> levelLastBox[MAX_PATH_SIZE];

    BulkLoadInfo(const bool &leafSkipping = false) :
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

    virtual ~BulkLoadInfo()
    {
      for(int i=0; i < MAX_PATH_SIZE; i++)
        if(node[i] != NULL)
          delete node[i];
    };
};

template <unsigned dim, class LeafInfo>
class R_Tree
{
  public:

    R_Tree( const int pageSize );
/*
The first constructor. Creates an empty R-tree.

*/

    R_Tree( const SmiFileId fileId );
/*
Opens an existing R-tree.

*/

    ~R_Tree();
/*
The destructor.

*/

    inline void DeleteFile()
    {
      file.Close();
      file.Drop();
    }
/*
Deletes the file of the R-Tree.

*/

    inline SmiFileId FileId()
    {
      return file.GetFileId();
    }
/*
Returns the ~SmiFileId~ of the R-Tree database file.

*/

    inline SmiRecordId RootRecordId() const
    {
      return header.rootRecordId;
    }
/*
Returns the ~SmiRecordId~ of the root node.

*/

    inline int MinEntries( int level ) const
    {
      return level == Height() ? header.minLeafEntries 
                               : header.minInternalEntries;
    }
/*
Returns the minimum number of entries per node.

*/

    inline int MaxEntries( int level ) const
    {
      return level == Height() ? header.maxLeafEntries 
                               : header.maxInternalEntries;
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
                R_TreeLeafEntry<dim, 
                LeafInfo>& result, 
                int replevel = -1 );
    bool Next( R_TreeLeafEntry<dim, LeafInfo>& result );
/*
Sets ~result~ to the (leaf) entry corresponding to the first/next
object whose bounding box overlaps ~bx~.
Returns ~true~ if a suitable entry was found and ~false~ if not.
Setting ~replevel~ to a value != -1 forces the search to return
entries at that level of the tree regardless of whether they
are at leaf nodes or not.

*/

    R_TreeNode<dim, LeafInfo>& Root();
/*
Loads ~nodePtr~ with the root node and returns it.

*/

    bool InitializeBulkLoad(const bool &leafSkipping = false);

/*
Verifies, that the R-tree is empty. Use this before calling
~InsertBulkLoad()~.
Data structures used during bulk loading will be initialized.

*/

    void InsertBulkLoad(const R_TreeEntry<dim>& entry);

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

    bool R_Tree<dim, LeafInfo>::IntrospectFirst(IntrospectResult<dim>& result);
    bool R_Tree<dim, LeafInfo>::IntrospectNext(IntrospectResult<dim>& result);
/*
The last two methods are used to produce a sequence of node decriptions, that
can be used to inspect the R-tree structure.

*/


  private:
    SmiRecordFile file;
/*
The record file of the R-Tree.

*/

    struct Header
    {
      SmiRecordId rootRecordId;	// Root node address (Path[ 0 ]).
      int minLeafEntries;      	// min # of entries per leaf node.
      int maxLeafEntries;      	// max # of entries per leaf node.
      int minInternalEntries;  	// min # of entries per internal node.
      int maxInternalEntries;  	// max # of entries per internal node.
      int nodeCount;          	// number of nodes in this tree.
      int entryCount;      	    // number of entries in this tree.
      int height;          	    // height of the tree.

      Header() :
        rootRecordId( 0 ), minLeafEntries( 0 ), maxLeafEntries( 0 ),
        minInternalEntries( 0 ), maxInternalEntries( 0 ),
        nodeCount( 0 ), entryCount( 0 ), height( 0 )
        {}
      Header( long rootRecordId, int minEntries, int maxEntries,
              int minInternalEntries, int maxInternalEntries,
              int nodeCount, int entryCount, int nodeSize, int height ) :
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

    R_TreeNode<dim, LeafInfo> *nodePtr;
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
/*
Bounding box for first/next.

*/

    bool scanFlag;
/*
A flag that tells whether we're in the middle of a First/Next
scan of the tree.

*/

    void PutNode( const SmiRecordId address, R_TreeNode<dim, LeafInfo> **node );
    void PutNode( SmiRecord& record, R_TreeNode<dim, LeafInfo> **node );
/*
Writes the node ~node~ at file position ~address~.
Also deletes the node.

*/

    R_TreeNode<dim, LeafInfo> *GetNode( const SmiRecordId address, 
                                        const bool leaf,
                                        const int min, const int max );
    R_TreeNode<dim, LeafInfo> *GetNode( SmiRecord& record, const bool leaf,
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
    void InsertBulkLoad(R_TreeNode<dim, LeafInfo> *node, 
                        const R_TreeEntry<dim>& entry);

    bool bulkMode;

/*
true, iff in bulk loading mode

*/

    BulkLoadInfo<dim, LeafInfo> *bli;
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

};

/*
5.1 The constructors

*/
template <unsigned dim, class LeafInfo>
R_Tree<dim, LeafInfo>::R_Tree( const int pageSize ) :
  file( true, pageSize ),
  header(),
  nodePtr( NULL ),
  currLevel( -1 ),
  currEntry( -1 ),
  reportLevel( -1 ),
  searchBox( false ),
  scanFlag( false ),
  bulkMode( false ),
  bli( NULL ),
  nodeIdCounter( 0 )
{
  file.Create();

  // Calculating maxEntries e minEntries
  int nodeEmptySize = R_TreeNode<dim, LeafInfo>::SizeOfEmptyNode();
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

  nodePtr = new R_TreeNode<dim, LeafInfo>( true, 
                                           MinEntries( 0 ), 
                                           MaxEntries( 0 ) );

  // Creating a new page for the R-Tree header.
  SmiRecordId headerRecno;
  SmiRecord headerRecord;
  assert( file.AppendRecord( headerRecno, headerRecord ) );
  assert( headerRecno == 1 );

  // Creating the root node.
  SmiRecordId rootRecno;
  SmiRecord rootRecord;
  assert( file.AppendRecord( rootRecno, rootRecord ) );
  header.rootRecordId = path[ 0 ] = rootRecno;
  header.nodeCount = 1;
  nodePtr->Write( rootRecord );

  currLevel = 0;
}

template <unsigned dim, class LeafInfo>
R_Tree<dim, LeafInfo>::R_Tree( const SmiFileId fileid ) :
file( true ),
header(),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox(),
scanFlag( false ),
nodeIdCounter( 0 )
{
  file.Open( fileid );

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
R_Tree<dim, LeafInfo>::~R_Tree()
{
  if( file.IsOpen() )
  {
    if( nodePtr != NULL )
      PutNode( path[ currLevel ], &nodePtr );

    WriteHeader();
    file.Close();
  }
}

/*
5.3 Reading and writing the header

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::ReadHeader()
{
  SmiRecord record;
  assert( file.SelectRecord( (SmiRecordId)1, record, SmiFile::ReadOnly ) );
  assert( record.Read( &header, sizeof( Header ), 0 ) == sizeof( Header ) );
}

template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::WriteHeader()
{
  SmiRecord record;
  assert( file.SelectRecord( (SmiRecordId)1, record, SmiFile::Update ) );
  assert( record.Write( &header, sizeof( Header ), 0 ) == sizeof( Header ) );
}

/*
5.4 Method PutNode: Putting node to disk

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::PutNode( const SmiRecordId recno, 
                                     R_TreeNode<dim, LeafInfo> **node )
{
  assert( file.IsOpen() );
  (*node)->Write( file, recno );
  delete *node;
  *node = NULL;
}

template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::PutNode( SmiRecord& record, 
                                     R_TreeNode<dim, LeafInfo> **node )
{
  (*node)->Write( record );
  delete *node;
  *node = NULL;
}

/*
5.5 Method GetNode: Getting node from disk

*/
template <unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo> *R_Tree<dim, LeafInfo>::GetNode(
    const SmiRecordId recno, 
    const bool leaf,
    const int min, 
    const int max )
{
  assert( file.IsOpen() );
  R_TreeNode<dim, LeafInfo> *node = 
      new R_TreeNode<dim, LeafInfo>( leaf, min, max );
  node->Read( file, recno );
  return node;
}

template <unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo> *R_Tree<dim, LeafInfo>::GetNode( 
    SmiRecord& record, 
    const bool leaf,
    const int min, 
    const int max )
{
  R_TreeNode<dim, LeafInfo> *node = 
      new R_TreeNode<dim, LeafInfo>( leaf, min, max );
  node->Read( record );
  return node;
}

/*
5.6 Method BoundingBox

*/
template <unsigned dim, class LeafInfo>
BBox<dim> R_Tree<dim, LeafInfo>::BoundingBox()
  // Returns the bounding box of this R_Tree
{
  if( currLevel == 0 )
    return nodePtr->BoundingBox();
  else
  {
    R_TreeNode<dim, LeafInfo> *tmp = 
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
5.7 Method Insert

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim,LeafInfo>::Insert(const R_TreeLeafEntry<dim,LeafInfo>& entry)
{
  scanFlag = false;

  LocateBestNode( entry, Height() );
  InsertEntry( entry );
  header.entryCount++;
}

/*
5.8 Method LocateBestNode

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::LocateBestNode( const R_TreeEntry<dim>& entry, 
                                            int level )
{
  GotoLevel( 0 );

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
void R_Tree<dim, LeafInfo>::GotoLevel( int level )
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
5.10 Method DownLevel

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::DownLevel( int entryNo )
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
5.11 Method InsertEntry

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::InsertEntry( const R_TreeEntry<dim>& entry )
{
  assert( file.IsOpen() );

  if( nodePtr->Insert( entry ) )
    UpdateBox();
  else
    if( !do_forced_reinsertion || currLevel == 0 ||
        overflowFlag[ Height() - currLevel ] )
    { // Node splitting is necessary
      R_TreeNode<dim, LeafInfo> *n1 =
        new R_TreeNode<dim, LeafInfo>
            (nodePtr->IsLeaf(), MinEntries(currLevel), MaxEntries(currLevel) );
      R_TreeNode<dim, LeafInfo> *n2 =
        new R_TreeNode<dim, LeafInfo>
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
        SmiRecord *node1record = new SmiRecord();
        assert( file.AppendRecord( node1recno, *node1record ) );
        PutNode( *node1record, &n1 );
        assert(nodePtr->Insert( R_TreeInternalEntry<dim>(n1Box,node1recno)));
        delete node1record;

        BBox<dim> n2Box( n2->BoundingBox() );
        SmiRecordId node2recno;
        SmiRecord *node2record = new SmiRecord();
        assert( file.AppendRecord( node2recno, *node2record ) );
        PutNode( *node2record, &n2 );
        assert(nodePtr->Insert(R_TreeInternalEntry<dim>(n2Box,node2recno)));
        delete node2record;

        header.height += 1;
        header.nodeCount += 2;
      }
      else
      { // splitting non-root node
        SmiRecordId newNoderecno;
        SmiRecord *newNoderecord = new SmiRecord();
        assert( file.AppendRecord( newNoderecno, *newNoderecord ) );
        R_TreeInternalEntry<dim> newEntry( n2->BoundingBox(), newNoderecno );
        PutNode( *newNoderecord, &n2 );
        delete newNoderecord;

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

/*
5.12 Method UpLevel

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::UpLevel()
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
void R_Tree<dim, LeafInfo>::UpdateBox()
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
bool R_Tree<dim, LeafInfo>::FindEntry( 
    const R_TreeLeafEntry<dim, LeafInfo>& entry )
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
bool R_Tree<dim, LeafInfo>::First( const BBox<dim>& box, 
                                   R_TreeLeafEntry<dim, 
                                   LeafInfo>& result, 
                                   int replevel )
{
  // Remember that we have started a scan of the R_Tree
  scanFlag = true;

  // Init search params
  searchBox = box;
  reportLevel = replevel;

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
bool R_Tree<dim, LeafInfo>::Next( R_TreeLeafEntry<dim, LeafInfo>& result )
{
  // Next can be called only after a 'First' or a 'Next' operation
  assert( scanFlag );

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
        if( (*nodePtr)[ currEntry ].box.Intersects( searchBox ) )
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

  return retcode;
}

/*
Variants of ~First~ and ~Next~ for introspection into the R-tree.

The complete r-tree will be iterated, but only all entries on replevel 
will be returned, unless replevel == -1.

*/

template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::IntrospectFirst( IntrospectResult<dim>& result)
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
  scanFlag = true;
  currEntry = -1;
  for(int i=0; i<MAX_PATH_SIZE; i++)
  {
    pathEntry[i] = -1;
    nodeId[i]    = -1;
  }

  return true;
}

template <unsigned dim, class LeafInfo>
    bool R_Tree<dim, LeafInfo>::IntrospectNext( IntrospectResult<dim>& result )
{
  // Next can be called only after a 'IntrospectFirst' 
  // or a 'IntrospectNext' operation
  assert( scanFlag );

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
5.17 Method Remove

*/
template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::Remove( const R_TreeLeafEntry<dim, 
                                    LeafInfo>& entry )
{
  assert( file.IsOpen() );

  scanFlag = 0;

  // First locate the entry in the tree
  if( !FindEntry( entry ) )
    return false;
  else
  { // Create a list of nodes whose entries must be reinserted
    stack<int> reinsertLevelList;
    stack<R_TreeNode<dim, LeafInfo>*> reinsertNodeList;
    BBox<dim> sonBox( false );

    // remove leaf node entry
    nodePtr->Remove( currEntry );
    header.entryCount -= 1;

    while( currLevel > 0 )
    {
      int underflow = nodePtr->EntryCount() < MinEntries( currEntry );

      if( underflow )
      { // Current node has underflow. Save it for later reinsertion
        R_TreeNode<dim, LeafInfo>* nodePtrcopy = new R_TreeNode<dim, LeafInfo>
            ( *nodePtr );

        reinsertNodeList.push( nodePtrcopy );
        reinsertLevelList.push( currLevel );

        // Remove node from the tree
        nodePtr->Clear();
        assert( file.DeleteRecord( path[ currLevel ] ) );
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
      R_TreeNode<dim, LeafInfo>* tmp = reinsertNodeList.top();
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
      file.DeleteRecord( RootRecordId() );
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

/*
5.18 Method Root

*/
template <unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo>& R_Tree<dim, LeafInfo>::Root()
// Loads nodeptr with the root node
{
  GotoLevel( 0 );

  return *nodePtr;
}

template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::InitializeBulkLoad(const bool &leafSkipping)
{
  assert( NodeCount() == 1 );

  if( bulkMode || bli != NULL )
  {
    return false;
  }
  bulkMode = true;
//   bli = new BulkLoadInfo<dim, LeafInfo>(leafSkipping); 
  bli = new BulkLoadInfo<dim, LeafInfo>(true);
  return true;
};

template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::InsertBulkLoad(const R_TreeEntry<dim>& entry)
{
  if( bli->node[0] != NULL ) 
  assert(bulkMode == true);
  bli->currentLevel = 0;
  if( bli->node[0] == NULL )
  {
    bli->node[0] = new R_TreeNode<dim,LeafInfo>(true,
                                                header.minLeafEntries,
                                                header.maxLeafEntries);
    bli->nodeCount++;
  }
  InsertBulkLoad(bli->node[0], entry);
  bli->entryCount++;
  return;
}

template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::InsertBulkLoad(R_TreeNode<dim, LeafInfo> *node, 
                                           const R_TreeEntry<dim>& entry)
{
  assert( bulkMode == true );
  assert( node != NULL );

  if( !bli->levelLastBox[bli->currentLevel].IsDefined() )
  { // initialize when called for the first time
    bli->levelLastBox[bli->currentLevel] = entry.box;
  }
  double dist    = entry.box.Distance(bli->levelLastBox[bli->currentLevel]);
  bli->levelEntryCounter[bli->currentLevel]++;
  bli->levelDistanceSum[bli->currentLevel] += dist;
  bli->levelLastBox[bli->currentLevel] = entry.box;
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
                               BULKLOAD_MIN_ENTRIES_FACTOR )// too few entries
       )
    )
  {
//     cout << "  --> Inserting here!" << endl;
    bli->node[bli->currentLevel]->Insert(entry);
    return;
  } // else: node is already full (or distance to large)...

//   cout << "  --> Passing upwards..." << endl;
  //  Write node[currentLevel] to disk
  assert(file.IsOpen());
  SmiRecordId recId;
  SmiRecord *rec = new SmiRecord();
  assert(file.AppendRecord(recId, *rec));
  // Possible signature for write are:
  bli->node[bli->currentLevel]->Write(file, recId);
  //bli->node[bli->currentLevel]->Write(*rec);

  // if no father exists, create one
  if( bli->node[bli->currentLevel+1] == NULL )
  {
    assert (bli->currentHeight == bli->currentLevel);
    bli->currentHeight++; // increase height reached
    bli->node[bli->currentLevel+1] =
        new R_TreeNode<dim,LeafInfo>(false,
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
  if(bli->currentLevel == 0)
  { // create a leaf node
    bli->node[bli->currentLevel] = 
        new R_TreeNode<dim,LeafInfo>(true,
                                     header.minLeafEntries,
                                     header.maxLeafEntries);
  }
  else
  { // create an internal node
    bli->node[bli->currentLevel] = 
        new R_TreeNode<dim,LeafInfo>(false,
                                     header.minInternalEntries,
                                     header.maxInternalEntries);
  }
  bli->nodeCount++;

  // finally, insert the original entry passed as argument
  bli->node[bli->currentLevel]->Insert(entry);
  return;
};


template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::FinalizeBulkLoad()
{
  if ( bulkMode != true )
  {
    return false;
  }
  // write all nodes to disk and delete them
  bool finished = (bli->node[0]->EntryCount() == 0);
  SmiRecordId rootId;
  for(int i=0; i < MAX_PATH_SIZE; i++)
  {
    if( !finished )
    { // need to write the node
      assert( bli->node[i] != NULL );
      assert( i <= bli->currentHeight );
      assert(file.IsOpen());
      SmiRecordId recId;
      SmiRecord *rec = new SmiRecord();
      assert(file.AppendRecord(recId, *rec));
      bli->node[i]->Write(*rec);
      rootId = recId;

      if( i < bli->currentHeight )
      { // insert node into father
        assert( i+1 <= bli->currentHeight );
        assert( bli->node[i+1] != NULL );
        if(i == 0)
        {// leaf level
          assert( bli->node[i]->IsLeaf() == true );
          R_TreeLeafEntry<dim, LeafInfo> entry( bli->node[i]->BoundingBox(), 
                                                recId );
          assert( bli->node[i+1]->Insert(entry) );
        }
        else
        { // inner level
          assert( bli->node[i]->IsLeaf() == false );
          R_TreeInternalEntry<dim> entry( bli->node[i]->BoundingBox(), 
                                          recId );
          assert( bli->node[i+1]->Insert(entry) );
        }
      }
      else 
      {// ( i == bli->currentHeight): change the root
        assert( i == bli->currentHeight );

        // Verify, that rtree is empty:
        assert( NodeCount() == 1 );
        assert( EntryCount() == 0 );
        SmiRecordId newRootId =  recId;

        // Remove old root node from the tree
        file.DeleteRecord( RootRecordId() );
        delete nodePtr; 
        nodePtr = NULL;
        currLevel   = 0;
        currEntry   = -1;
        reportLevel = -1;
        searchBox   = false;
        scanFlag    = false;

        // Set new root to topmost node
        header.nodeCount    = bli->nodeCount;
        header.entryCount   = bli->entryCount;
        header.height       = bli->currentHeight;
        header.rootRecordId = newRootId;
        path[ 0 ]           = newRootId;

        WriteHeader();
        finished = true;
      }
    }
    if( bli->node[i] != NULL )
    {
      delete bli->node[i];
      bli->node[i] = NULL;
    }
  }
  delete bli;
  bli = NULL;

  // re-initialize the rtree
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

  return true;
};

struct TwoLayerLeafInfo
{
  TupleId tupleId;
  int low;
  int high;

  TwoLayerLeafInfo() {}

  TwoLayerLeafInfo( TupleId tupleId, int low, int high ):
  tupleId( tupleId ), low( low ), high( high )
  {}
};


/*
6 Template functions for the type constructors

6.1 ~Out~-function

It does not make sense to have an R-Tree as an independent value
since the record ids stored in it become obsolete as soon as
the underlying relation is deleted. Therefore this function
outputs will show only some statistics about the tree.

*/
template <unsigned dim>
ListExpr OutRTree(ListExpr typeInfo, Word value)
{
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    ListExpr bboxList, appendList;
    R_Tree<dim, TwoLayerLeafInfo> *rtree = 
        (R_Tree<dim, TwoLayerLeafInfo> *)value.addr;

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
             nl->StringAtom( "R-Tree statistics" ),
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
    R_Tree<dim, TupleId> *rtree = (R_Tree<dim, TupleId> *)value.addr;

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
             nl->StringAtom( "R-Tree statistics" ),
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
Word InRTree( ListExpr typeInfo, ListExpr value,
              int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  return SetWord(0);
}

/*
6.3 ~Create~-function

*/
template <unsigned dim>
Word CreateRTree( const ListExpr typeInfo )
{
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
    return SetWord( new R_Tree<dim, TwoLayerLeafInfo>( 4000 ) );
  else
    return SetWord( new R_Tree<dim, TupleId>( 4000 ) );
}

/*
6.4 ~Close~-function

*/
template <unsigned dim>
void CloseRTree( const ListExpr typeInfo, Word& w )
{
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    R_Tree<dim, TwoLayerLeafInfo>* rtree = (R_Tree<dim,
                                            TwoLayerLeafInfo>*)w.addr;
    delete rtree;
  }
  else
  {
    R_Tree<dim, TupleId>* rtree = (R_Tree<dim, TupleId>*)w.addr;
    delete rtree;
  }
}

/*
6.5 ~Clone~-function

Not implemented yet.

*/
template <unsigned dim>
Word CloneRTree( const ListExpr typeInfo, const Word& w )
{
  return SetWord( Address(0) );
}

/*
6.6 ~Delete~-function

*/
template <unsigned dim>
void DeleteRTree( const ListExpr typeInfo, Word& w )
{
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    R_Tree<dim, TwoLayerLeafInfo>* rtree = (R_Tree<dim,
                                            TwoLayerLeafInfo>*)w.addr;
    rtree->DeleteFile();
    delete rtree;
  }
  else
  {
    R_Tree<dim, TupleId>* rtree = (R_Tree<dim, TupleId>*)w.addr;
    rtree->DeleteFile();
    delete rtree;
  }
}

/*
6.7 ~Cast~-function

*/
template <unsigned dim>
void* CastRTree( void* addr)
{
  return ( 0 );
}

/*
6.8 ~Open~-function

*/
template <unsigned dim>
bool OpenRTree( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    R_Tree<dim, TwoLayerLeafInfo> *rtree = 
        new R_Tree<dim, TwoLayerLeafInfo>( fileid );
    value = SetWord( rtree );
  }
  else
  {
    R_Tree<dim, TupleId> *rtree = new R_Tree<dim, TupleId>( fileid );
    value = SetWord( rtree );
  }
  return true;
}

/*
6.9 ~Save~-function

*/
template <unsigned dim>
bool SaveRTree( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  SmiFileId fileId;

  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    R_Tree<dim, TwoLayerLeafInfo> *rtree = 
        (R_Tree<dim, TwoLayerLeafInfo> *)value.addr;
    fileId = rtree->FileId();
  }
  else
  {
    R_Tree<dim, TupleId> *rtree = (R_Tree<dim, TupleId> *)value.addr;
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
int SizeOfRTree()
{
  return 0;
}


#endif

