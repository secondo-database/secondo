/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of R-Tree Algebra

July 2003, Victor Almeida

[TOC]

1 Defines and Includes

*/
#include <iostream>
#include <stack>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RectangleAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

typedef Rectangle BBox;

#ifndef DOUBLE_MAX
#define DOUBLE_MAX (1.7E308)
#endif

#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };

/*
2 Constants

*/

const int MAX_PATH_SIZE = 50;
/*
The maximum height of the R-Tree.

*/
const int nDim = 2;
/*
Just a constant for the number of dimensions.

Below are a bunch of constants that will govern
some decisions that have to be made when inserting or
deleting a node into the R-tree. By setting this flags
in a particular pattern, it should be possible to obtain
assorted flavors of R-tree and R*-tree behaviours.

*/

const int minimize_leafnode_overlap = 1;
/*
Checked while choosing the node in which a new entry will be placed.
Makes the insertion algorithm behave differently when next to the
bottom-most level of the tree, in that it will try to minimize
leaf node overlap instead of minimizing area enlargement (as is done
with the upper levels).
Used in R*-trees.

*/

const int leafnode_subset_max = 32;
/*
If minimize_leafnode_overlap is set, this variable determines
how many of the leafnodes will actually be checked (Kriegel et al
recommend 32). This set is chosed amongst the leafnodes that
result in least area enlargement.

*/

const int do_forced_reinsertion = 1;
/*
Checked while trying to insert an entry into a full leaf node. If set,
some of the entries of the leaf node (governed by variable
forced_reinsertion_percent below) will be reinserted into the tree.
Used in R*-trees.

*/

const int forced_reinsertion_percent = 30;
/*
A number between 0 and 100 that indicates the percentage full leaf node
entries that will be reinserted if the node overflows.
Used in R*-trees.

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

3 Struct ~R_TreeEntry~

This struct will store an entry inside a node of the R_Tree.

*/
struct R_TreeEntry
{
  BBox box;
/*
If it is a leaf entry, then the bounding box spatially constains the spatial
object. If it is an internal entry, the bounding box contains all bounding
boxes of the entries of its child node.

*/
  SmiRecordId pointer;
/*
Points to an ~SmiRecord~ in a file. If it is a leaf entry, this is the record
where the spatial object is stored, otherwise this is the pointer to its
child node.

*/

  R_TreeEntry() {}
/*
The simple constructor.

*/

  R_TreeEntry( const BBox& box, long pointer = 0 ) :
     box( box ), pointer( pointer )
    {}
/*
The second constructor passing a bounding box and a page.

*/
  friend ostream& operator<< ( ostream&, const R_TreeEntry& );
/*
Prints this entry (for debugging purposes).

*/
};

ostream& operator<< ( ostream& o, const R_TreeEntry& e )
{
  o << "Pointer: " << e.pointer << " BBox: " << e.box;

  return o;
}

/*
4 Class R_TreeNode

This is a node in the R-Tree.

*/
class R_TreeNode
{
  public:
    R_TreeNode( const bool leaf, const int min, const int max );
/*
The constructor.

*/

    R_TreeNode( const R_TreeNode& n );
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

    R_TreeEntry& operator[] ( int index ) const
      { assert( index >= 0 && index <= maxEntries ); return entry[ index ]; }
/*
Returns entry given by index.

*/

    BBox BoundingBox() const;
/*
Returns the bounding box of this node.

*/

    void Clear()
      { leaf = false; count = 0; modified = true; }
/*
Clears all entries.

*/

    R_TreeNode& operator = ( const R_TreeNode& );
/*
Assignment operator between nodes.

*/

    bool Remove( int );
/*
Removes the given entry from the node. Returns true if successful
or false if underflow (The entry is deleted regardless).

*/

    bool Insert( const R_TreeEntry& e );
/*
Adds ~e~ to this node if possible. Returns ~true~ if successful,
i.e., if there is enough room in the node,  or ~false~ if the insertion
caused an overflow. In the latter case, the entry is inserted,
but the node should be split by whoever called the insert method.

*/

    void Split( R_TreeNode& n1, R_TreeNode& n2 );
/*
Splits this node in two: ~n1~ and ~n2~, which should be empty nodes.

*/

    void UpdateBox( BBox& box, SmiRecordId pointer );
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

    friend ostream& operator<< ( ostream&, const R_TreeNode& );
/*
Prints this node (for debugging purposes).

*/

  private:
    bool leaf;
/*
Flag that tells whether this is a leaf node

*/

    int minEntries;
/*
Min # of entries per node.

*/

    int maxEntries;
/*
Max # of entries per node.

*/

    int count;
/*
Number of entries in this node.

*/

    R_TreeEntry* const entry;
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

    int QuadraticPickNext( BBox& b1, BBox& b2 ) const;
/*
Returns the entry position that should be assigned next to one of the
two groups with bounding boxes ~b1~ and ~b2~, respectively.
(Algorithm ~PickNext~ of Guttman)

*/
};

R_TreeNode::R_TreeNode( const bool leaf, const int min, const int max ) :
leaf( leaf ),
minEntries( min ),
maxEntries( max ),
count( 0 ),
entry( new R_TreeEntry[ max + 1 ] ),
modified( true )
{
}

R_TreeNode::R_TreeNode( const R_TreeNode& node ) :
leaf( node.leaf ),
minEntries( node.minEntries ),
maxEntries( node.maxEntries ),
count( node.count ),
entry( new R_TreeEntry[ node.maxEntries + 1 ] ),
modified( true )
{
  for( int i = 0; i < node.count; i++ )
    entry[ i ] = node.entry[ i ];
}

R_TreeNode::~R_TreeNode()
{
  delete []entry;
}

int R_TreeNode::SizeOfEmptyNode()
{
  return sizeof( bool ) + // leaf
         sizeof( int );  // count
}

int R_TreeNode::Size() const
{
  int size = sizeof( leaf ) + sizeof( count );

  size += sizeof( R_TreeEntry ) * MaxEntries();

  return size;
}

R_TreeNode& R_TreeNode::operator = ( const R_TreeNode& node )
{
  assert( minEntries == node.minEntries && maxEntries == node.maxEntries );
  assert( count >= 0 && count <= maxEntries + 1 );

  for( int i = 0; i < node.count; i++ )
    entry[ i ] = node.entry[ i ];

  leaf = node.leaf;
  count = node.count;
  modified = true;

  return *this;
}

bool R_TreeNode::Remove( int index )
{
  assert( index >= 0 && index < count );

  entry[ index ] = entry[ count - 1 ];
  count -= 1;

  modified = true;
  return count >= minEntries;
}

bool R_TreeNode::Insert( const R_TreeEntry& ent )
{
  assert( count <= maxEntries );
  entry[ count++ ] = ent;
  modified = true;
  return count <= maxEntries;
}

void R_TreeNode::LinearPickSeeds( int& seed1, int& seed2 ) const
{
  assert( count == maxEntries + 1 );
    // This should be called only if the node has an overflow

  double maxMinVal[ 2 ] = { -DOUBLE_MAX, -DOUBLE_MAX };
  double minMaxVal[ 2 ] = { DOUBLE_MAX, DOUBLE_MAX };
  double minVal[ 2 ] = { DOUBLE_MAX, DOUBLE_MAX };
  double maxVal[ 2 ] = { -DOUBLE_MAX, -DOUBLE_MAX };
  double sep[ 2 ];
  double maxSep = -DOUBLE_MAX;
  int maxMinNode[ 2 ] = { -1, -1 };
  int minMaxNode[ 2 ] = { -1, -1 };
  int bestD = -1;

  for( int i = 0; i < count; i++ )
  {
    for( int d = 0; d < 2; d++ )
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

  for( int d = 0; d < 2; d++ )
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
  assert( seed1 != seed2 );
}

void R_TreeNode::QuadraticPickSeeds( int& seed1, int& seed2 ) const
{
  assert( count == maxEntries + 1 );
    // This should be called only if the node has an overflow

  double bestWaste = -DOUBLE_MAX;
  double *area = new double[ maxEntries + 1 ];
    // Compute areas just once

  for( int i = 0; i < count; i++ )
  {
    area[ i ] = entry[ i ].box.Area();

    for( int j = 0; j < i; ++j )
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

int R_TreeNode::QuadraticPickNext( BBox& b1, BBox& b2 ) const
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

    void sort();
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
}

int myCompare( const void* a, const void* b )
{
  if( ((SortedArrayItem *) a)->pri < ((SortedArrayItem *) b)->pri )
    return -1;
  else if( ((SortedArrayItem *) a)->pri > ((SortedArrayItem *) b)->pri )
    return 1;
  else
    return 0;
}

void SortedArray::sort()
{
  assert( i == 0 && n > 0 && !sorted );

  qsort( a, n, sizeof( SortedArrayItem ), myCompare );
  sorted = 1;
}

void R_TreeNode::Split( R_TreeNode& n1, R_TreeNode& n2 )
// Splits this node in two: n1 and n2, which should be empty nodes.
{
  modified = true;

  assert( count == maxEntries + 1 );
    // Make sure this node is ready to be split

  assert( n1.EntryCount() == 0 && n2.EntryCount() == 0  );
  assert( n1.MinEntries() == minEntries && n1.MaxEntries() == maxEntries );
  assert( n2.MinEntries() == minEntries && n2.MaxEntries() == maxEntries );
    // Make sure n1 and n2 are ok

  if( do_axis_split )
  { // Do R*-Tree style split
    int *sortedEntry[ 2*nDim ] = { NULL }; // Arrays of sorted entries
    struct StatStruct
    {
      double margin;
      double overlap;
      double area;
    } *stat = new StatStruct[ 2*nDim*(maxEntries + 2 - 2*minEntries) ],
      *pstat = stat; // Array of distribution statistics
    int dim;
    double minMarginSum = DOUBLE_MAX;
    int minMarginAxis = -1;

    for( dim = 0; dim < nDim; dim++ )
    { // Compute sorted lists. Sort entry numbers by minimum value of axis 'dim'.
      int* psort = sortedEntry[ 2*dim ] = new int[ maxEntries + 1 ];
      SortedArray sort( maxEntries + 1 );
      int i;

      for( i = 0; i <= maxEntries; i++ )
        sort.push( i, entry[ i ].box.MinD( dim ) );

      for( i = 0; i <= maxEntries; i++ )
        *psort++ = sort.pop();

      assert( sort.empty() );

      // Sort entry numbers by maximum value of axis 'dim'
      psort = sortedEntry[ 2*dim + 1 ] = new int[ maxEntries + 1 ];
      for( i = 0; i <= maxEntries; i++ )
        sort.push( i, entry[ i ].box.MaxD( dim ) );

      for( i = 0; i <= maxEntries; i++ )
        *psort++ = sort.pop();

      assert( sort.empty() );
    }

    // Compute statistics for the various distributions
    for( dim = 0; dim < nDim; dim++ )
    { // Sum margins over all distributions correspondig to one axis
      double marginSum = 0.0;

      for( int minMax = 0; minMax < 2; minMax++ )
      { // psort points to one of the sorted arrays of entries
        int* psort = sortedEntry[ 2*dim + minMax ];
        // Start by computing the cummulative bounding boxes of the
        // 'maxEntries-minEntries+1' entries of each end of the scale
        BBox *b1 = new BBox[ maxEntries + 1 ];
        BBox *b2 = new BBox[ maxEntries + 1 ];

        b1[ 0 ] = entry[ psort[ 0 ] ].box;
        b2[ 0 ] = entry[ psort[ maxEntries ] ].box;

        for( int i = 1; i <= maxEntries; i++ )
        {
          b1[ i ] = b1[ i - 1 ].Union( entry[ psort[ i ] ].box );
          b2[ i ] = b2[ i - 1 ].Union( entry[ psort[ maxEntries - i ] ].box );
        }

        // Now compute the statistics for the
        // maxEntries - 2*minEntries + 2 distributions
        for( int splitPoint = minEntries - 1;
             splitPoint <= maxEntries - minEntries;
             splitPoint++ )
        {
          BBox& box1 = b1[ splitPoint ];
          BBox& box2 = b2[ maxEntries - splitPoint - 1 ];

          pstat->margin = box1.Perimeter() + box2.Perimeter();
          pstat->overlap = box1.Intersection( box2 ).Area();
          pstat->area = box1.Area() + box2.Area();
          marginSum += pstat->margin;
          pstat += 1;

          assert( pstat - stat <= 2*nDim*(maxEntries + 2 - 2*minEntries) );
        }

        delete [] b2;
        delete [] b1;
      }

      if( marginSum < minMarginSum )
      {
        minMarginSum = marginSum;
        minMarginAxis = dim;
      }
    }
    assert( pstat - stat == 2*nDim*(maxEntries + 2 - 2*minEntries));

    // At this point we have in minMarginAxis the axis on which we will
    // split. Choose the distribution with  minimum overlap,
    // breaking ties by choosing the distribution with minimum area
    {
      double minOverlap = DOUBLE_MAX;
      double minArea = DOUBLE_MAX;
      int minSplitPoint = -1;
      int *sort = 0;
      int dim = minMarginAxis;

      pstat = &stat[ 2*dim*(maxEntries + 2 - 2*minEntries) ];
      for( int minMax = 0; minMax < 2; minMax++ )
      {
        int *psort = sortedEntry[ 2*dim + minMax ];
        int splitPoint;

        for( splitPoint = minEntries - 1;
             splitPoint <= maxEntries-minEntries;
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
      assert( pstat - stat == (dim + 1 )*2*(maxEntries + 2 - 2*minEntries) );

      // Picked distribution; now put the corresponding entries in the
      // two split blocks
      for( int i = 0; i <= minSplitPoint; i++ )
        n1.Insert( entry[ sort[ i ] ] );

      for( int i = minSplitPoint + 1; i <= maxEntries; i++ )
        n2.Insert( entry[ sort[ i ] ] );

      assert( n1.BoundingBox().Intersection( n2.BoundingBox() ).Area() == minOverlap );

      // Deallocate the sortedEntry arrays
      for( int i = 0; i < 2*nDim; i++)
        delete sortedEntry[ i ];

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
    BBox box1 = entry[ seed1 ].box;
    BBox box2 = entry[ seed2 ].box;
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
      int notAssigned = count;

      assert( notAssigned == maxEntries - 1 );
      while( notAssigned > 0 )
      {
        if( n1.EntryCount() + notAssigned == n1.MinEntries() )
        { // Insert all remaining entries in n1
          for( i = 0; i < count ; i++, notAssigned-- )
            n1.Insert( entry[ i ] );

          count = 0;
          assert( notAssigned == 0 );
        }
        else if( n2.EntryCount() + notAssigned == n2.MinEntries() )
        { // Insert all remaining entries in n2
          for( i = 0; i < count; ++i, notAssigned-- )
            n2.Insert( entry[ i ] );

          count = 0;
          assert( notAssigned == 0 );
        }
        else
        {
          if( do_quadratic_split )
            i = QuadraticPickNext( box1, box2 );
          else
          {
            assert( do_linear_split );
            i = 0;
          }
          BBox union1 = box1.Union( entry[ i ].box );
          BBox union2 = box2.Union( entry[ i ].box );

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
      assert( count == 0 );
    }
  }

  assert( n1.EntryCount() + n2.EntryCount() == maxEntries + 1 );
  assert( n1.EntryCount() >= minEntries && n1.EntryCount() <= maxEntries );
  assert( n2.EntryCount() >= minEntries && n2.EntryCount() <= maxEntries );
}

BBox R_TreeNode::BoundingBox() const
{
  if( count == 0 )
    return BBox( false );
  else
  {
    BBox result = entry[ 0 ].box;
    int i;

    for( i = 1; i < count; i++ )
      result = result.Union( entry[ i ].box );

    return result;
  }
}

void R_TreeNode::UpdateBox( BBox& b, SmiRecordId pointer )
{
  modified = true;

  for( int i = 0; i < count; i++ )
    if( entry[ i ].pointer == pointer )
    {
      entry[ i ].box = b;

      return;
    }

  // Should never reach this point
  assert( 0 );
}

void R_TreeNode::Read( SmiRecordFile& file, const SmiRecordId pointer )
{
  SmiRecord record;
  assert( file.SelectRecord( pointer, record, SmiFile::ReadOnly ) );
  Read( record );
}

void R_TreeNode::Read( SmiRecord& record )
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
    R_TreeEntry *e = new ((void *)(buffer + offset)) R_TreeEntry;
    entry[i] = *e;
    offset += sizeof( R_TreeEntry );
  }

  modified = false;
}

void R_TreeNode::Write( SmiRecordFile& file, const SmiRecordId pointer )
{
  if( modified )
  {
    SmiRecord record;
    assert( file.SelectRecord( pointer, record, SmiFile::Update ) );
    Write( record );
  }
}

void R_TreeNode::Write( SmiRecord& record )
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

    assert( count <= maxEntries );

    // Now read the entry array.
    memcpy( buffer + offset, entry, count * sizeof( R_TreeEntry ) );

    assert( record.Write( buffer, Size(), 0 ) );
    modified = false;
  }
}

ostream& operator<<( ostream& o, const R_TreeNode& nod )
{
   if( nod.IsLeaf() )
     o << "Leaf Node" << endl;
   else
     o << "Internal Node" << endl;

   o << "Entrycount:" << nod.EntryCount() << endl;

   for(int i = 0; i < nod.EntryCount(); i++ )
     o << i << ":" << nod[ i ] << endl;

   return o;
}

/*
5 Class R_Tree

This class implements the R-Tree.

*/

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

    void DeleteFile()
      { file.Close(); file.Drop(); }
/*
Deletes the file of the R-Tree.

*/

    SmiFileId FileId()
      { return file.GetFileId(); }
/*
Returns the ~SmiFileId~ of the R-Tree database file.

*/

    SmiRecordId RootRecordId() const
      { return header.rootRecordId; }
/*
Returns the ~SmiRecordId~ of the root node.

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

    int Height() const
      { return header.height; }
/*
Returns the height of this tree.

*/

    BBox BoundingBox();
/*
Returns the bounding box of this rtree.

*/

    void Insert( const R_TreeEntry& );
/*
Inserts the given entry somewhere in the tree.

*/

    bool Remove( const R_TreeEntry& );
/*
Deletes the given entry from the tree. Returns ~true~ if entry found
and successfully deleted, and ~false~ otherwise.

*/

    bool First( const BBox& bx, R_TreeEntry& result, int replevel = -1 );
    bool Next( R_TreeEntry& result );
/*
Sets ~result~ to the (leaf) entry corresponding to the first/next
object whose bounding box overlaps ~bx~.
Returns ~true~ if a suitable entry was found and ~false~ if not.
Setting ~replevel~ to a value != -1 forces the search to return
entries at that level of the tree regardless of whether they
are at leaf nodes or not.

*/

    R_TreeNode& Root();
/*
Loads ~nodePtr with the root node and returns it.

*/

    friend ostream& operator << ( ostream&, R_Tree& );
/*
Dumps rtree onto output stream (for debugging purposes).

*/

  private:
    SmiRecordFile file;
/*
The record file of the R-Tree.

*/

    struct Header
    {
      SmiRecordId rootRecordId;	// Root node address (Path[ 0 ]).
      int minEntries;      	// min # of entries per node.
      int maxEntries;      	// max # of entries per node.
      int nodeCount;       	// number of nodes in this tree.
      int entryCount;      	// number of entries in this tree.
      int height;          	// height of the tree.

      Header() :
        rootRecordId( 0 ), minEntries( 0 ), maxEntries( 0 ),
        nodeCount( 0 ), entryCount( 0 ), height( 0 )
        {}
      Header( long rootRecordId, int minEntries, int maxEntries, int nodeCount,
              int entryCount, int nodeSize, int height ) :
        rootRecordId( rootRecordId ), minEntries( minEntries ), maxEntries( maxEntries ),
        nodeCount( nodeCount ), entryCount( entryCount ), height( height )
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

    R_TreeNode *nodePtr;
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

    BBox searchBox;
/*
Bounding box for first/next.

*/

    bool scanFlag;
/*
A flag that tells whether we're in the middle of a First/Next
scan of the tree.

*/

    void PutNode( const SmiRecordId address, R_TreeNode **node );
    void PutNode( SmiRecord& record, R_TreeNode **node );
/*
Writes the node ~node~ at file position ~address~.
Also deletes the node.

*/

    R_TreeNode *GetNode( const SmiRecordId address, const bool leaf, const int min, const int max );
    R_TreeNode *GetNode( SmiRecord& record, const bool leaf, const int min, const int max );
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

    void InsertEntry( const R_TreeEntry& );
/*
Inserts given entry in current node.

*/

    void LocateBestNode( const R_TreeEntry& ent, int level );
/*
Locates the "best" node of level ~level~ to insert ~ent~.

*/

    bool FindEntry( const R_TreeEntry& ent );
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
};


R_Tree::R_Tree( const int pageSize ) :
file( true, pageSize ),
header(),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox( false ),
scanFlag( false )
{
  file.Create();

  // Calculating maxEntries e minEntries
  int nodeEmptySize = R_TreeNode::SizeOfEmptyNode();
  int entrySize = sizeof( R_TreeEntry );
  int max = ( pageSize - nodeEmptySize ) / entrySize;

  header.maxEntries = max;
  header.minEntries = (int)(max * 0.4);

  assert( MaxEntries() >= 2*MinEntries() && MinEntries() > 0 );


  // initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
    overflowFlag[ i ] = 0;

  nodePtr = new R_TreeNode( true, MinEntries(), MaxEntries() );

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

R_Tree::R_Tree( const SmiFileId fileid ) :
file( true ),
header(),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox(),
scanFlag( false )
{
  file.Open( fileid );

  // initialize overflowflag array
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
    overflowFlag[ i ] = 0;

  ReadHeader();
  assert( MaxEntries() >= 2*MinEntries() && MinEntries() > 0 );

  currLevel = 0;

  nodePtr = GetNode( RootRecordId(), currLevel == Height(), MinEntries(), MaxEntries() );
  path[ 0 ] = header.rootRecordId;
}

R_Tree::~R_Tree()
{
  if( file.IsOpen() )
  {
    if( nodePtr != NULL )
      PutNode( path[ currLevel ], &nodePtr );

    WriteHeader();
    file.Close();
  }
}

void R_Tree::ReadHeader()
{
  SmiRecord record;
  assert( file.SelectRecord( (SmiRecordId)1, record, SmiFile::ReadOnly ) );
  assert( record.Read( &header, sizeof( Header ), 0 ) == sizeof( Header ) );
}

void R_Tree::WriteHeader()
{
  SmiRecord record;
  assert( file.SelectRecord( (SmiRecordId)1, record, SmiFile::Update ) );
  assert( record.Write( &header, sizeof( Header ), 0 ) == sizeof( Header ) );
}

void R_Tree::PutNode( const SmiRecordId recno, R_TreeNode **node )
{
  assert( file.IsOpen() );
  (*node)->Write( file, recno );
  delete *node;
  *node = NULL;
}

void R_Tree::PutNode( SmiRecord& record, R_TreeNode **node )
{
  (*node)->Write( record );
  delete *node;
  *node = NULL;
}

R_TreeNode *R_Tree::GetNode( const SmiRecordId recno, const bool leaf, const int min, const int max )
{
  assert( file.IsOpen() );
  R_TreeNode *node = new R_TreeNode( leaf, min, max );
  node->Read( file, recno );
  return node;
}

R_TreeNode *R_Tree::GetNode( SmiRecord& record, const bool leaf, const int min, const int max )
{
  R_TreeNode *node = new R_TreeNode( leaf, min, max );
  node->Read( record );
  return node;
}

BBox R_Tree::BoundingBox()
  // Returns the bounding box of this R_Tree
{
  if( currLevel == 0 )
    return nodePtr->BoundingBox();
  else
  {
    R_TreeNode *tmp = GetNode( RootRecordId(), 0, MinEntries(), MaxEntries() );
    BBox result = tmp->BoundingBox();
    delete tmp;

    return result;
  }
}

void R_Tree::Insert( const R_TreeEntry& entry )
{
  scanFlag = false;

  LocateBestNode( entry, Height() );
  InsertEntry( entry );
  header.entryCount++;
}

void R_Tree::LocateBestNode( const R_TreeEntry& entry, int level )
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
      SortedArray enlargeList( MaxEntries() + 1 );
      int i, j, k;

      for( i = 0; i < nodePtr->EntryCount(); i++ )
      {
        R_TreeEntry &son = (*nodePtr)[ i ];
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
          BBox boxBefore = (*nodePtr)[ i ].box;
          BBox boxAfter = boxBefore.Union( entry.box );

          for( j = 0; j < nodePtr->EntryCount(); ++j )
            if( j == i )
              continue;
            else
            {
              R_TreeEntry &son = (*nodePtr)[ j ];
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
        R_TreeEntry &son = (*nodePtr)[ i ];
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

void R_Tree::GotoLevel( int level )
{
  if( currLevel == level )
  {
    if( nodePtr == NULL )
      nodePtr = GetNode( path[ currLevel ], Height() == level, MinEntries(), MaxEntries() );
  }
  else
  {
    assert( level >= 0 && level <= Height() );

    if( nodePtr != NULL )
      PutNode( path[ currLevel ], &nodePtr );

    currLevel = level;
    nodePtr = GetNode( path[ currLevel ], Height() == level, MinEntries(), MaxEntries() );
  }
}

void R_Tree::DownLevel( int entryNo )
{
  assert( currLevel != Height() );
  assert( nodePtr != 0 );
  assert( entryNo >= 0 && entryNo < nodePtr->EntryCount() );

  pathEntry[ currLevel ] = entryNo;
  path[ currLevel+1 ] = (*nodePtr)[ entryNo ].pointer;
  PutNode( path[ currLevel ], &nodePtr );
  currLevel += 1;
  nodePtr = GetNode( path[ currLevel ], Height() == currLevel, MinEntries(), MaxEntries() );
}

void R_Tree::InsertEntry( const R_TreeEntry& entry )
{
  assert( file.IsOpen() );

  if( nodePtr->Insert( entry ) )
    UpdateBox();
  else
    if( !do_forced_reinsertion || currLevel == 0 ||
        overflowFlag[ Height() - currLevel ] )
    { // Node splitting is necessary
      R_TreeNode *n1 = new R_TreeNode( nodePtr->IsLeaf(), MinEntries(), MaxEntries() );
      R_TreeNode *n2 = new R_TreeNode( nodePtr->IsLeaf(), MinEntries(), MaxEntries() );

      nodePtr->Split( *n1, *n2 );

      // Write split nodes and update parent
      if( currLevel == 0)
      { // splitting root node

        nodePtr->Clear();

        BBox n1Box( n1->BoundingBox() );
        SmiRecordId node1recno;
        SmiRecord *node1record = new SmiRecord();
        assert( file.AppendRecord( node1recno, *node1record ) );
        PutNode( *node1record, &n1 );
        assert( nodePtr->Insert( R_TreeEntry( n1Box, node1recno ) ) );
        delete node1record;

        BBox n2Box( n2->BoundingBox() );
        SmiRecordId node2recno;
        SmiRecord *node2record = new SmiRecord();
        assert( file.AppendRecord( node2recno, *node2record ) );
        PutNode( *node2record, &n2 );
        assert( nodePtr->Insert( R_TreeEntry( n2Box, node2recno ) ) );
        delete node2record;

        header.height += 1;
        header.nodeCount += 2;
      }
      else
      { // splitting non-root node
        SmiRecordId newNoderecno;
        SmiRecord *newNoderecord = new SmiRecord();
        assert( file.AppendRecord( newNoderecno, *newNoderecord ) );
        R_TreeEntry newEntry( n2->BoundingBox(), newNoderecno );
        PutNode( *newNoderecord, &n2 );
        delete newNoderecord;

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
      int reinsertLevel = Height() - currLevel;

      // Avoid reinserting at this level
      overflowFlag[ reinsertLevel ] = 1;

      // Compute the center of the node
      BBox nodeBox = nodePtr->BoundingBox();
      double nodeCenter[ nDim ];

      for( int i = 0; i < nDim; i++ )
        nodeCenter[ i ] = (nodeBox.MinD( i ) + nodeBox.MaxD( i ))/2;

      // Make list sorted by distance from the center of each
      // entry bounding box to the center of the bounding box
      // of all entries.
      // NOTE: We use CHESSBOARD metric for the distance
      SortedArray distSort( MaxEntries() + 1 );

      for( int i = 0; i < nodePtr->EntryCount(); i++ )
      {
        double entryDistance = 0.0;

        for( int j = 0; j < nDim; j++ )
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
        R_TreeEntry *tmp = new R_TreeEntry[ MaxEntries() + 1 ];
        int *keepFlag = new int[ MaxEntries() + 1 ];
        int deleteCount, n = 0;

        for( int i = 0; i <= MaxEntries(); i++ )
          keepFlag[ i ] = 0;

        deleteCount = (forced_reinsertion_percent * MaxEntries()) / 100;

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
          R_TreeEntry reinsertEntry = tmp[ entryNo ];

          assert( !keepFlag[ entryNo ] );

          LocateBestNode( reinsertEntry, Height() - reinsertLevel );
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

void R_Tree::UpLevel()
{
  assert( currLevel > 0 );

  if( nodePtr != NULL )
    PutNode( path[ currLevel ], &nodePtr );

  currLevel -= 1;
  nodePtr = GetNode( path[ currLevel ], Height() == currLevel, MinEntries(), MaxEntries() );
}

void R_Tree::UpdateBox()
{
  // Save where we were before
  int formerlevel = currLevel;

  for( int l = currLevel; l > 0; l-- )
  {
    // Compute bounding box of child
    BBox box = nodePtr->BoundingBox();

    // Update 'father' node
    UpLevel();
    nodePtr->UpdateBox( box, path[ l ] );
  }

  // Return to where we were before
  GotoLevel( formerlevel );
}

bool R_Tree::FindEntry( const R_TreeEntry& entry )
{
  // First see if the current entry is the one that is being sought,
  // as is the case with many searches followed by Delete
  if( currLevel == Height() &&
      currEntry >= 0 && currEntry < MaxEntries() &&
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
      if( currLevel == Height() ) // This is a leaf node. Search all entries
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

bool R_Tree::First( const BBox& box, R_TreeEntry& result, int replevel )
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

bool R_Tree::Next( R_TreeEntry& result )
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
            result = (*nodePtr)[ currEntry ];
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

ostream& operator<<( ostream& o, R_Tree& rt )
// Dump R_Tree onto a text stream( for debugging)
{
  rt.GotoLevel( 0 );
  rt.currEntry = 0;

  // Search the tree until no more subtrees are found in the root
  while( rt.currEntry < (*rt.nodePtr).EntryCount() || rt.currLevel > 0 )
    if( rt.currEntry >= ( *rt.nodePtr).EntryCount() )
    { // All entries or subtrees of the current node were
      // examined. Go up on the tree

      rt.UpLevel();
      rt.currEntry = rt.pathEntry[ rt.currLevel ] + 1;
    }
    else
    { // Try another subtree or entry
      if( rt.currLevel == rt.Height() )
      {
        o << "Leaf node " << rt.path[ rt.currLevel ]
          << " at level " << rt.currLevel << endl;

        // This is a leaf node. Search all entries
        while( rt.currEntry < (*rt.nodePtr).EntryCount() )
        {
          o <<( *rt.nodePtr)[ rt.currEntry ] << endl;
          rt.currEntry++;
        }
      }
      else
      {
        if( rt.currEntry == 0 )
        {
          int i;

          o << "Internal node " << rt.path[ rt.currLevel ]
            << " at level " << rt.currLevel << endl;

          for( i = 0; i <( *rt.nodePtr).EntryCount(); i++ )
            o <<( *rt.nodePtr)[ i ] << endl;
        }

        rt.DownLevel( rt.currEntry );
        rt.currEntry = 0;
      }
    }

  return o;
}

bool R_Tree::Remove( const R_TreeEntry& entry )
{
  assert( file.IsOpen() );

  scanFlag = 0;

  // First locate the entry in the tree
  if( !FindEntry( entry ) )
    return false;
  else
  { // Create a list of nodes whose entries must be reinserted
    stack<int> reinsertLevelList;
    stack<R_TreeNode*> reinsertNodeList;
    BBox sonBox( false );

    // remove leaf node entry
    nodePtr->Remove( currEntry );
    header.entryCount -= 1;

    while( currLevel > 0 )
    {
      int underflow = nodePtr->EntryCount() < MinEntries();

      if( underflow )
      { // Current node has underflow. Save it for later reinsertion
        R_TreeNode* nodePtrcopy = new R_TreeNode( *nodePtr );

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
      R_TreeNode* tmp = reinsertNodeList.top();
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
      long newRoot = (*nodePtr)[ 0 ].pointer;

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

R_TreeNode& R_Tree::Root()
// Loads nodeptr with the root node
{
  GotoLevel( 0 );

  return *nodePtr;
}

/*
6 Type constructor ~rtree~

6.1 Type property of type constructor ~rtree~

*/
ListExpr RTreeProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"<relation> creatertree [<attrname>] where <attrname> is the key");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                             nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                             nl->StringAtom("(let myrtree = countries createbtree [boundary])"))));
}

/*
6.2 ~Out~-function of type constructor ~rtree~

An rtree does not make sense as an independent value since
the record ids stored in it become obsolete as soon as
the underlying relation is deleted. Therefore this function
outputs will show only some statistics about the tree.

*/
ListExpr OutRTree(ListExpr typeInfo, Word value)
{
  R_Tree *rtree = (R_Tree *)value.addr;
  return nl->SixElemList(
           nl->StringAtom( "R-Tree statistics" ),
           nl->ThreeElemList( nl->StringAtom( "Entries (min/max)" ),
                              nl->IntAtom( rtree->MinEntries() ),
                              nl->IntAtom( rtree->MaxEntries() ) ),
           nl->TwoElemList( nl->StringAtom( "Height" ),
                            nl->IntAtom( rtree->Height() ) ),
           nl->TwoElemList( nl->StringAtom( "# of (leaf) entries" ),
                            nl->IntAtom( rtree->EntryCount() ) ),
           nl->TwoElemList( nl->StringAtom( "# of nodes" ),
                            nl->IntAtom( rtree->NodeCount() ) ),
           nl->FiveElemList( nl->StringAtom( "Bounding Box" ),
                              nl->RealAtom( rtree->BoundingBox().MinD( 0 ) ),
                              nl->RealAtom( rtree->BoundingBox().MinD( 1 ) ),
                              nl->RealAtom( rtree->BoundingBox().MaxD( 0 ) ),
                              nl->RealAtom( rtree->BoundingBox().MaxD( 1 ) ) ) );
}

/*
6.3 ~In~-function of type constructor ~rtree~

Reading an rtree from a list does not make sense because an rtree
is not an independent value. Therefore calling this function leads
to program abort.

*/
Word InRTree(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = false;
  return SetWord(0);
}

/*
6.4 ~Create~-function of type constructor ~rtree~

*/
Word CreateRTree(const ListExpr typeInfo)
{
//  cout << "Create RTree" << endl;
  return SetWord( new R_Tree( 4000 ) );
}

/*
6.5 ~Close~-function of type constructor ~rtree~

*/
void CloseRTree(Word& w)
{
//  cout << "Close RTree" << endl;
  R_Tree* rtree = (R_Tree*)w.addr;
  delete rtree;
}

/*
6.6 ~Clone~-function of type constructor ~rtree~

Not implemented yet.

*/
Word CloneRTree(const Word& w)
{
  return SetWord( Address(0) );
}

/*
6.7 ~Delete~-function of type constructor ~rtree~

*/
void DeleteRTree(Word& w)
{
//  cout << "Delete RTree" << endl;
  R_Tree* rtree = (R_Tree*)w.addr;
  rtree->DeleteFile();
  delete rtree;
}

/*
6.8 ~Check~-function of type constructor ~rtree~

*/
bool CheckRTree(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("rtree")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo)
      && algMgr->CheckKind("SPATIAL", nl->Third(type), errorInfo);
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("RTREE"), type));
    return false;
  }
  return true;
}

/*
6.9 ~Cast~-function of type constructor ~rtree~

*/
void* CastRTree(void* addr)
{
  return ( 0 );
}

/*
6.10 ~Open~-function of type constructor ~rtree~

*/
bool
OpenRTree( SmiRecord& valueRecord,
           const ListExpr typeInfo,
           Word& value )
{
//  cout << "Open RTree" << endl;
  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), 0 );
  R_Tree *rtree = new R_Tree( fileid );
  value = SetWord( rtree );
  return true;
}

/*
6.11 ~Save~-function of type constructor ~rtree~

*/
bool
SaveRTree( SmiRecord& valueRecord,
           const ListExpr typeInfo,
           Word& value )
{
//  cout << "Save RTree" << endl;
  R_Tree *rtree = (R_Tree *)value.addr;
  SmiFileId fileid = rtree->FileId();
  valueRecord.Write( &fileid, sizeof( SmiFileId ), 0 );
  return true;
}

/*
6.11 ~SizeOf~-function of type constructor ~rtree~

*/
int
NoSize()
{
  return 0;
}

/*
6.12 Type Constructor object for type constructor ~rtree~

*/
TypeConstructor rtree( "rtree",              RTreeProp,
                       OutRTree,             InRTree,
                       0,                    0,
                       CreateRTree,          DeleteRTree,
                       OpenRTree,            SaveRTree,
                       CloseRTree,           CloneRTree,
                       CastRTree,            NoSize,
                       CheckRTree,
                       0,
                       TypeConstructor::DummyInModel,
                       TypeConstructor::DummyOutModel,
                       TypeConstructor::DummyValueToModel,
                       TypeConstructor::DummyValueListToModel );


/*
7 Operators of the ~rtree~ algebra

7.1 Operator ~creatertree~

7.1.1 Type Mapping of operator ~creatertree~

*/
ListExpr CreateRTreeTypeMap(ListExpr args)
{
  string attrName;
  char* errmsg = "Incorrect input for operator creatertree.";
  int attrIndex;
  ListExpr attrType;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 2, errmsg);

  ListExpr relDescription = nl->First(args);
  ListExpr attrNameLE = nl->Second(args);

  CHECK_COND(nl->IsAtom(attrNameLE), errmsg);
  CHECK_COND(nl->AtomType(attrNameLE) == SymbolType, errmsg);
  attrName = nl->SymbolValue(attrNameLE);

  CHECK_COND(!nl->IsEmpty(relDescription), errmsg);
  CHECK_COND(nl->ListLength(relDescription) == 2, errmsg);

  ListExpr relSymbol = nl->First(relDescription);;
  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsAtom(relSymbol), errmsg);
  CHECK_COND(nl->AtomType(relSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(relSymbol) == "rel", errmsg);

  CHECK_COND(nl->ListLength(tupleDescription) == 2, errmsg);
  ListExpr tupleSymbol = nl->First(tupleDescription);;
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(tupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(tupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(attrList), errmsg);
  CHECK_COND((attrIndex = FindAttribute(attrList, attrName, attrType)) > 0, errmsg);

  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo;
  CHECK_COND(algMgr->CheckKind("SPATIAL", attrType, errorInfo), errmsg);

  ListExpr resultType =
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->TwoElemList(
        nl->IntAtom(attrIndex),
        nl->StringAtom(nl->SymbolValue(attrType))),
      nl->ThreeElemList(
        nl->SymbolAtom("rtree"),
        tupleDescription,
        attrType));

  return resultType;
}

/*
7.1.2 Value mapping function of operator ~createbtree~

*/
enum SpatialKind { Point, Points, Line, Region };

int
CreateRTreeValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Relation* relation;
  CcInt* attrIndexCcInt;
  int attrIndex;
  CcString* attrTypeStr;
  RelationIterator* iter;
  Tuple* tuple;
  SpatialKind dataType;

  R_Tree *rtree = (R_Tree*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

  relation = (Relation*)args[0].addr;
  attrIndexCcInt = (CcInt*)args[2].addr;
  attrTypeStr = (CcString*)args[3].addr;

  assert(rtree != 0);
  assert(relation != 0);
  assert(attrIndexCcInt != 0);
  assert(attrTypeStr != 0);

  attrIndex = attrIndexCcInt->GetIntval() - 1;
  char* attrType = (char*)attrTypeStr->GetStringval();
  if(strcmp(attrType, "point") == 0)
  {
    dataType = Point;
  }
  else if(strcmp(attrType, "points") == 0)
  {
    dataType = Points;
  }
  else if(strcmp(attrType, "line") == 0)
  {
    dataType = Line;
  }
  else if(strcmp(attrType, "region") == 0)
  {
    dataType = Region;
  }
  else
  {
    assert(false /* this should not happen */);
  }

  iter = relation->MakeScan();
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    BBox box = ((StandardSpatialAttribute*)tuple->GetAttribute(attrIndex))->BoundingBox();
    R_TreeEntry e( box, tuple->GetTupleId() );
    rtree->Insert( e );

    tuple->DeleteIfAllowed();
  }

  delete iter;
  return 0;
}

/*
7.1.3 Specification of operator ~creatertree~

*/
const string CreateRTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>((rel (tuple ((x1 t1)...(xn tn))))"
                                " xi)"
                                " -> (rtree (tuple ((x1 t1)...(xn tn))) ti)"
                                "</text--->"
                                "<text>_ creatertree [ _ ]</text--->"
                                "<text>Creates an rtree. The key type ti must"
                                " be a spatial type.</text--->"
                                "<text>let myrtree = countries creatertree [boundary]"
                                "</text--->"
                                ") )";

/*
7.1.4 Definition of operator ~creatertree~

*/
Operator creatertree (
          "creatertree",                // name
          CreateRTreeSpec,              // specification
          CreateRTreeValueMapping,   	// value mapping
          Operator::DummyModel, 	// dummy model mapping, defines in Algebra.h
          Operator::SimpleSelect,         	// trivial selection function
          CreateRTreeTypeMap        	// type mapping
);

/*
7.2 Operator ~windowintersects~

7.2.1 Type mapping function of operator ~windowintersects~

*/
ListExpr WindowIntersectsTypeMap(ListExpr args)
{
  char* errmsg = "Incorrect input for operator windowintersects.";

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 3, errmsg);

  /* Split argument in three parts */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr searchWindow = nl->Third(args);

  /* find out type of key */
  CHECK_COND(nl->IsAtom(searchWindow), errmsg);
  CHECK_COND(nl->AtomType(searchWindow) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(searchWindow) == "rect", errmsg );

  /* handle btree part of argument */
  CHECK_COND(!nl->IsEmpty(rtreeDescription), errmsg);
  CHECK_COND(!nl->IsAtom(rtreeDescription), errmsg);
  CHECK_COND(nl->ListLength(rtreeDescription) == 3, errmsg);

  ListExpr rtreeSymbol = nl->First(rtreeDescription);;
  ListExpr rtreeTupleDescription = nl->Second(rtreeDescription);
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType), errmsg);
  CHECK_COND(nl->AtomType(rtreeKeyType) == SymbolType, errmsg);
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo;
  CHECK_COND(algMgr->CheckKind("SPATIAL", rtreeKeyType, errorInfo), errmsg);

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol), errmsg);
  CHECK_COND(nl->AtomType(rtreeSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(rtreeSymbol) == "rtree", errmsg);

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription), errmsg);
  CHECK_COND(!nl->IsAtom(rtreeTupleDescription), errmsg);
  CHECK_COND(nl->ListLength(rtreeTupleDescription) == 2, errmsg);
  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);;
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(rtreeTupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(rtreeTupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(rtreeAttrList), errmsg);

  /* handle rel part of argument */
  CHECK_COND(!nl->IsEmpty(relDescription), errmsg);
  CHECK_COND(!nl->IsAtom(relDescription), errmsg);
  CHECK_COND(nl->ListLength(relDescription) == 2, errmsg);

  ListExpr relSymbol = nl->First(relDescription);;
  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsAtom(relSymbol), errmsg);
  CHECK_COND(nl->AtomType(relSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(relSymbol) == "rel", errmsg);

  CHECK_COND(!nl->IsEmpty(tupleDescription), errmsg);
  CHECK_COND(!nl->IsAtom(tupleDescription), errmsg);
  CHECK_COND(nl->ListLength(tupleDescription) == 2, errmsg);
  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol), errmsg);
  CHECK_COND(nl->AtomType(tupleSymbol) == SymbolType, errmsg);
  CHECK_COND(nl->SymbolValue(tupleSymbol) == "tuple", errmsg);
  CHECK_COND(IsTupleDescription(attrList), errmsg);

  /* check that btree and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, rtreeAttrList), errmsg);

  ListExpr resultType =
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription);

  return resultType;
}

/*
7.2.2 Value mapping function of operator ~windowintersects~

*/
struct WindowIntersectsLocalInfo
{
  Relation* relation;
  R_Tree* rtree;
  BBox *searchBox;
  bool first;
};

int
WindowIntersectsValueMapping(Word* args, Word& result,
                             int message, Word& local,
                             Supplier s)
{
  WindowIntersectsLocalInfo *localInfo;

  switch (message)
  {
    case OPEN :
    {
      Word rtreeWord, relWord, boxWord;
      qp->Request(args[0].addr, rtreeWord);
      qp->Request(args[1].addr, relWord);
      qp->Request(args[2].addr, boxWord);

      localInfo = new WindowIntersectsLocalInfo;
      localInfo->rtree = (R_Tree*)rtreeWord.addr;
      localInfo->relation = (Relation*)relWord.addr;
      localInfo->first = true;
      localInfo->searchBox = (Rectangle *)boxWord.addr;

      assert(localInfo->rtree != 0);
      assert(localInfo->relation != 0);
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (WindowIntersectsLocalInfo*)local.addr;
      R_TreeEntry e;

      if(localInfo->first)
      {
        localInfo->first = false;
        if( localInfo->rtree->First( *localInfo->searchBox, e ) )
        {
          Tuple *tuple = localInfo->relation->GetTuple(e.pointer);
          result = SetWord(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
      else
      {
        if( localInfo->rtree->Next( e ) )
        {
          Tuple *tuple = localInfo->relation->GetTuple(e.pointer);
          result = SetWord(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
    }

    case CLOSE :
    {
      localInfo = (WindowIntersectsLocalInfo*)local.addr;
      delete localInfo;
      return 0;
    }
  }
  return 0;
}

/*
7.2.3 Specification of operator ~windowintersects~

*/
const string WindowIntersectsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\""
                                     " \"Example\" )"
                                     "( <text>((rtree (tuple ((x1 t1)...(xn tn)))"
                                     " ti)(rel (tuple ((x1 t1)...(xn tn)))) rect) ->"
                                     " (stream (tuple ((x1 t1)...(xn tn))))"
                                     "</text--->"
                                     "<text>_ _ windowintersects [ _ ]</text--->"
                                     "<text>Uses the given rtree to find all tuples"
                                     " in the given relation with .xi intersects the "
                                     " argument value.</text--->"
                                     "<text>query citiesInd cities windowintersects"
                                     " [r] consume; where citiesInd "
                                     "is e.g. created with 'let citiesInd = "
                                     "cities creatertree [pos]'</text--->"
                                     ") )";
/*
7.2.4 Definition of operator ~windowintersects~

*/
Operator windowintersects (
         "windowintersects",            // name
         WindowIntersectsSpec,          // specification
         WindowIntersectsValueMapping,  // value mapping
         Operator::DummyModel, 		// dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,         		// trivial selection function
         WindowIntersectsTypeMap	// type mapping
);


/*
8 Definition and initialization of ~rtree~ algebra

*/
class RTreeAlgebra : public Algebra
{
 public:
  RTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &rtree );

    AddOperator(&creatertree);
    AddOperator(&windowintersects);
  }
  ~RTreeAlgebra() {};
};

RTreeAlgebra rtreealgebra;


extern "C"
Algebra*
InitializeRTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&rtreealgebra);
}

