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

July 2008, Christian Duentgen added definitions required for the
~Nearest NeighborAlgebra~ on request of Angelika Braese.

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
#include <string.h>

// Includes required by extensions for the NearestNeighborAlgebra:
#include <vector>
#include <queue>

#include "NestedList.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Attribute.h"

extern NestedList* nl;

#define BBox Rectangle
#define BBoxSet RectangleSet

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

#define BULKLOAD_LEAF_SKIPPING true
/*
If set to 'true', ~leaf skipping~ is activated within the bulk
loading mechanism. Otherwise, set to 'false'.

Leaf skipping will result in more nodes, but with smaller bounding boxes,
when bulkloading an R-tree.

*/

const double BULKLOAD_TOLERANCE = 8.0;
/*
Tolerance for ~leaf skipping~ in bulkload mechanism.
The tolerance specifies, which multiple of the average distance
of bounding boxes is acceptable within a single node.

Value must be >0.0.

*/

const double BULKLOAD_MIN_ENTRIES_FACTOR = 0.0;
/*
Specifies a multiple of MinEntries, that must be reached, before
~leaf skipping~ is performed during a bulkload.

Value should be between 0.0 and 1.0.

*/

const int BULKLOAD_INITIAL_SEQ_LENGTH = 20;
/*
Specifies the minimum number of the first N entries, that will be inserted
without ~leaf skipping~. This holds only for the first N entries on each level.
After that, ~BULKLOAD\_MIN\_ENTRIES\_FACTOR~ will control the skipping behaviour.

*/

/*
2 Template Class ~DistanceElement~

Required for Nearest Neighbour operations.

*/

template<class LeafInfo>
    class DistanceElement
{
  private:
    long nodeId;
    bool leaf;
    LeafInfo tpId;
    double distance;
    int level;

  public:

    bool IsLeaf() const { return leaf; }
    LeafInfo TupleId() const { return tpId; }
    long NodeId() const { return nodeId; }
    int Level() const { return level; }

    struct Near
    {
      bool operator()(const DistanceElement<LeafInfo> e1,
      const DistanceElement<LeafInfo> e2) const
      {
        return e1.distance >= e2.distance;
      }
    };

    DistanceElement():
        nodeId( -1 ),
    leaf( true ),
    tpId( -1 ),
    level( -1 )
    {}

    DistanceElement( long node, bool l, LeafInfo tid,
                      double dist, int curLevel ):
        nodeId( node ),
    leaf( l ),
    tpId( tid ),
    distance( dist ),
    level( curLevel )
    {}

    virtual ~DistanceElement()
    {}
};

typedef std::vector< DistanceElement<TupleId> > NNVector;
typedef std::priority_queue< DistanceElement<TupleId>, NNVector,
        DistanceElement<TupleId>::Near > NNpriority_queue;

/*
3 Struct ~R\_TreeEntry~

*/

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
    memcpy((char*)(&this->box), buffer+offset, sizeof(BBox<dim>) );
    new(&this->box) BBox<dim>();
    offset += sizeof(BBox<dim>);
    memcpy( &pointer, buffer+offset, sizeof(SmiRecordId) );
    offset += sizeof(SmiRecordId);
  }
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char *buffer, int& offset )
  {
    memcpy( buffer+offset,(void*)(&this->box), sizeof(BBox<dim>) );
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
    memcpy((char*)( &this->box), buffer+offset, sizeof(BBox<dim>) );
    new(&this->box) BBox<dim>();
    offset += sizeof(BBox<dim>);
    memcpy( &info, buffer+offset, sizeof(Info) );
    offset += sizeof(Info);
  }
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char *buffer, int& offset )
  {
    memcpy( buffer+offset,(char*)( &this->box), sizeof(BBox<dim>) );
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
    R_TreeEntry<dim>** entries;//


    IntrospectResult():
      level( -1 ),
      nodeId( -1 ),
      fatherId( -1 ),
      isLeaf( true ),
      minEntries( -1 ),
      maxEntries( -1 ),
      countEntries( -1 ),
      entries(new R_TreeEntry<dim>*[maxEntries+1])
      {
        double dmin[dim], dmax[dim];
        for(unsigned int i=0; i < dim; i++)
        {
          dmin[i] = 0.0;
          dmax[i] = 0.0;
        }
//
    for(int i = 0 ;i <= maxEntries;i++)
      entries[i] = NULL;
//
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
      countEntries( countE ),
    entries(new R_TreeEntry<dim>*[maxEntries+1])
    {
    for(int i = 0 ;i <= maxEntries;i++)
      entries[i] = NULL;
  }


   IntrospectResult<dim>& operator=(const IntrospectResult<dim>& src){
      // delete old entries
      for(int i=0;i<=maxEntries;i++){
        if(entries[i]){
          delete entries[i];
          entries[i] = 0;
        }
      }
      if(maxEntries != src.maxEntries){ // different number of entries
        if(entries){
          delete [] entries;
        }
        entries = new R_TreeEntry<dim>*[src.maxEntries+1];
      }
      // copy values
      level = src.level;
      nodeId = src.nodeId;
      MBR = src.MBR;
      fatherId = src.fatherId;
      isLeaf = src.isLeaf;
      minEntries = src.minEntries;
      maxEntries = src.maxEntries;
      countEntries = src.countEntries;
      for(int i=0;i<=maxEntries;i++){
        entries[i] = src.entries[i];
      }
      return *this;
   }



    virtual ~IntrospectResult()
    {
       if(entries){
          for(int i=0;i<=maxEntries;i++){
            if(entries[i]){
              delete entries[i];
              entries[i] = 0;
            }
          }
          delete[] entries;
          entries = 0;
       }
    }
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
/*
Returns the bounding box of this node.

*/

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


    void splitOneDimenNode( R_TreeNode<dim, LeafInfo>& n1, 
                            R_TreeNode<dim, LeafInfo>& n2,
                            int** sortedEntries );
/*
Special split function for one dimensional nodes.

*/

    void Split( R_TreeNode<dim, LeafInfo>& n1, R_TreeNode<dim, LeafInfo>& n2 );
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
  for( int i = 0; i <= maxEntries; i++ ){
    entries[ i ] = 0;
  }
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
  for( ; i <= node.maxEntries; i++ ){
    entries[ i ] = NULL;
  }
}


/*
4.2 The destructor

*/
template<unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo>::~R_TreeNode()
{
  for( int i = 0; i <= count; i++ ){
    delete entries[ i ];
  }
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
R_TreeNode<dim, LeafInfo>& R_TreeNode<dim, LeafInfo>::operator=
    (const R_TreeNode<dim, LeafInfo>& node )
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
  double maxSep = -std::numeric_limits<double>::max();
  int maxMinNode[ dim ];
  int minMaxNode[ dim ];
  int bestD = -1;

  for( unsigned i = 0; i < dim; i++ )
  {
    maxMinVal[i] = -std::numeric_limits<double>::max();
    minMaxVal[i] = std::numeric_limits<double>::max();
    minVal[i] = std::numeric_limits<double>::max();
    maxVal[i] = -std::numeric_limits<double>::max();
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

  double bestWaste = -std::numeric_limits<double>::max();
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


4.6.1 Special case for only one dimension


*/
template<unsigned dim,class LeafInfo>
void R_TreeNode<dim,LeafInfo>::splitOneDimenNode(
                       R_TreeNode<dim,LeafInfo>& n1,
                       R_TreeNode<dim,LeafInfo>& n2,
                       int** sortedEntries){
  assert(dim==1);
  
  double minD = entries[sortedEntries[0][MaxEntries()/2]]->box.MinD(0);
  double maxD = entries[sortedEntries[1][MaxEntries()/2]]->box.MaxD(0);
  double avg = (minD+maxD)/2.0;
  int mE = MaxEntries()-MinEntries();
  for(int i=0;i<EntryCount();i++){
     if(n1.EntryCount() > mE){
        n2.Insert( *entries[ i ]);
     } else if(n2.EntryCount() > mE){
        n1.Insert( *entries[ i ]);
     } else {
       double avgi = ( entries[i]->box.MinD(0) + entries[i]->box.MaxD(0))/2.0;
       if(avgi < avg){
          n1.Insert( *entries[ i ] );
       } else {
          n2.Insert( *entries[ i ]);
       } 
     }
  }
}


/*
4.6.2 General function

*/
template<unsigned dim, class LeafInfo>
void R_TreeNode<dim, LeafInfo>::Split( R_TreeNode<dim,
                                       LeafInfo>& n1,
                                       R_TreeNode<dim,
                                       LeafInfo>& n2 )
// Splits this node into two ones: n1 and n2, which must be empty nodes.
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

    if(dim==1){ // for only 1 dimension, there is no 
                //reason for computing the split
                // dimension
       splitOneDimenNode( n1,n2,sortedEntry);
       delete [] sortedEntry[ 0 ];
       delete [] sortedEntry[ 1 ];
       modified = true;
       return;
    }


    struct StatStruct
    {
      double margin;
      double overlap;
      double area;
    } *stat = new StatStruct[ dim*dim*(MaxEntries() + 2 - 2*MinEntries()) ],
      *pstat = stat; // Array of distribution statistics
    
    double minMarginSum = std::numeric_limits<double>::max();
    int minMarginAxis = -1;

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
      double minOverlap = std::numeric_limits<double>::max();
      double minArea = std::numeric_limits<double>::max();
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
void R_TreeNode<dim, LeafInfo>::Read( SmiRecordFile *file,
                                      const SmiRecordId pointer )
{
  SmiRecord record;
  int RecordSelected = file->SelectRecord( pointer, record, SmiFile::ReadOnly );
  assert( RecordSelected );
  Read( record );
}

template<unsigned dim, class LeafInfo>
void R_TreeNode<dim, LeafInfo>::Read( SmiRecord& record )
{
  int offset = 0;
  size_t readed;
  char* buffer = record.GetData(readed);

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

  free(buffer);
  assert(offset<=(int)readed); // otherwise some entries will be uninitialized
  modified = false;
}

template<unsigned dim, class LeafInfo>
void R_TreeNode<dim, LeafInfo>::Write( SmiRecordFile *file,
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

    int RecordWritten = record.Write( buffer, Size(), 0 );
    assert( RecordWritten );
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
/*
The first constructor. Creates an empty R-tree.

*/
    R_Tree( const int pageSize, const bool isTemp );

/*
Opens an existing R-tree.

*/
    R_Tree( SmiRecordFile *file );


    R_Tree( const SmiFileId fileId, const bool isTemp );
    R_Tree( SmiRecordFile *file,
            const SmiRecordId headerRecordId );
    R_Tree( const SmiFileId fileId,bool update, const bool isTemp );
/////////////////////////////////////////////////////////////////////////////
    R_Tree( const SmiFileId fileid,const int, const bool isTemp);
    
    void Clear();

    void OpenFile(const SmiFileId fileid){file->Open(fileid);}
    void CloseFile(){file->Close();}
    void SwitchHeader(R_Tree<dim,LeafInfo>*);
    int GetShare(){return header.share;}
    void IncreaseShare(){header.share++; WriteHeader();}
    void DecreaseShare(){header.share--; WriteHeader();}
    void MergeRtree();
    SmiRecordId Record_Path_Id(){return header.path_rec_id;}
////////////////////////////////////////////////////////////////////////////
    void Clone(R_Tree<dim,LeafInfo>*);
    SmiRecordId DFVisit_Rtree(R_Tree<dim,LeafInfo>*,R_TreeNode<dim,LeafInfo>*);


/*
The destructor.

*/
    ~R_Tree();

/*
Open and Save are used by NetworkAlgebra to save and open the rtree of network.

*/
    bool  Open( SmiRecord& valueRecord,
                size_t& offset,
                std::string typeInfo,
                Word &value);

    bool Save(SmiRecord& valueRecord,
                size_t& offset);


/*
The type name used in Secondo:

*/
    inline static const std::string BasicType(){
      if(dim==2){
        return "rtree";
      } else {
        std::ostringstream ss;
        ss << "rtree" << dim;
        return ss.str();
      }
    }
   
   static const bool checkType(ListExpr type){
     return listutils::isRTreeDescription(type, BasicType());
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

    inline void GetNode(SmiRecordId id, R_TreeNode<dim, LeafInfo>& result) {
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

    R_TreeNode<dim, LeafInfo>& Root();
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

Implemented by NearestNeighborAlgebra.

*/

  void GetNeighborNode(const R_TreeLeafEntry<dim, LeafInfo>& ent ,
                          std::vector<int>& list);
   R_TreeNode<dim, LeafInfo> *GetMyNode(SmiRecordId& address,
                                        const bool leaf,
                                        const int min, const int max )
  {
    return GetNode(address,leaf,min,max);
  }

  bool getFileStats( SmiStatResultType &result );

////////////

  bool InitializeBLI(const bool& leafSkipping=BULKLOAD_LEAF_SKIPPING);


  std::ostream& printHeader(std::ostream& o) const{
      return header.print(o);
  }

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

      //new record
      SmiRecordId second_head_id;    //two rtrees on the same file
      SmiRecordId path_rec_id;  //record update path for new coverage
      int share;

      Header() :
        headerRecordId( 0 ), rootRecordId( 0 ),
        minLeafEntries( 0 ), maxLeafEntries( 0 ),
        minInternalEntries( 0 ), maxInternalEntries( 0 ),
        nodeCount( 0 ), entryCount( 0 ), height( 0 ),
        second_head_id(0),path_rec_id(0), share(0)
        {}
      Header( SmiRecordId _headerRecordId, SmiRecordId _rootRecordId = 0,
              int _minEntries = 0, int _maxEntries = 0,
              int _minInternalEntries = 0, int _maxInternalEntries = 0,
              int _nodeCount = 0, int _entryCount = 0,
              int _nodeSize = 0, int _height = 0, int s =0 ) :
        headerRecordId( _headerRecordId ),
        rootRecordId( _rootRecordId ),
        minLeafEntries( _minEntries ),
        maxLeafEntries( _maxEntries ),
        minInternalEntries( _minInternalEntries ),
        maxInternalEntries( _maxInternalEntries ),
        nodeCount( _nodeCount ),
        entryCount( _entryCount ),
        height( _height ), share(s)
        {}

      std::ostream& print(std::ostream& o) const{
       o << "RTreeHeader["
         << "headerRecordId = " <<  headerRecordId
         << ", rootRecordId = " <<  rootRecordId
         << ", minLeafEntries = " <<  minLeafEntries
         << ", maxLeafEntries = " << maxLeafEntries
         << ", minInternalEntries = " << minInternalEntries
         << ", maxInternalEntries = " << maxInternalEntries
         << ", nodeCount = " << nodeCount
         << ", entryCount = " << entryCount
         << ", height = " << height
         << ", second_head_id = " <<  second_head_id
         << ", path_rec_id = " << path_rec_id
         << ", share = " << share
         << "]";
        return o;
      }

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

    NNpriority_queue* pq;
/*
The priority queue for the distancescan functions
Used by NearestNeighborAlgebra.

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
R_Tree<dim, LeafInfo>::R_Tree( const int pageSize, const bool isTemp ) :
  fileOwner( true ),
  file( new SmiRecordFile( true, pageSize, isTemp) ),
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

template<unsigned dim, class LeafInfo>
void R_Tree<dim,LeafInfo>::Clear(){
  assert(file);
  file->Truncate(); // delete all stuff in file
  // reset header entries which depend on entries
  header.nodeCount = 0;
  header.entryCount = 0;
  header.height = 0;
  header.second_head_id = 0;
  header.path_rec_id = 0;
  header.share = 0;
  currLevel = -1;
  currEntry = -1;
  reportLevel = -1;
  searchBoxSet.Clear();
  searchBox.SetDefined(false);
  searchType = NoSearch;
  bulkMode = false;
  if(bli) {
     delete bli;
     bli = 0;
  }
  nodeIdCounter = 0;

  // NNpriority queue is managed from outside
  distanceFlag = false;
  noentrynode = 0; 

  // initialize member arrays
  // should be replaced by memset
  for( int i = 0; i < MAX_PATH_SIZE; i++ )
  {
    overflowFlag[ i ] = 0;
    nodeId[ i ] = 0;
    path[i] = 0;
    pathEntry[i] = 0;
  }

  if(nodePtr){
      delete nodePtr;
  }
  
  nodePtr = new R_TreeNode<dim, LeafInfo>( true,
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

};



template <unsigned dim, class LeafInfo>
R_Tree<dim, LeafInfo>::R_Tree( SmiRecordFile *file ) :
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
  int nodeEmptySize = R_TreeNode<dim, LeafInfo>::SizeOfEmptyNode();
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

  nodePtr = new R_TreeNode<dim, LeafInfo>( true,
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
R_Tree<dim, LeafInfo>::R_Tree( const SmiFileId fileid , const bool isTemp) :
fileOwner( true ),
file( new SmiRecordFile( true,0,isTemp ) ),
header( 1 ),
nodePtr( NULL ),
currLevel( -1 ),
currEntry( -1 ),
reportLevel( -1 ),
searchBox( false ),
searchBoxSet(),
searchType( NoSearch ),
bulkMode(false),
bli(0),
nodeIdCounter( 0 )
{
//  cout<<"111"<<endl;
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
//  cout<<"header share"<<header.share<<endl;
  if(header.share > 0) header.share++;

  nodePtr = GetNode( RootRecordId(),
                     currLevel == Height(),
                     MinEntries( currLevel ),
                     MaxEntries( currLevel ) );
  path[ 0 ] = header.rootRecordId;
}

template <unsigned dim, class LeafInfo>
R_Tree<dim, LeafInfo>::R_Tree( SmiRecordFile *file,
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
bulkMode(false),
bli(0),
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
R_Tree<dim, LeafInfo>::R_Tree( const SmiFileId fileid,const int pageSize,
                               const  bool isTemp ) :
fileOwner( true ),
file( new SmiRecordFile( true,0,isTemp )  ),
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
R_Tree<dim, LeafInfo>::R_Tree( const SmiFileId fileid, bool update, 
                               const bool isTemp ) :
fileOwner( true ),
file( new SmiRecordFile( true,0,isTemp ) ),
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
R_Tree<dim, LeafInfo>::~R_Tree()
{
//  cout<<"~R_Tree()"<<endl;
  header.share--;
  if( file->IsOpen() )
  {
    if( nodePtr != NULL )
      PutNode( path[ currLevel ], &nodePtr );

    WriteHeader();

    if( fileOwner )
      file->Close();
  }
  
  if(fileOwner)
  {
    delete file;
  }
  
  if(bli != NULL)
  {
    delete bli;
    bli = NULL;
  }
}

/*
5.3 Reading and writing the header

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::ReadHeader()
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
void R_Tree<dim, LeafInfo>::WriteHeader()
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
void R_Tree<dim, LeafInfo>::PutNode( const SmiRecordId recno,
                                     R_TreeNode<dim, LeafInfo> **node )
{
  assert( file->IsOpen() );
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
  assert( file->IsOpen() );
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
  searchType = NoSearch;

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
        double bestEnlargement = std::numeric_limits<double>::max(),
               bestoverlap = std::numeric_limits<double>::max();

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
      double bestEnlargement = std::numeric_limits<double>::max();
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
  assert( file->IsOpen() );

  if( nodePtr->Insert( entry ) ){
    UpdateBox();
  } else {
    if( !do_forced_reinsertion || currLevel == 0 ||
        overflowFlag[ Height() - currLevel ] )
    { // Node splitting is necessary
      R_TreeNode<dim, LeafInfo> *n1 =
        new R_TreeNode<dim, LeafInfo>
            (nodePtr->IsLeaf(), nodePtr->MinEntries(), nodePtr->MaxEntries());
      R_TreeNode<dim, LeafInfo> *n2 =
        new R_TreeNode<dim, LeafInfo>
            (nodePtr->IsLeaf(), nodePtr->MinEntries(), nodePtr->MaxEntries() );

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
bool R_Tree<dim, LeafInfo>::First( const BBoxSet<dim>& boxSet,
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
bool R_Tree<dim, LeafInfo>::Next( R_TreeLeafEntry<dim, LeafInfo>& result )
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
    bool R_Tree<dim, LeafInfo>::IntrospectNextE(unsigned long& nodeid,BBox<dim>&
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
    bool R_Tree<dim, LeafInfo>::IntrospectNext( IntrospectResult<dim>& result )
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
5.17 Method Remove

*/
template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::Remove( const R_TreeLeafEntry<dim,
                                    LeafInfo>& entry )
{
  assert( file->IsOpen() );

  searchType = NoSearch;

  // First locate the entry in the tree
  if( !FindEntry( entry ) )
    return false;
  else
  { // Create a list of nodes whose entries must be reinserted
    std::stack<int> reinsertLevelList;
    std::stack<R_TreeNode<dim, LeafInfo>*> reinsertNodeList;
    BBox<dim> sonBox( false );

    // remove leaf node entry
    nodePtr->Remove( currEntry );
    header.entryCount -= 1;

    while( currLevel > 0 )
    {
      int underflow = nodePtr->EntryCount() < MinEntries( currLevel );

      if( underflow )
      { // Current node has underflow. Save it for later reinsertion
        R_TreeNode<dim, LeafInfo>* nodePtrcopy = new R_TreeNode<dim, LeafInfo>
            ( *nodePtr );

        reinsertNodeList.push( nodePtrcopy );
        reinsertLevelList.push( currLevel );

        // Remove node from the tree
        nodePtr->Clear();
        int RecordDeleted = file->DeleteRecord( path[ currLevel ] );
        assert( RecordDeleted );
        delete nodePtr;  // destroy in memory node
        nodePtr = 0; // avoid flushing node durcing UpLevel
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
void R_Tree<dim, LeafInfo>::GetNeighborNode(const R_TreeLeafEntry<dim, LeafInfo>
                                                & ent ,std::vector<int>& list)
{
  if(FindEntry(ent)){
    //currEntry,;
    //R_TreeNode<dim,LeafInfo>* nodePtr;
    if(nodePtr->IsLeaf()){
      for(int i = 0;i < nodePtr->EntryCount();i++){
        if(i != currEntry){
          R_TreeLeafEntry<2,LeafInfo> *entry = nodePtr->GetLeafEntry(i);
          list.push_back(entry->info);
        }
      }
    }
  }
}

/*
5.18 Method Root

*/
template <unsigned dim, class LeafInfo>
R_TreeLeafEntry<dim,LeafInfo>*
    R_TreeNode<dim,LeafInfo>::GetLeafEntry(const int index) const
{
  assert(index >= 0 && index <= maxEntries);
  return (R_TreeLeafEntry<dim,LeafInfo>*)entries[index];
}

template <unsigned dim, class LeafInfo>
R_TreeInternalEntry<dim>*
     R_TreeNode<dim,LeafInfo>::GetInternalEntry(const int index) const
{
  assert(index >= 0 && index < count);
  return (R_TreeInternalEntry<dim>*)entries[index];
}

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

  if(bulkMode ||
     bli != NULL)
  {
    return false;
  }
  
  bulkMode = true;
  bli = new BulkLoadInfo<dim, LeafInfo>(leafSkipping);
  return true;
};

template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::InsertBulkLoad(const R_TreeEntry<dim>& entry)
{
  if( bli->node[0] != NULL ) {
    assert(bulkMode == true);
  }
  bli->currentLevel = 0;
  if( bli->node[0] == NULL ) { // create a fresh leaf node
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
  if(bli->currentLevel == 0) { // create a leaf node
    bli->node[bli->currentLevel] =
        new R_TreeNode<dim,LeafInfo>(true,
                                     header.minLeafEntries,
                                     header.maxLeafEntries);
  } else { // create an internal node
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


/*
Merge two rtrees which are stored in the same file
1) height1 > height2 insert second r-tree to the first
2) height1 = height2 create a new node and insert the two into it
3) height1 < height2 insert first r-tree to the second

*/
/*template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::MergeRtree(R_Tree<dim,LeafInfo>* rtree_in1,
R_Tree<dim,LeafInfo>* rtree_in2)
{
  //get root node of the second rtree
  SmiRecordId adr2 = rtree_in2->RootRecordId();
  R_TreeNode<dim,TupleId>* root2 = rtree_in2->GetMyNode(adr2,false,
                        MinEntries(0),MaxEntries(0));
  R_TreeInternalEntry<dim> e2(root2->BoundingBox(),adr2);
  delete root2;

  //get root node of the first rtree
  SmiRecord record1;
  int RecordSelected =
    file->SelectRecord(header.second_head_id,record1,SmiFile::ReadOnly);
  assert(RecordSelected);
  Header temp_head;
  int RecordRead = record1.Read(&temp_head,sizeof(Header),0) == sizeof(Header);
  assert(RecordRead);

  SmiRecordId root1_id = temp_head.rootRecordId;
  int tree1_node_count = temp_head.nodeCount;
  int tree1_entry_count = temp_head.entryCount;

  R_TreeNode<dim,TupleId>* insert_node;
  //create an entry on the second root node

  //first assume, the height of first rtree is higher than the second
//  cout<<" height1 "<<temp_head.height<<" height2 "<<header.height<<endl;
  if(temp_head.height > header.height){
      int cur_height = temp_head.height;//height of first r-tree
      vector<SmiRecordId> path;
      SmiRecordId path_record_id = root1_id;
      while(cur_height > header.height){
        path.push_back(path_record_id);
        insert_node = GetMyNode(path_record_id,false,
                        MinEntries(0),MaxEntries(0));
        assert(insert_node->IsLeaf() == false);
        int i = insert_node->EntryCount();
        R_TreeInternalEntry<dim> e =
                (R_TreeInternalEntry<dim>&)(*insert_node)[i-1];
        path_record_id = e.pointer;
        delete insert_node;
        cur_height--;
      }
      //new height
      header.height = temp_head.height;

      assert(path.size() > 0);
      int index = path.size()-1;
///////////////////////////////////////////////////////////////////
        R_TreeNode<dim,TupleId>*  update_path = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        //the entries have to recalculate coverage number
//////////////////////////////////////////////////////////////////
      // find where to insert and insert the root of second rtree
      for(; index >= 0;index--){
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));
        int i = node->EntryCount();
        if(i < node->MaxEntries()){ //find the place to insert
//          cout<<"before "<<node->BoundingBox()<<endl;
          assert(node->Insert(e2));
          node->UpdateBox(e2.box,e2.pointer);
          header.entryCount++;
//          cout<<"after "<<node->BoundingBox()<<endl;
       //////////// replace entry for update parent node///////////
          e2.box = node->BoundingBox();
          e2.pointer = path[index];
          update_path->Insert(e2);  //recored new coverage id
      ///////////////////////////////////////////////////////////
          PutNode(path[index],&node);
          delete node;
          break;
        }else{ //node is full
          delete node;
          node = new
                R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
          SmiRecordId new_node_rec_id;
          SmiRecord new_node_rec;
          int AppendRecord = file->AppendRecord(new_node_rec_id,new_node_rec);
          assert(AppendRecord);
          e2.pointer = new_node_rec_id;
          node->Insert(e2);
          node->UpdateBox(e2.box,e2.pointer);
          node->Write(new_node_rec);
          header.nodeCount++;
          update_path->Insert(e2);  //recored new coverage id
          delete node;
        }
      }
      //update parent node
      index--;
      while(index >= 0){
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));
        node->UpdateBox(e2.box,e2.pointer);
        PutNode(path[index],&node);
        e2.box = node->BoundingBox();
        e2.pointer = path[index];
        update_path->Insert(e2);  //recored new coverage id
        delete node;
      }

   ///////////////////////////////////////////////////////
      SmiRecordId path_rec_id;
      SmiRecord path_rec;
      int append = file->AppendRecord(path_rec_id,path_rec);
      assert(append);
      update_path->Write(path_rec);
      delete update_path;
      //update header
      header.entryCount += tree1_entry_count;
      header.nodeCount += tree1_node_count;
      header.rootRecordId = root1_id;
      header.path_rec_id = path_rec_id;
      WriteHeader();
  }else if(temp_head.height == header.height){
        /// create a new node and insert the two root nodes as entries//////
        R_TreeNode<dim,TupleId>* node = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        SmiRecordId new_node_rec_id;
        SmiRecord new_node_rec;
        int AppendRecord = file->AppendRecord(new_node_rec_id,new_node_rec);
        assert(AppendRecord);

        R_TreeNode<dim,TupleId>* root1 = rtree_in1->GetMyNode(root1_id,false,
                        MinEntries(0),MaxEntries(0));
        R_TreeInternalEntry<dim> e1(root1->BoundingBox(),root1_id);
        delete root1;
        //update new node
        node->Insert(e1);
        node->UpdateBox(e1.box,e1.pointer);
        node->Insert(e2);
        node->UpdateBox(e2.box,e2.pointer);

        ////////////////how to insert e1 and e2 into new root//////////////
        cout<<"the same height"<<endl;

        //////////////////////////////////////////////////////////////////

        node->Write(new_node_rec);
        BBox<3> bbox = node->BoundingBox();
        delete node;
        //////////// record update path ///////////////
        R_TreeNode<dim,TupleId>*  update_path = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        R_TreeInternalEntry<dim> e3(bbox,new_node_rec_id);
        update_path->Insert(e3);
        SmiRecordId path_rec_id;
        SmiRecord path_rec;
        AppendRecord = file->AppendRecord(path_rec_id,path_rec);
        assert(AppendRecord);
        update_path->Write(path_rec);
        delete update_path;

        ///////////update header///////////////////////
//        cout<<"node count "<<header.nodeCount<<
//           " entry count "<< header.entryCount<<endl;
        header.nodeCount++;
        header.entryCount += 2;
        header.entryCount += tree1_entry_count;
        header.nodeCount += tree1_node_count;
        header.rootRecordId = new_node_rec_id;
        header.height++;
                ////////////
        header.path_rec_id = path_rec_id;
               ////////////
        WriteHeader();
  }else{ //height1 < height

    ///////// insert the first r-tree to the second r-tree  /////////////
      int cur_height = header.height;//height of first r-tree
      vector<SmiRecordId> path;
      SmiRecordId path_record_id = adr2;
      while(cur_height > temp_head.height){
        path.push_back(path_record_id);
        insert_node = GetMyNode(path_record_id,false,
                        MinEntries(0),MaxEntries(0));
        assert(insert_node->IsLeaf() == false);
        R_TreeInternalEntry<dim> e =
                (R_TreeInternalEntry<dim>&)(*insert_node)[0];
        path_record_id = e.pointer;
        delete insert_node;
        cur_height--;
      }
      assert(path.size() > 0);

    ///////////////////////////////////////////////////////////////////
        R_TreeNode<dim,TupleId>*  update_path = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        //the entries have to recalculate coverage number
    ///////////////////////////////////////////////////////////////////

      int index = path.size()-1;
      // find where to insert and insert the root of second rtree
      for(; index >= 0;index--){
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));
        int i = node->EntryCount();
        if(i < node->MaxEntries()){ //find the place to insert

          R_TreeNode<dim,TupleId>*  node_copy = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));

//          cout<<"before "<<node_copy->BoundingBox()<<endl;
          ////// insert the first r-tree ahead //
          assert(node_copy->Insert(e2));
          node_copy->UpdateBox(e2.box,e2.pointer);
          header.entryCount++;
          ///////////copy the original entry ////
          for(int j = 0;j < i;j++){
            R_TreeInternalEntry<dim> e =
                (R_TreeInternalEntry<dim>&)(*node)[j];
            assert(node_copy->Insert(e));
            node_copy->UpdateBox(e.box,e.pointer);
          }
//          cout<<"after "<<node_copy->BoundingBox()<<endl;
          delete node;
       //////////// replace entry for update parent node///////////
          e2.box = node_copy->BoundingBox();
          e2.pointer = path[index];
          update_path->Insert(e2);  //recored new coverage id
      ///////////////////////////////////////////////////////////
          PutNode(path[index],&node_copy);
          delete node_copy;
          break;
        }else{ //node is full
          delete node;
          node = new
                R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
          SmiRecordId new_node_rec_id;
          SmiRecord new_node_rec;
          int AppendRecord = file->AppendRecord(new_node_rec_id,new_node_rec);
          assert(AppendRecord);
          e2.pointer = new_node_rec_id;
          node->Insert(e2);
          node->UpdateBox(e2.box,e2.pointer);
          node->Write(new_node_rec);
          header.nodeCount++;
          update_path->Insert(e2); //recored new coverage id
          delete node;
        }
      }
      //update parent node
      index--;
      while(index >= 0){
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));
        node->UpdateBox(e2.box,e2.pointer);
        PutNode(path[index],&node);
        e2.box = node->BoundingBox();
        e2.pointer = path[index];
        update_path->Insert(e2); //recored new coverage id
        delete node;
      }

    /////////////////////////////////////////////////////////
      SmiRecordId path_rec_id;
      SmiRecord path_rec;
      int append = file->AppendRecord(path_rec_id,path_rec);
      assert(append);
      update_path->Write(path_rec);
      delete update_path;
      //update header
      header.entryCount += tree1_entry_count;
      header.nodeCount += tree1_node_count;
      header.rootRecordId = adr2;
      header.path_rec_id = path_rec_id;
      WriteHeader();
  }
}*/


template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::MergeRtree()
{
  //get root node of the second rtree
  SmiRecordId adr2 = RootRecordId();
  R_TreeNode<dim,TupleId>* root2 = GetMyNode(adr2,false,
                        MinEntries(0),MaxEntries(0));
  R_TreeInternalEntry<dim> e2(root2->BoundingBox(),adr2);
  delete root2;

  //get root node of the first rtree
  SmiRecord record1;
  int RecordSelected =
    file->SelectRecord(header.second_head_id,record1,SmiFile::ReadOnly);
  assert(RecordSelected);
  Header temp_head;
  int RecordRead = record1.Read(&temp_head,sizeof(Header),0) == sizeof(Header);
  assert(RecordRead);

  SmiRecordId root1_id = temp_head.rootRecordId;
  R_TreeNode<dim,TupleId>* root1 = GetMyNode(root1_id,false,
                        MinEntries(0),MaxEntries(0));
  R_TreeInternalEntry<dim> e1(root1->BoundingBox(),root1_id);
  delete root1;

  int tree1_node_count = temp_head.nodeCount;
  int tree1_entry_count = temp_head.entryCount;

  R_TreeNode<dim,TupleId>* insert_node;
  //create an entry on the second root node

  //first assume, the height of first rtree is higher than the second
//  cout<<" height1 "<<temp_head.height<<" height2 "<<header.height<<endl;
  if(temp_head.height > header.height){
      int cur_height = temp_head.height;//height of first r-tree
      std::vector<SmiRecordId> path;
      SmiRecordId path_record_id = root1_id;
      while(cur_height > header.height){
        path.push_back(path_record_id);
        insert_node = GetMyNode(path_record_id,false,
                        MinEntries(0),MaxEntries(0));
        assert(insert_node->IsLeaf() == false);
        int i = insert_node->EntryCount();
        R_TreeInternalEntry<dim> e =
                (R_TreeInternalEntry<dim>&)(*insert_node)[i-1];
        path_record_id = e.pointer;
        delete insert_node;
        cur_height--;
      }
      //new height
      header.height = temp_head.height;

      assert(path.size() > 0);
      int index = path.size()-1;

/*      int temp_index = index;
      while(temp_index >= 0){
          cout<<"path[temp_index] "<<path[temp_index]<<endl;
          temp_index--;
      }*/

///////////////////////////////////////////////////////////////////
        R_TreeNode<dim,TupleId>*  update_path = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        //the entries have to recalculate coverage number
//////////////////////////////////////////////////////////////////
      // find where to insert and insert the root of second rtree
      for(; index >= 0;index--){
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));
        int i = node->EntryCount();
        if(i < node->MaxEntries()){ //find the place to insert
//          cout<<"before "<<node->BoundingBox()<<endl;
          assert(node->Insert(e2));
          node->UpdateBox(e2.box,e2.pointer);
          header.entryCount++;
//          cout<<"after "<<node->BoundingBox()<<endl;
       //////////// replace entry for update parent node///////////
          e2.box = node->BoundingBox();
          e2.pointer = path[index];
          update_path->Insert(e2);  //record new coverage id
      ///////////////////////////////////////////////////////////
          PutNode(path[index],&node);
          delete node;
          break;
        }else{ //node is full
          delete node;
          node = new
                R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
          SmiRecordId new_node_rec_id;
          SmiRecord new_node_rec;
          int AppendRecord = file->AppendRecord(new_node_rec_id,new_node_rec);
          assert(AppendRecord);
          e2.pointer = new_node_rec_id;
          node->Insert(e2);
          node->UpdateBox(e2.box,e2.pointer);
          node->Write(new_node_rec);
          header.nodeCount++;
          update_path->Insert(e2);  //record new coverage id
          delete node;
        }
      }
      //update parent node
      index--;

      while(index >= 0){
//        cout<<"index "<<path[index]<<" e2 "<<e2.pointer<<endl;
//        cout<<"path[index] "<<path[index]<<endl;
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));

        node->UpdateBox(e2.box,e2.pointer);

//        cout<<node->BoundingBox()<<endl;
        e2.box = node->BoundingBox();
        e2.pointer = path[index];
        update_path->Insert(e2);  //record new coverage id
        PutNode(path[index],&node);
        delete node;
        index--;
      }

   ///////////////////////////////////////////////////////
      SmiRecordId path_rec_id;
      SmiRecord path_rec;
      int append = file->AppendRecord(path_rec_id,path_rec);
      assert(append);
      update_path->Write(path_rec);
      delete update_path;
      //update header
      header.entryCount += tree1_entry_count;
      header.nodeCount += tree1_node_count;
      header.rootRecordId = root1_id;
      header.path_rec_id = path_rec_id;
      header.share = 2;
      WriteHeader();

  }else if(temp_head.height == header.height){
        /// create a new node and insert the two root nodes as entries//////
        R_TreeNode<dim,TupleId>* node = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        SmiRecordId new_node_rec_id;
        SmiRecord* new_node_rec = new SmiRecord();
        int AppendRecord = file->AppendRecord(new_node_rec_id,*new_node_rec);
        assert(AppendRecord);

        ///////////////////////////////////////////////////////////////

          R_TreeNode<dim,TupleId>* n1 = GetMyNode(e1.pointer,false,
                        MinEntries(0),MaxEntries(0));
          R_TreeNode<dim,TupleId>* n2 = GetMyNode(e2.pointer,false,
                        MinEntries(0),MaxEntries(0));
//          cout<<" n1 entry "<<n1->EntryCount()
//              <<" n2 entry "<<n2->EntryCount()<<endl;

          if(n1->EntryCount() >= MinEntries(0) &&
             n2->EntryCount() >= MinEntries(0)){
//            cout<<"11"<<endl;
            node->Insert(e1);
            node->UpdateBox(e1.box,e1.pointer);
            node->Insert(e2);
            node->UpdateBox(e2.box,e2.pointer);
            delete n1;
            delete n2;
            header.entryCount += 2;
          }else if((n1->EntryCount() +  n2->EntryCount())
                      < node->MaxEntries()){
//              cout<<"222"<<endl;
              for(int i = 0;i < n2->EntryCount();i++){
                R_TreeInternalEntry<dim> e =
                  (R_TreeInternalEntry<dim>&)(*n2)[i];
                n1->Insert(e);
                n1->UpdateBox(e.box, e.pointer);
              }
              R_TreeInternalEntry<dim> e3(n1->BoundingBox(),e1.pointer);
              PutNode(e1.pointer, &n1);
              node->Insert(e3);
              node->UpdateBox(e3.box,e3.pointer);
              delete n2;
              header.entryCount += 1;
          }else if(n1->EntryCount() > n2->EntryCount()){
//              cout<<"333"<<endl;
              int deviation = MinEntries(0) - n2->EntryCount();
              assert(deviation > 0);
              assert(n1->EntryCount() - deviation > n1->MinEntries());

              R_TreeNode<dim,TupleId>*  n3 =
                      new R_TreeNode<dim,LeafInfo>
                      (false, MinEntries(0),MaxEntries(0));

              for(int i = n1->EntryCount() - deviation;
                       i < n1->EntryCount();i++){
                  R_TreeInternalEntry<dim> e =
                    (R_TreeInternalEntry<dim>&)(*n1)[i];
                  n3->Insert(e);
                  n3->UpdateBox(e.box, e.pointer);
              }
              //////////////////////////////////////////
              R_TreeNode<dim,TupleId>*  n4 =
                      new R_TreeNode<dim,LeafInfo>
                      (false, MinEntries(0),MaxEntries(0));

              for(int i = 0; i < deviation;i++){
                 R_TreeInternalEntry<dim> e =
                    (R_TreeInternalEntry<dim>&)(*n1)[i];
                  n4->Insert(e);
                  n4->UpdateBox(e.box, e.pointer);
              }
              R_TreeInternalEntry<dim> e3(n4->BoundingBox(),e1.pointer);
              PutNode(e1.pointer,&n4);
              node->Insert(e3);
              node->UpdateBox(e3.box, e3.pointer);
              /////////////////////////////////////////////

              for(int i = 0;i < n2->EntryCount();i++){
                R_TreeInternalEntry<dim> e =
                    (R_TreeInternalEntry<dim>&)(*n2)[i];
                n3->Insert(e);
                n3->UpdateBox(e.box, e.pointer);
              }
              R_TreeInternalEntry<dim> e4(n3->BoundingBox(),e2.pointer);
              PutNode(e2.pointer,&n3);
              node->Insert(e4);
              node->UpdateBox(e4.box, e4.pointer);
              delete n1;
              delete n2;
              header.entryCount += 2;
          }else if(n2->EntryCount() > n1->EntryCount()){
//              cout<<"44"<<endl;
              int deviation = MinEntries(0) - n1->EntryCount();
              assert(deviation > 0);
              assert(n2->EntryCount() - deviation > n2->MinEntries());

              for(int i = 0; i < deviation;i++){
                R_TreeInternalEntry<dim> e =
                  (R_TreeInternalEntry<dim>&)(*n2)[i];
                  n1->Insert(e);
                  n1->UpdateBox(e.box, e.pointer);
              }

              R_TreeNode<dim,TupleId>*  n3 =
                      new R_TreeNode<dim,LeafInfo>
                      (false, MinEntries(0),MaxEntries(0));
              for(int i = deviation; i < n2->EntryCount();i++){
                  R_TreeInternalEntry<dim> e =
                    (R_TreeInternalEntry<dim>&)(*n2)[i];
                n3->Insert(e);
                n3->UpdateBox(e.box, e.pointer);
              }

              PutNode(e1.pointer,&n1);
              node->Insert(e1);
              node->UpdateBox(e1.box,e1.pointer);
              R_TreeInternalEntry<dim> e3(n3->BoundingBox(),e2.pointer);
              PutNode(e2.pointer,&n3);
              node->Insert(e3);
              node->UpdateBox(e3.box,e3.pointer);
              header.entryCount += 2;
          }else assert(false);

        ////////////////////////////////////////////////////////////

        //update new node
/*        node->Insert(e1);
        node->UpdateBox(e1.box,e1.pointer);
        node->Insert(e2);
        node->UpdateBox(e2.box,e2.pointer);*/


        BBox<3> bbox = node->BoundingBox();
        PutNode(*new_node_rec,&node);

//        nodePtr->Insert(R_TreeInternalEntry<dim>(bbox,new_node_rec_id));

        delete new_node_rec;
        delete node;

        //////////// record update path ///////////////
        R_TreeNode<dim,TupleId>*  update_path = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        R_TreeInternalEntry<dim> e3(bbox, new_node_rec_id);
        update_path->Insert(e3);
        SmiRecordId path_rec_id;
        SmiRecord path_rec;
        AppendRecord = file->AppendRecord(path_rec_id, path_rec);
        assert(AppendRecord);
        update_path->Write(path_rec);
        delete update_path;

        ///////////update header///////////////////////
//        cout<<"node count "<<header.nodeCount<<
//           " entry count "<< header.entryCount<<endl;
        header.nodeCount++;
//        header.entryCount += 2;
        header.entryCount += tree1_entry_count;
        header.nodeCount += tree1_node_count;
        header.rootRecordId = new_node_rec_id;
        header.height++;
                ////////////
        header.path_rec_id = path_rec_id;
               /////////////
        path[0] = new_node_rec_id;
        header.share = 2;
        WriteHeader();
//    cout<<"header "<<header.headerRecordId<<"root id "<<RootRecordId()<<endl;
/*        ReadHeader();
        nodePtr = GetNode(RootRecordId(),false,MinEntries(0),MaxEntries(0));
        path[0] = header.rootRecordId;*/
//        PutNode(RootRecordId(),&nodePtr);

  }else{ //height1 < height2

    ///////// insert the first r-tree to the second r-tree  /////////////
      int cur_height = header.height;//height of second r-tree
      std::vector<SmiRecordId> path;
      SmiRecordId path_record_id = adr2;
      while(cur_height > temp_head.height){
        path.push_back(path_record_id);
        insert_node = GetMyNode(path_record_id,false,
                        MinEntries(0),MaxEntries(0));
        assert(insert_node->IsLeaf() == false);
        R_TreeInternalEntry<dim> e =
                (R_TreeInternalEntry<dim>&)(*insert_node)[0];
        path_record_id = e.pointer;
        delete insert_node;
        cur_height--;
      }
      assert(path.size() > 0);

    ///////////////////////////////////////////////////////////////////
        R_TreeNode<dim,TupleId>*  update_path = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
        //the entries have to recalculate coverage number
    ///////////////////////////////////////////////////////////////////

      int index = path.size()-1;
      // find where to insert and insert the root of second rtree
      for(; index >= 0;index--){
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));
        int i = node->EntryCount();
        if(i < node->MaxEntries()){ //find the place to insert

          R_TreeNode<dim,TupleId>*  node_copy = new
              R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));

//          cout<<"before "<<node_copy->BoundingBox()<<endl;
          ////// insert the first r-tree ahead //
          assert(node_copy->Insert(e1));
          node_copy->UpdateBox(e1.box,e1.pointer);
          header.entryCount++;
          ///////////copy the original entry ////
          for(int j = 0;j < i;j++){
            R_TreeInternalEntry<dim> e =
                (R_TreeInternalEntry<dim>&)(*node)[j];
            assert(node_copy->Insert(e));
            node_copy->UpdateBox(e.box,e.pointer);
          }
//          cout<<"after "<<node_copy->BoundingBox()<<endl;
          delete node;
       //////////// replace entry for update parent node///////////
          e1.box = node_copy->BoundingBox();
          e1.pointer = path[index];
          update_path->Insert(e1);  //recored new coverage id
      ///////////////////////////////////////////////////////////
          PutNode(path[index],&node_copy);
          delete node_copy;
          break;
        }else{ //node is full
          delete node;
          node = new
                R_TreeNode<dim,LeafInfo>(false,MinEntries(0),MaxEntries(0));
          SmiRecordId new_node_rec_id;
          SmiRecord new_node_rec;
          int AppendRecord = file->AppendRecord(new_node_rec_id,new_node_rec);
          assert(AppendRecord);
          e1.pointer = new_node_rec_id;
          node->Insert(e1);
          node->UpdateBox(e1.box,e1.pointer);
          node->Write(new_node_rec);
          header.nodeCount++;
          update_path->Insert(e1); //recored new coverage id
          delete node;
        }
      }
      //update parent node
      index--;
      while(index >= 0){
        R_TreeNode<dim,TupleId>* node = GetMyNode(path[index],false,
                        MinEntries(0),MaxEntries(0));
        node->UpdateBox(e1.box,e1.pointer);
        e1.box = node->BoundingBox();
        e1.pointer = path[index];
        update_path->Insert(e1); //record new coverage id
        PutNode(path[index],&node);
        delete node;
        index--;
      }

    /////////////////////////////////////////////////////////
      SmiRecordId path_rec_id;
      SmiRecord path_rec;
      int append = file->AppendRecord(path_rec_id,path_rec);
      assert(append);
      update_path->Write(path_rec);
      delete update_path;
      //update header
      header.entryCount += tree1_entry_count;
      header.nodeCount += tree1_node_count;
      header.rootRecordId = adr2;
      header.path_rec_id = path_rec_id;
      header.share = 2;
//      cout<<"write path_rec_id "<<header.path_rec_id<<endl;
      WriteHeader();

//      ReadHeader();
//      cout<<"read path_rec_id "<<header.path_rec_id<<endl;
  }
}

/*
Switch the header content of the first and second R-tree
afterwards, query R-tree will open the second rtree
but there is a record in the header of the first rtree pointing to the second
rtree, so that reverse operator is still valid

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::SwitchHeader(R_Tree<dim,LeafInfo>* rtree_in)
{
  SmiRecord record1;
  int RecordSelected =
    file->SelectRecord(rtree_in->HeaderRecordId(),record1,SmiFile::ReadOnly);
  assert(RecordSelected);
  Header temp_head;
  int RecordRead = record1.Read(&temp_head,sizeof(Header),0) == sizeof(Header);
  assert(RecordRead);

  file->SelectRecord(rtree_in->HeaderRecordId(),record1,SmiFile::Update);
  assert(RecordSelected);

  //write second head into the file (recno-1)
  SmiRecordId sec_head_id = HeaderRecordId();
  header.second_head_id = sec_head_id;
  header.headerRecordId = 1;
  int RecordWrite = record1.Write(&header,sizeof(Header),0) == sizeof(Header);
  assert(RecordWrite);

  //write first head into the file
  SmiRecord record2;
  RecordSelected =
    file->SelectRecord(header.second_head_id,record2,SmiFile::Update);
  assert(RecordSelected);
  temp_head.headerRecordId = sec_head_id;
  temp_head.second_head_id = 1;
  RecordWrite = record2.Write(&temp_head,sizeof(Header),0) == sizeof(Header);
  assert(RecordWrite);
}


/*
DF traverse R-Tree, get new RecordId

Recursively calling

1) write the leaf node into file
2) when all entries of a node have been written to the file,
the (parent) node is written into the file (a record)
3) repeat this process unitl root node

*/

template <unsigned dim, class LeafInfo>
SmiRecordId R_Tree<dim, LeafInfo>::
DFVisit_Rtree(R_Tree<dim,LeafInfo>* rtree_in,R_TreeNode<dim,LeafInfo>* node)
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
    R_TreeNode<dim,LeafInfo>* new_n =
      new R_TreeNode<dim,LeafInfo>(node->IsLeaf(),MinEntries(0),MaxEntries(0));

    for(int i = 0;i < node->EntryCount();i++){
      R_TreeInternalEntry<dim> e =
          (R_TreeInternalEntry<dim>&)(*node)[i];
      R_TreeNode<dim,LeafInfo>* n = rtree_in->GetMyNode(
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
void R_Tree<dim, LeafInfo>::Clone(R_Tree<dim,LeafInfo>* rtree_in)
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

  R_TreeNode<dim,LeafInfo>* rootnode = rtree_in->GetMyNode(
        root_id,false,rtree_in->MinEntries(0),rtree_in->MaxEntries(0));

  root_id = DFVisit_Rtree(rtree_in,rootnode);

  header.nodeCount = rtree_in->NodeCount();
  header.entryCount = rtree_in->EntryCount();
  header.height = rtree_in->Height();
  header.rootRecordId = root_id;
  delete rootnode;
}



template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::FinalizeBulkLoad()
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
        searchBox.SetDefined(false);
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
bool R_Tree<dim, LeafInfo>::getFileStats( SmiStatResultType &result )
{
  result = file->GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << file->GetFileId();
  result.push_back(std::pair<std::string,std::string>("FilePurpose",
            "SecondaryRtreeIndexFile"));
  result.push_back(std::pair<std::string,std::string>("FileId",fileid.str()));
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
  return SetWord(Address(0));
}

/*
6.3 ~Create~-function

*/
template <unsigned dim>
Word CreateRTree( const ListExpr typeInfo )
{

  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
     return SetWord( new R_Tree<dim, TwoLayerLeafInfo>( 4000, false) );
  else
    return SetWord( new R_Tree<dim, TupleId>( 4000, false ));
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

implemented by Jianqiu xu --- 2009.11.30

*/
template <unsigned dim>
Word CloneRTree( const ListExpr typeInfo, const Word& w )
{
/////////////// new implementation ////////////////////////////
  R_Tree<dim,TupleId>* rtree = (R_Tree<dim,TupleId>*)w.addr;
  R_Tree<dim,TupleId>* newrtree =
                  new R_Tree<dim,TupleId>(4000, false);

  newrtree->Clone(rtree);
  return SetWord( newrtree);

////////////////  original version ////////////////////////////////////////
//  return SetWord( Address(0) );
}

/*
6.6 ~Delete~-function

*/
template <unsigned dim>
void DeleteRTree( const ListExpr typeInfo, Word& w )
{

  if (nl->ListLength(typeInfo) == 4)
  {
    if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
      {
        R_Tree<dim, TwoLayerLeafInfo>* rtree = (R_Tree<dim,
                                                TwoLayerLeafInfo>*)w.addr;
    /*    rtree->DeleteFile();
        delete rtree;*/
    //    cout<<"DeleteRTree1 "<<rtree->GetShare()<<endl;
        if(rtree->GetShare() > 0){
           rtree->DecreaseShare();
           rtree->CloseFile();
         }else{
           rtree->DeleteFile();
           delete rtree;
          }
        return;
      }
  }

  R_Tree<dim, TupleId>* rtree = (R_Tree<dim, TupleId>*)w.addr;
  /*    rtree->DeleteFile();
  delete rtree;*/
  //    cout<<"DeleteRTree2 "<<rtree->GetShare()<<endl;
  if(rtree->GetShare() > 0){
    rtree->DecreaseShare();
    rtree->CloseFile();
  }else{
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
        new R_Tree<dim, TwoLayerLeafInfo>( fileid, false );
    value = SetWord( rtree );
  }
  else
  {
    R_Tree<dim, TupleId> *rtree = new R_Tree<dim, TupleId>( fileid, false );
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
    assert(value.addr);
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
  return sizeof(SmiFileId);
}
template<unsigned dim>
struct RTreeNodesLocalInfo {
  bool firstCall;
  bool finished;
  TupleType *resultTupleType;
  R_Tree<dim, TupleId> *rtree;
};

template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::Open(SmiRecord& valueRecord,
                                  size_t& offset,
                                  std::string typeInfo,
                                  Word &value)
{
  SmiFileId fileId;
  size_t n = sizeof(SmiFileId);
  valueRecord.Read(&fileId, n, offset);
  offset += n;
  value = SetWord(new R_Tree<dim, LeafInfo> (fileId,false));
  return true;
};

template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::Save(SmiRecord& valueRecord,
                                 size_t& offset)
{
const size_t n=sizeof(SmiFileId);
SmiFileId fileId= this->FileId();
if (valueRecord.Write(&fileId, n, offset) != n) return false;
offset += n;
return true;
};

template <unsigned dim, class LeafInfo>
bool R_Tree<dim,LeafInfo>::InitializeBLI(const bool& leafSkipping)
{
    if(bulkMode)
    {
      cout << "bulkMode" << endl;
    }
    
    if(bli != NULL)
    {
      cout << "bli" << endl;
      delete bli;
      bli = NULL;
    }
    
//    if(bulkMode || bli != NULL)
//      return false;

    bulkMode = true;
    bli = new BulkLoadInfo<dim,LeafInfo>(leafSkipping);
    return true;
}

typedef R_Tree<1, TupleId> RTree1TID;
typedef R_Tree<2, TupleId> RTree2TID;
typedef R_Tree<3, TupleId> RTree3TID;
typedef R_Tree<4, TupleId> RTree4TID;
typedef R_Tree<8, TupleId> RTree8TID;

typedef R_Tree<1, TwoLayerLeafInfo> RTree1TLLI;
typedef R_Tree<2, TwoLayerLeafInfo> RTree2TLLI;
typedef R_Tree<3, TwoLayerLeafInfo> RTree3TLLI;
typedef R_Tree<4, TwoLayerLeafInfo> RTree4TLLI;
typedef R_Tree<8, TwoLayerLeafInfo> RTree8TLLI;

#endif
