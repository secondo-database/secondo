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

1 Defines and includes for the NearestNeighbor Algebra

*/


#ifndef __NEARESTNEIGHBOR_ALGEBRA_H__
#define __NEARESTNEIGHBOR_ALGEBRA_H__

#include "TemporalAlgebra.h"

using namespace std;

/*
2 Definitions for the ~distancescan operator~

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
  
  cout << "tmp.bbox = "; tmp->BoundingBox().Print(cout)  << endl;
  cout << "box = "; box.Print(cout) << endl;
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

/*
3. Class definitions for the ~knearest operator~

*/
enum EventType {E_RIGHT, E_INTERSECT, E_LEFT};
struct EventElem
{
  EventType type;
  Instant pointInTime; //x-axes, sortkey in the priority queue
  Tuple* tuple;
  Tuple* tuple2; //for intersection
  const UPoint* up;
  MReal* distance;
  EventElem(EventType t, Instant i, Tuple* tu, const UPoint* upt,
    MReal* d) : type(t), pointInTime(i), tuple(tu), tuple2(NULL),
    up(upt), distance(d){}
  EventElem(EventType t, Instant i, Tuple* tu, Tuple* tu2, MReal* d) 
    : type(t), pointInTime(i), tuple(tu), tuple2(tu2),
    up(NULL), distance(d){}
  bool operator<( const EventElem& e ) const 
  {
    if( e.pointInTime != pointInTime)
    {
      return e.pointInTime < pointInTime;
    }
    else
    {
      //same times
      if( e.type != type )
      {
        return e.type < type;
      }
      else
      {
        //same types
        return e.tuple < tuple || (e.tuple == tuple && e.tuple2 < tuple2);
      }
    }
  }
};


namespace near{
class ActiveElem
{
public:
  static Instant currtime;
  MReal *distance;
  Tuple* tuple;
  Instant start; //the start time where the element is needed
  Instant end;
  bool lc;
  bool rc;
  ActiveElem(MReal *dist, Tuple* t, Instant s, Instant e, bool l, bool r) 
    : distance(dist), tuple(t), start(s), end(e), lc(l), rc(r){}
};
}


/*
the class NNTree implements a tree with iterators
the methods to find an element or to find the position
of a new element are implemented outside of the class
in the NearestNeighborAlgebra.cpp

*/
template <class T>
class NNTree
{
  public:
    NNTree() : first(NULL), last(NULL), nrelements(0){}
    ~NNTree();
    class iterator
    {
      class node;
    public:
      iterator( node *n) : itNode(n){}
      friend bool operator==(const iterator& i, const iterator& j)
      {
        return i.itNode == j.itNode;
      }
      friend bool operator!=(const iterator& i, const iterator& j)
      {
        return i.itNode != j.itNode;
      }
      T* operator->(){ return &itNode->elem;}
      T& operator*(){ return itNode->elem;}
      T& operator++(){itNode = itNode->next; return *this;} //prefix
      T operator++(int)  //postfix
      {
        iterator tmp = *this; 
        ++this;
        return tmp;
      }
      T& operator--(){itNode = itNode->prev; return *this;} //prefix
      T operator--(int)  //postfix
      {
        iterator tmp = *this; 
        --this;
        return tmp;
      }
      bool hasLeft(){ return itNode->prev != NULL; }
      bool hasRight(){ return itNode->next != NULL; }
    private:
      node *itNode;
    };
    iterator begin(){return iterator(first);}
    iterator end(){return iterator(last);}
    iterator erase( iterator &pos);
    iterator insert( T &e, iterator &it);
    unsigned int size(){ return nrelements;}
  private:
    class node
    {
    public:
      node *next;
      node *prev;
      T elem;
    };

    node *first;
    node *last;
    unsigned int nrelements;
};

/*
destructor of NNTree delete all nodes

*/
template<class T>
NNTree<T>::~NNTree()
{
  while( first != NULL )
  {
    node *tmp = first->next;
    delete first;
    first = tmp;
  }
  nrelements = 0;
}

/*
delete the node of the iterator pos und returns the element
beyond pos or end()

*/
template<class T>
typename NNTree<T>::iterator NNTree<T>::erase( iterator &pos)
{
  if( pos == end())
  {
    return pos;
  }
  if( pos.itNode->prev != NULL )
  {
    pos.itNode->prev->next = pos.itNode->next;
  }
  pos.itNode->next->prev = pos.itNode->prev;
  node *tmp = pos.itNode->next;
  delete pos.itNode;
  --nrelements;
  return iterator(tmp);
}

/*
insert a new node with the element e before the iterator it

*/
template<class T>
typename NNTree<T>::iterator NNTree<T>::insert( T &e, iterator &it)
{
  node *newNode = new node;
  if( !size )
  {
    ++nrelements;
    node *lastNode = new node;
    lastNode->next = NULL;
    lastNode->prev = newNode;
    last = lastNode;
    first = newNode;
    newNode->prev = NULL;
    newNode->next = last;
    newNode->elem = e;
  }
  else
  {
    ++nrelements;
    newNode->prev = it.itNode->prev;
    newNode->next = it.itNode;
    newNode->elem = e;
  }
  return iterator(newNode); 
}

#endif
