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
  ActiveElem(){}
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
  private:
    class NNnode
    {
    public:
      NNnode(){}
      NNnode *left;
      NNnode *right;
      NNnode *parent;
      T elem;
    };

  public:
    class iter
    {
      friend class NNTree;
    public:
      iter( ) : itNode(NULL){}
      iter( NNnode *n) : itNode(n){}
      friend bool operator==(const NNTree::iter& i, 
        const iter& j)
      {
        return i.itNode == j.itNode;
      }
      friend bool operator!=(const NNTree::iter& i, 
        const NNTree::iter& j)
      {
        return i.itNode != j.itNode;
      }
      T* operator->(){ return &itNode->elem;}
      T& operator*(){ return itNode->elem;}
      iter& operator++(); //prefix
      iter operator++(int)  //postfix
      {
        iter tmp = *this; 
        ++this;
        return tmp;
      }
      iter& operator--(); //prefix
      iter operator--(int)  //postfix
      {
        iter tmp = *this; 
        --this;
        return tmp;
      }
      iter &leftItem()
      { 
        itNode = itNode->left; return *this; 
      }
      iter &rightItem()
      { 
        itNode = itNode->right; return *this; 
      }
      bool hasLeft( ){ return itNode->left != NULL; }
      bool hasRight( ){ return itNode->right != NULL; }
    private:
      typename NNTree::NNnode *itNode;
      NNnode *nextNode( NNnode *n );
      NNnode *prevNode( NNnode *n );
    };
    NNTree();
    ~NNTree();
    iter begin(){return iter(first);}
    iter end(){return iter(NULL);}
    iter root(){ return iter(rootnode);}
    iter erase( iter &pos);
    iter addFirst( T &e);
    iter addLeft( T &e, iter &it);
    iter addRight( T &e, iter &it);
    iter addElem( T &e, iter &it);
    unsigned int size(){ return nrelements;}
  private:

    NNnode *first;
    NNnode *rootnode;
    unsigned int nrelements;
    NNnode *newNode( T &e, NNnode *p);
    unsigned int nodeCount( NNnode *n );
    static NNnode *maxNode( NNnode *n );
    static NNnode *minNode( NNnode *n );
};

/*
Konstruktor of NNTree

*/
template<class T>
NNTree<T>::NNTree() : first(NULL), rootnode(NULL),nrelements(0)
{   
}

/*
destructor of NNTree delete all nodes

*/
template<class T>
NNTree<T>::~NNTree()
{
  iter it = begin();
  while( (it = erase(it)) != end());
  nrelements = 0;
}

/*
delete the NNnode of the iterator pos und returns the element
beyond pos or end()

*/
template<class T>
typename NNTree<T>::iter NNTree<T>::erase( iter &pos)
{
  //cout << "in erase" << endl;
  if( pos.itNode == NULL)
  {
  //cout << "out erase NULL" << endl;
    return pos;
  }
  iter res(pos);
  ++res;
  NNnode *p = pos.itNode->parent;
  if( pos.itNode->left == NULL && pos.itNode->right == NULL)
  {
    if( p != NULL )
    {
      if( p->left == pos.itNode) 
      {
        p->left = NULL;
      }
      else 
        p->right = NULL;
    }
    else
    {
      rootnode = NULL;
    }
  }
  else if( pos.itNode->left == NULL || pos.itNode->right == NULL )
  {
    //only one subtree exists
    if( p != NULL )
    {
      if( p->left == pos.itNode)
      {
        p->left = (pos.itNode->left != NULL) ? pos.itNode->left 
                                            : pos.itNode->right;
        p->left->parent = p;
        //if( p->left == res.itNode)
          //cout << "p->left ist der nächste" << endl;
      }
      else 
      {
        p->right = (pos.itNode->left != NULL) ? pos.itNode->left 
                                            : pos.itNode->right;
        p->right->parent = p;
      }
    }
    else
    {
      rootnode = (pos.itNode->left != NULL) 
        ? pos.itNode->left : pos.itNode->right;
      rootnode->parent = NULL;
    }
  }
  else
  {
    //left and right subtree exists
    //cout << "nodecount left: " << nodeCount( pos.itNode->left) << endl;
    //cout << "nodecount right: " << nodeCount( pos.itNode->right) << endl;
    if( nodeCount( pos.itNode->left) >= nodeCount( pos.itNode->right))
    {
      NNnode *max = maxNode( pos.itNode->left);
      if( max->left != NULL )
      {
        max->left->parent = max->parent;
      }
      if( max->parent->left == max )
        max->parent->left = max->left;
      else
        max->parent->right = max->left;

      max->left = pos.itNode->left;
      max->right = pos.itNode->right;
      max->parent = pos.itNode->parent;
      if( max->left != NULL )
        max->left->parent = max;
      max->right->parent = max;
      if( max->parent != NULL)
      {
        if( max->parent->right == pos.itNode )
          max->parent->right = max;
        else
          max->parent->left = max;
      }
      else
        rootnode = max;
    }
    else
    {
      NNnode *min = minNode( pos.itNode->right);
      if( min->right != NULL )
      {
        min->right->parent = min->parent;
      }
      if( min->parent->left == min )
        min->parent->left = min->right;
      else
        min->parent->right = min->right;

      min->left = pos.itNode->left;
      min->right = pos.itNode->right;
      min->parent = pos.itNode->parent;
      min->left->parent = min;
      if( min->right != NULL )
        min->right->parent = min;
      if( min->parent != NULL)
      {
        if( min->parent->right == pos.itNode )
          min->parent->right = min;
        else
          min->parent->left = min;
      }
      else
        rootnode = min;
    }
  }
  if( first == pos.itNode )
  {
    first = res.itNode;
    //cout << "first neu belegt" << endl;
  }
  delete pos.itNode;
  --nrelements;
  //cout << "out erase " << nrelements << endl;
  return res;
}

/*
add the first rootnode

*/
template<class T>
typename NNTree<T>::iter NNTree<T>::addFirst( T &e)
{
  assert( nrelements == 0 );
  return iter(newNode(e, NULL));
}

/*
add a new NNnode with the element e beyond the iter it

*/
template<class T>
typename NNTree<T>::iter NNTree<T>::addLeft( T &e, iter &it)
{
  //cout << "add left in" << endl;
  assert( it.itNode->left == NULL );
  NNnode *nNode = newNode(e, it.itNode);
  it.itNode->left = nNode;
  if( it.itNode == first )
  {
    first = nNode;
  }
  //cout << "add left out" << endl;
  return iter(nNode); 
}

template<class T>
typename NNTree<T>::iter NNTree<T>::addRight( T &e, iter &it)
{
  //cout << "add right in" << endl;
  assert( it.itNode->right == NULL );
  NNnode *nNode = newNode(e, it.itNode);
  it.itNode->right = nNode;
  //cout << "add right out" << endl;
  return iter(nNode); 
}

/*
add elem beyond given node

*/
template<class T>
typename NNTree<T>::iter NNTree<T>::addElem( T &e, iter &it)
{
  //cout << "add elem in" << endl;
  assert( it.itNode == NULL );
  if( it.itNode->right == NULL)
  {
    NNnode *nNode = newNode(e, it.itNode);
    it.itNode->right = nNode;
    //cout << "add elem out" << endl;
    return iter(nNode); 
  }
  else
  {
    NNnode *n = minNode(it.itNode->right);
    NNnode *nNode = newNode(e, n);
    it.itNode->left = nNode;
    //cout << "add elem out" << endl;
    return iter(nNode); 
  }
}

/*
iter functions

*/
//T& operator++(){itNode = itNode->right; return *this;} //prefix
template<class T>
typename NNTree<T>::iter& NNTree<T>::iter::operator++()    //prefix
{
  itNode = nextNode(itNode); 
  return *this;
}

template<class T>
typename NNTree<T>::iter& NNTree<T>::iter::operator--() //prefix
{
  itNode = prevNode(itNode); 
  return *this;
}

/*
private functions

*/
template<class T>
typename NNTree<T>::NNnode *NNTree<T>::newNode( T &e, NNnode *p)
{
  NNnode *newNode = new NNnode;
  newNode->left = NULL;
  newNode->right = NULL;
  newNode->elem = e;
  if( !nrelements )
  {
    first = newNode;
    rootnode = newNode;
    newNode->parent = NULL;
  }
  else
  {
    newNode->parent = p;
  }
  ++nrelements;
  return newNode; 
}

template<class T>
unsigned int NNTree<T>::nodeCount( NNnode *n )
{
  if( n != NULL ) 
    return 1 + nodeCount(n->left) + nodeCount(n->right);
  else
    return 0;
}

template<class T>
typename NNTree<T>::NNnode *NNTree<T>::maxNode( NNnode *n)
{
  if( n->right != NULL )
    return maxNode( n->right );
  else
    return n;
}

template<class T>
typename NNTree<T>::NNnode *NNTree<T>::minNode( NNnode *n)
{
  if( n->left != NULL )
    return minNode( n->left );
  else
    return n;
}

template<class T>
typename NNTree<T>::NNnode *NNTree<T>::iter::nextNode( NNnode *n)
{
  if( n->right != NULL )
    return minNode( n->right );
  else
  {
    while( n->parent != NULL && n->parent->right == n )
    {
      n = n->parent;
    }
    return n->parent;
  }
}

template<class T>
typename NNTree<T>::NNnode *NNTree<T>::iter::prevNode( NNnode *n)
{
  if( n->left != NULL )
    return NNTree<T>::maxNode( n->left );
  else
  {
    while( n->parent != NULL && n->parent->left == n )
    {
      n = n->parent;
    }
    return n->parent;
  }
}
#endif
