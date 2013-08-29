/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the MON-Tree Algebra

[TOC]

0 Overview

This header file implements the MON-Tree index structure for moving
objects in networks.

1 Defines and Includes

*/

#ifndef _MONTREE_ALGEBRA_H
#define _MONTREE_ALGEBRA_H

#define MONTREE_DEBUG

using namespace std;

#include "SecondoSMI.h"
#include "StandardTypes.h"
#include "RTreeAlgebra.h"
#include "HashAlgebra.h"
#include "TemporalAlgebra.h"
#include "NetworkAlgebra.h"
#include "TemporalNetAlgebra.h"


/*
2 Auxiliary structures

2.1 Struct ~MON\_TreeStatistics~

This structure stores some statistics about the MON-Tree.

*/
struct MON_TreeStatistics
{
  MON_TreeStatistics():
    edgeCount( 0 ),
    moCount( 0 ),
    topR_TreeHeight( 0 ),
    topR_TreeNodes( 0 ),
    topR_TreeTotalArea( 0.0 ),
    topR_TreeLx( 0.0 ),
    topR_TreeLy( 0.0 ),
    bottomR_TreeCount( 0 ),
    bottomR_TreeMaximumHeight( 0 ),
    bottomR_TreeAverageHeight( 0 ),
    indexSize( 0 )
    {}

  MON_TreeStatistics( const long edgeCount,
                      const long moCount,
                      const int topR_TreeHeight,
                      const long topR_TreeNodes,
                      const double topR_TreeTotalArea,
                      const double topR_TreeLx,
                      const double topR_TreeLy,
                      const int bottomR_TreeCount,
                      const int bottomR_TreeMaximumHeight,
                      const double bottomR_TreeAverageHeight,
                      const long indexSize ):
    edgeCount( edgeCount ),
    moCount( moCount ),
    topR_TreeHeight( topR_TreeHeight ),
    topR_TreeNodes( topR_TreeNodes ),
    topR_TreeTotalArea( topR_TreeTotalArea ),
    topR_TreeLx( topR_TreeLx ), topR_TreeLy( topR_TreeLy ),
    bottomR_TreeCount( bottomR_TreeCount ),
    bottomR_TreeMaximumHeight( bottomR_TreeMaximumHeight ),
    bottomR_TreeAverageHeight( bottomR_TreeAverageHeight ),
    indexSize( indexSize )
    {}

  long edgeCount;
  long moCount;
  int topR_TreeHeight;
  long topR_TreeNodes;
  double topR_TreeTotalArea;
  double topR_TreeLx;
  double topR_TreeLy;
  int bottomR_TreeCount;
  int bottomR_TreeMaximumHeight;
  double bottomR_TreeAverageHeight;
  long indexSize;
};

/*
2.2 Struct ~TopR\_TreeLeafInfo~

This structure maintains the information that is
stored in the leaf entries of the top R-Tree.
Basically, pointers to the route in the network
and to the bottom R-Tree are stored in such
entries.

*/
struct TopR_TreeLeafInfo
{
  int routeId;
  SmiRecordId childTreeId;

  TopR_TreeLeafInfo():
    routeId( 0 ),
    childTreeId( 0 )
    {}

  TopR_TreeLeafInfo( const int routeId,
                     const SmiRecordId childTreeId ):
    routeId( routeId ),
    childTreeId( childTreeId )
    {}

  TopR_TreeLeafInfo( const TopR_TreeLeafInfo& info ):
    routeId( info.routeId ),
    childTreeId( info.childTreeId )
    {}
};


struct BottomR_TreeLeafInfo
{
  TupleId tupleId;
  int low;
  int high;

  BottomR_TreeLeafInfo() {}

  BottomR_TreeLeafInfo( TupleId tupleId, int low, int high ):
      tupleId( tupleId ), low( low ), high( high )
      {}
};

/*
3 Class ~MON\_Tree~

*/
template<class BottomR_TreeLeafInfo>
class MON_Tree
{
  public:
    MON_Tree();
/*
The simple constructor

*/

    MON_Tree( network::Network *network,
              SmiFileId indexFile,
              SmiFileId hashFile );
/*
This constructor opens an existing MON-tree. It
receives a relation of routes representing the
network as well as the identifiers for the index
and the hash files.

*/

    ~MON_Tree();
/*
The destructor.

*/

    MON_TreeStatistics getStatistics();
/*
Returns statistics from the MON-Tree.

*/

    SmiFileId GetIndexFileId()
    { return index.GetFileId(); }
    SmiFileId GetHashFileId()
    { return routeHash->GetFileId(); }

    void SetNetwork( network::Network *network );
//    void LoadRoutes( Relation *routes );
/*
Loads the routes from the network into the index.

*/

    void Insert( const temporalnet::MGPoint& mgpoint,
                 const BottomR_TreeLeafInfo& mgpointId );
    void Insert( const temporalnet::UGPoint& ugpoint,
                 const BottomR_TreeLeafInfo& ugpointId );
/*
Inserts a moving(gpoint) into the MON-Tree.

*/

    bool First( const Rectangle<2>& searchRectangle,
                const Interval<Instant>& timeInterval,
                R_TreeLeafEntry<2, BottomR_TreeLeafInfo>& result );
    bool Next( R_TreeLeafEntry<2, BottomR_TreeLeafInfo>& result );
/*
The search of the MON-Tree.

*/

  private:
    void Insert( const int routeId, const SmiRecordId routeTupleId );
/*
Inserts a route in the MON-Tree.

*/
    void CalculateSearchBoxSet( const Rectangle<2>& box,
                                const SimpleLine& curve,
                                const Interval<Instant>& timeInterval,
                                RectangleSet<2>& result ) const;

/*
Attributes

*/
    SmiRecordFile index;
    Hash *routeHash;
    network::Network *network;

    R_Tree<2, TopR_TreeLeafInfo> *top_RTree;
    R_Tree<2, BottomR_TreeLeafInfo> *bottom_RTree;

    Rectangle<2> searchBox;
    Interval<Instant> searchTimeInterval;
    bool begin;
    RectangleSet<2> searchBoxSet;

};

#endif // _MONTREE_ALGEBRA_H
