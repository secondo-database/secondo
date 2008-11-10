/*

[1] Header-File of NearestNeighborAlgebra

November 2008, Angelika Braese.

This header file implements some functions which the NearestNeighborAlgebra
needs and which are defined in RTree Algebra.
The follow code must be included in the RTree Algebra:

----
#include <vector>
#include <queue>

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

    struct Near : public binary_function< DistanceElement<LeafInfo>, 
                        DistanceElement<LeafInfo>, bool >
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

typedef vector< DistanceElement<TupleId> > NNVector;
typedef priority_queue< DistanceElement<TupleId>, NNVector, 
        DistanceElement<TupleId>::Near > NNpriority_queue;


template <unsigned dim, class LeafInfo>
class R_Tree
{
  public:
...
void FirstDistanceScan( const BBox<dim>& box );
/ *
FirstDistanceScan initializes the priority queue

* /

void LastDistanceScan(  );
/ *
LastDistanceScan deletes the priority queue of the distancescan

* /

bool R_Tree<dim, LeafInfo>::NextDistanceScan( const BBox<dim>& box, 
                                   LeafInfo& result );
/ *
NextDistanceScan returns true and fills the result with the
ID of the next tuple if there is a next tuple else it returns false

* /
...
  private:
...
    NNpriority_queue* pq;
    / *
    The priority queue for the distancescan functions

    * /
    bool distanceFlag;
    / *
    true, after a call of FirstDistanceScan

    * /
...
}
End of the definitions which had to be included into RTreeAlgebra.h
----

1 Defines and Includes

*/

#ifndef __NEARESTNEIGHBOR_ALGEBRA_H__
#define __NEARESTNEIGHBOR_ALGEBRA_H__

using namespace std;

/*
FirstDistanceScan initializes the priority queue

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::FirstDistanceScan( const BBox<dim>& box )
{
  distanceFlag = true;
  pq = new NNpriority_queue;
  R_TreeNode<dim, TupleId> *tmp = GetNode( RootRecordId(), 
                     false, 
                     MinEntries( 0 ), 
                     MaxEntries( 0 ) );

  pq->push( DistanceElement<TupleId>( RootRecordId(), false, -1, 
      tmp->BoundingBox().Distance(box), 0));
  delete tmp;
}

/*
LastDistanceScan deletes the priority queue of the distancescan

*/
template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::LastDistanceScan(  )
{
  assert(distanceFlag);
  distanceFlag = false;
  delete pq;
}

/*
NextDistanceScan returns true and fills the result with the
ID of the next tuple if there is a next tuple else it returns false

*/
template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::NextDistanceScan( const BBox<dim>& box, 
                                   LeafInfo& result )
{
  assert(distanceFlag);
  while ( !pq->empty() )
  {
    DistanceElement<LeafInfo> elem = pq->top();
    pq->pop();
    if ( elem.IsLeaf() )
    {
      result = elem.TupleId();
      return true;
    }
    else
    {
     R_TreeNode<dim, LeafInfo> *tmp = GetNode( elem.NodeId(), 
                     elem.IsLeaf(), 
                     MinEntries( elem.Level() ), 
                     MaxEntries( elem.Level() ) );
      for ( int ii = 0; ii < tmp->EntryCount(); ++ii )
      {
        if ( tmp->IsLeaf() )
        {
          R_TreeLeafEntry<dim, LeafInfo> e = 
            (R_TreeLeafEntry<dim, LeafInfo>&)(*tmp)[ii];

          pq->push( DistanceElement<LeafInfo>( 0, 
              true, e.info, e.box.Distance( box ), 
              elem.Level() + 1));
        }
        else
        {
          R_TreeInternalEntry<dim> e = 
            (R_TreeInternalEntry<dim>&)(*tmp)[ii];
          pq->push( DistanceElement<LeafInfo>( e.pointer, 
              false, -1, e.box.Distance( box ), 
              elem.Level() + 1));
        }
      }
      delete tmp;
    }
  }

  return false;
}


#endif
