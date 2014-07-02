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
  R_TreeNode<dim, TupleId> tmp = Root();
  if(!tmp.IsLeaf()){
     pq->push( DistanceElement<TupleId>( RootRecordId(),false, -1,
          tmp.BoundingBox().Distance(box), 0));
  } else {
    for ( int ii = 0; ii < tmp.EntryCount(); ++ii ) {
          R_TreeLeafEntry<dim, LeafInfo> e =
            (R_TreeLeafEntry<dim, LeafInfo>&)(tmp)[ii];
          pq->push( DistanceElement<LeafInfo>( 0,
              true, e.info, e.box.Distance( box ),
              1));
    }
  }
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
    void deleteAll( NNnode *node);
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
  deleteAll( rootnode );
}

/*
delete the NNnode of the iterator pos und returns the element
after pos or end()

*/
template<class T>
typename NNTree<T>::iter NNTree<T>::erase( iter &pos)
{
  if( pos.itNode == NULL)
  {
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
  }
  delete pos.itNode;
  --nrelements;
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
  assert( it.itNode->left == NULL );
  NNnode *nNode = newNode(e, it.itNode);
  it.itNode->left = nNode;
  if( it.itNode == first )
  {
    first = nNode;
  }
  return iter(nNode);
}

template<class T>
typename NNTree<T>::iter NNTree<T>::addRight( T &e, iter &it)
{
  assert( it.itNode->right == NULL );
  NNnode *nNode = newNode(e, it.itNode);
  it.itNode->right = nNode;
  return iter(nNode);
}

/*
add elem beyond given node

*/
template<class T>
typename NNTree<T>::iter NNTree<T>::addElem( T &e, iter &it)
{
  assert( it.itNode != NULL );
  if( it.itNode->right == NULL)
  {
    NNnode *nNode = newNode(e, it.itNode);
    it.itNode->right = nNode;
    return iter(nNode);
  }
  else
  {
    NNnode *n = minNode(it.itNode->right);
    NNnode *nNode = newNode(e, n);
    n->left = nNode;
    return iter(nNode);
  }
}

/*
iter functions

*/
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

template<class T>
void NNTree<T>::deleteAll( NNnode *node)
{
  if( node )
  {
    deleteAll( node->left);
    deleteAll( node->right);
    delete node;
  }
}


/*
Definitions for the operator knearestfilter
The struct FieldEntry is needed to insert
elements into a vector. The class NNSegTree
is a special segment tree for the operator
knearestfilter

*/
template<class timeType>
struct FieldEntry
{
  long nodeid;
  double maxdist;
  timeType start, end;
  int level;
  FieldEntry( long node, double maxd, const timeType &s, const timeType &e,
                  int l):
    nodeid(node),
    maxdist(maxd),
    start(s),
    end(e),
    level(l)
    {}

};
/*
extend one more attribute: mindist

*/
template<class timeType>
struct EFieldEntry:public FieldEntry<timeType>
{
  double mindist; //extension
  EFieldEntry( long node, double mind,double maxd,int l,
              const timeType &s, const timeType &e):
            FieldEntry<timeType>(node,maxd,s,e,l),mindist(mind){}
};

template<class timeType>
class SegEntry {
  public:
    BBox<2> xyBox;
    timeType start, end;
    double mindist, maxdist;
    int coverage;
    long nodeid;
//    TupleId tpid;
    long tpid;
    char direction;//new entry
    SegEntry():
      start(), end(),
      mindist(0),maxdist(0),coverage(0),
      nodeid( -1 ),
      tpid( -1 )
      {}

    SegEntry( const BBox<2> &box, const timeType &s,
              const  timeType &e, double mind,
      double maxd, int cov,long node, TupleId tid):
      xyBox(box),
      start(s), end(e),
      mindist(mind),maxdist(maxd),coverage(cov),
      nodeid( node ),
      tpid( tid )
    {}

    virtual ~SegEntry()
    {}

    friend bool operator!=(const SegEntry<timeType>& i,
                    const SegEntry<timeType>& j)
    {
      return i.nodeid != j.nodeid || i.tpid != j.tpid;
    }

    bool operator<( const SegEntry<timeType>& e ) const
    {
      if( e.start != start)
      {
        return start < e.start;
      }
      else
      {
        //same starttimes
        if( e.end != end )
        {
          return end < e.end;
        }
        else
        {
          //same times
          return tpid < e.tpid;
        }
      }
    }
};


template<class timeType>
class SegNode {
  public:
    timeType start, end;
    SegNode<timeType>* left;
    SegNode<timeType>* right;
    SegNode<timeType>* parent;
    NNTree<SegEntry<timeType> > segEntries;
    SegNode( const timeType &s, const timeType &e) :
      start(s), end(e), left(NULL),
      right(NULL), parent(NULL), segEntries()
      {}
};


template<class timeType>
class NNSegTree{
public:
  NNSegTree( const timeType &s, const timeType &e);
  ~NNSegTree();
  void insert( SegEntry<timeType> &s, int k );
  bool erase( const timeType& start, const timeType& end,
              long rnodeid, double dist);
  void fillMap( map< SegEntry<timeType>, TupleId> &m);
  int calcCoverage( const timeType& t1,const  timeType& t2, double distance );

  typedef typename NNTree<SegEntry<timeType> >::iter ITSE;
private:
  SegNode<timeType> *sroot;
  void makeEmpty( SegNode<timeType> *node);
  void insertNode( SegEntry<timeType> &s, SegNode<timeType> *node, int k);
  void eraseEntry( const timeType&  start, const timeType& end, long rnodeid,
                   double dist, SegNode<timeType> *node, bool &result);
  void checkErase( const timeType& t1, const timeType& t2, double distance,
                   SegNode<timeType> *node, int k );
  void mapfill( map< SegEntry<timeType>, TupleId> &m,
                SegNode<timeType> *node);
  ITSE addEntry(NNTree<SegEntry<timeType> > &t, SegEntry<timeType>  &e);
  ITSE findEntry(NNTree<SegEntry<timeType> > &t, long rnodeid, double dist);
  ITSE findEntryMindistance(NNTree<SegEntry<timeType> > &t, double dist);
  int calcCoverage( const timeType& t1,const timeType& t2, double distance,
                    SegNode<timeType> *node, bool hasEqual );
};

/*
constructor

*/
template<class timeType>
NNSegTree<timeType>::NNSegTree( const timeType &s, const timeType &e)
{
  sroot = new SegNode<timeType>(s, e);
}

/*
destructor

*/
template<class timeType>
NNSegTree<timeType>::~NNSegTree<timeType>()
{
  makeEmpty( sroot );
}

/*
insert, inserts an elements in all nodes where
it is necessary. Some childs may be created.
This function calls the private recursive function
insertNode. Some elements would be needless.
They are deleted

*/
template<class timeType>
void NNSegTree<timeType>::insert( SegEntry<timeType> &s, int k )
{
  insertNode( s, sroot, k );
}

/*
erase looks for an element in the segment tree
in all possible nodes. Every appearance is deleted.
This function calls the private recursive function
eraseEntry

*/
template<class timeType>
bool NNSegTree<timeType>::erase(const timeType& start,
                                const timeType& end,
                                long rnodeid, double dist)
{
  bool result = false;
  eraseEntry( start, end, rnodeid, dist, sroot, result );
  return result;
}

/*
calls the private recursive method mapfill
to fill the given map with all elements which
are in the segment tree

*/
template<class timeType>
void NNSegTree<timeType>::fillMap( map< SegEntry<timeType>, TupleId> &m)
{
  mapfill( m, sroot );
}

/*
calcCoverage calculates the reached coverage in the given
time intervall until the given distance. It calls the
recursive private method calcCoverage

*/
template<class timeType>
int NNSegTree<timeType>::calcCoverage( const timeType& t1,
                                       const timeType& t2,
                                       double distance )
{
  return calcCoverage( t1, t2, distance, sroot, false );
}

/*
private functions of NNSegTree

*/

/*
makeEmpty is the private recursive funtion
to free all nodes of the segment tree.
It is calles by the destructor of the tree.

*/
template<class timeType>
void NNSegTree<timeType>::makeEmpty( SegNode<timeType> *node)
{
  if( node )
  {
    makeEmpty( node->left );
    makeEmpty( node->right);
    delete node;
  }
}

/*
mapfill is a private recursive method called by
fillMap. It fills a map with all elements which
are in the segment tree

*/
template<class timeType>
void NNSegTree<timeType>::mapfill( map< SegEntry<timeType>, TupleId> &m,
                                  SegNode<timeType> *node)
{
  if( node )
  {
    if( node->left )
    {
      mapfill( m, node->left );
      mapfill( m, node->right);
    }
    ITSE it = node->segEntries.begin();
    while( it != node->segEntries.end() )
    {
      assert( it->tpid != -1);
      m[ *it ] = it->tpid;
      ++it;
    }
  }
}

/*
insertNode insert an element in all nodes where
it is necessary. Some childs may be created

*/
template<class timeType>
void NNSegTree<timeType>::insertNode( SegEntry<timeType> &s,
                                      SegNode<timeType> *node, int k)
{
  if( s.start <= node->start && s.end >= node->end)
  {
    // insert the element, the timeintervall is o.K.
    addEntry(node->segEntries, s);
    checkErase(s.start, s.end, s.maxdist, node, k);
  }
  else if( node->left != NULL)
  {
    //the node has childs
    if( s.start < node->left->end && s.end <= node->left->end)
    {
      //both times are on the left
      insertNode( s, node->left, k );
    }
    else if( s.start >= node->right->start )
    {
      //both times are on the right
      insertNode( s, node->right, k );
    }
    else /* starttime on the left, endtime on the right */
    {
      insertNode( s, node->left, k );
      insertNode( s, node->right, k );
    }
  }
  else /* the node has no childs, make some */
  {
    if( s.start > node->start )
    {
      SegNode<timeType> *newleft = new SegNode<timeType>(node->start, s.start);
      SegNode<timeType> *newright = new SegNode<timeType>(s.start, node->end);
      newleft->parent = node;
      newright->parent = node;
      node->left = newleft;
      node->right = newright;
      insertNode( s, node->right, k );
    }
    else
    {
      /* the endtime was too low */
      SegNode<timeType> *newleft = new SegNode<timeType>(node->start, s.end);
      SegNode<timeType> *newright = new SegNode<timeType>(s.end, node->end);
      newleft->parent = node;
      newright->parent = node;
      node->left = newleft;
      node->right = newright;
      insertNode( s, node->left, k );
    }
  }
}

/*
eraseEntry looks for an element in the segment tree
in all possible nodes. Every appearance is deleted
in the given (partial) tree

*/
template<class timeType>
void NNSegTree<timeType>::eraseEntry(const  timeType& start,
                                     const  timeType& end, long rnodeid,
    double dist, SegNode<timeType> *node, bool &result)
{
  if( start <= node->start && end >= node->end)
  {
    ITSE it = findEntry( node->segEntries, rnodeid, dist);
    if( it != node->segEntries.end() )
    {
      node->segEntries.erase( it );
      result = true;
    }
  }
  else if( node->left )
  {
    // there are childs to look for the element
    if( start < node->left->end && end <= node->left->end)
    {
      //both times are on the left
      eraseEntry( start, end, rnodeid, dist, node->left, result);
    }
    else if( start >= node->right->start )
    {
      //both times are on the right
      eraseEntry( start, end, rnodeid, dist, node->right, result);
    }
    else /* starttime on the left, endtime on the right */
    {
      eraseEntry( start, end, rnodeid, dist, node->left, result);
      eraseEntry( start, end, rnodeid, dist, node->right, result);
    }
  }
}

/*
addEntry inserts a element SegEntry in the
NNTree of the attribute segEntries of a
node of the segment tree

*/
template<class timeType>
typename NNSegTree<timeType>::ITSE
NNSegTree<timeType>::addEntry(NNTree<SegEntry<timeType> > &t,
                              SegEntry<timeType> &e)
{
  if( t.size() == 0)
  {
    return t.addFirst(e);
  }

  double dist = e.maxdist;
  ITSE it = t.root();
  while( true)
  {
    double storeDistance = it->maxdist;
    if( dist < storeDistance)
    {
      if( it.hasLeft() )
      {
        it.leftItem();
      }
      else
      {
        return t.addLeft( e, it);
      }
    }
    else if( dist > storeDistance)
    {
      if( it.hasRight() )
      {
        it.rightItem();
      }
      else
      {
        return t.addRight( e, it);
      }
    }
    else //same distance
    {
      return t.addElem( e, it );
    }
  }
}

/*
findEntry looks for a element SegEntry in the
NNTree of the attribute segEntries of a
node of the segment tree

*/
template<class timeType>
typename NNSegTree<timeType>::ITSE
NNSegTree<timeType>::findEntry(NNTree<SegEntry<timeType> > &t,
                               long rnodeid, double dist)
{
  ITSE it = t.root();
  bool havePos = false;
  while( !havePos && it != t.end())
  {
    double storeDistance = it->maxdist;
    if( dist < storeDistance)
    {
      if( it.hasLeft() )
      {
        it.leftItem();
      }
      else
      {
        havePos = true;
      }
    }
    else if( dist > storeDistance)
    {
      if( it.hasRight() )
        it.rightItem();
      else
      {
        havePos = true;
      }
    }
    else //same distance
    {
      havePos = true;
    }
  }
  if( it != t.end() && rnodeid == it->nodeid){ havePos = true; }
  else { havePos = false; }

  ITSE pos1 = it;
  ITSE pos2 = it;
  while( !havePos && (pos1 != t.begin() || pos2 != t.end()))
  {
    if( pos1 != t.begin() )
    {
      --pos1;
      if( rnodeid == pos1->nodeid)
      {
        pos2 = pos1;
        havePos = true;
      };
    }

    if( !havePos && pos2 != t.end() )
    {
      ++pos2;
      if( pos2 != t.end() && rnodeid == pos2->nodeid)
      {
        havePos = true;
      }
    }
  }
  return pos2;
}

/*
findEntryMindistance looks for a element SegEntry in the
NNTree of the attribute segEntries of a
node of the segment tree which has a mindistance
higher than the given distance

*/
template<class timeType>
typename NNSegTree<timeType>::ITSE
NNSegTree<timeType>::findEntryMindistance(NNTree<SegEntry<timeType> > &t,
                                     double dist)
{
  //the function looks first for a maxdistance higher because
  // this is the key for the tree and
  //goes then with the operator++ to the element where
  //the mindistance is also higher
  ITSE it = t.root();
  bool havePos = false;
  while( !havePos && it != t.end())
  {
    double storeDistance = it->maxdist;
    if( dist < storeDistance)
    {
      if( it.hasLeft() )
      {
        it.leftItem();
      }
      else
      {
        havePos = true;
      }
    }
    else if( dist > storeDistance)
    {
      if( it.hasRight() )
      {
        it.rightItem();
      }
      else
      {
        havePos = true;
      }
    }
    else //same distance
    {
      havePos = true;
    }
  }
  while( it != t.end())
  {
    if( it->mindist > dist )
    {
      return it;
    }
    ++it;
  }
  return it;
}

/*
checkErase checks, if there some elements no longer needed
after the insertion of the element s into the given node.
This function deletes the needless elements.

*/
template<class timeType>
void NNSegTree<timeType>::checkErase( const timeType& t1,
                                      const timeType& t2, double distance,
                           SegNode<timeType> *node, int k )
{
  int c = calcCoverage( t1, t2, distance, sroot, true );
  if( c >= k )
  {
    ITSE it = findEntryMindistance( node->segEntries, distance);
    while( it != node->segEntries.end() )
    {
      if( it->mindist > distance )
      {
        it = node->segEntries.erase( it );
      }
      else
      {
        ++it;
      }
    }
  }
  if( node->left )
  {
    //do the same for the childs
    checkErase( node->left->start, node->left->end, distance, node->left, k);
    checkErase( node->right->start, node->right->end, distance, node->right, k);
  }
}

/*
calculates the reached coverage in the given timeintervall
until the given distance

*/
template<class timeType>
int NNSegTree<timeType>::calcCoverage(const timeType& t1,
                                      const timeType& t2,
                                      double distance,
                                      SegNode<timeType> *node, bool hasEqual)
{
  int result = 0;
  if( node )
  {
    if( node->start <= t1 && node->end >= t2)
    {
      ITSE it = node->segEntries.begin();
      if( hasEqual )
      {
        //includes also the elements with equal distance
        while( it != node->segEntries.end() && it->maxdist <= distance)
        {
          result += it->coverage;
          ++it;
        }
      }
      else
      {
        //includes only the elements with lower distance
        while( it != node->segEntries.end() && it->maxdist < distance)
        {
          result += it->coverage;
          ++it;
        }
      }
    }
    if( node->left )
    {
      // there are childs to look for the element
      if( t2 <= node->left->end)
      {
        //both times are on the left
        result += calcCoverage( t1, t2, distance, node->left, hasEqual);
      }
      else if( t1 >= node->right->start )
      {
        //both times are on the right
        result += calcCoverage( t1, t2, distance, node->right, hasEqual);
      }
      else /* starttime on the left, endtime on the right */
      {
        int lc, rc;
        lc = calcCoverage( t1, node->left->end, distance, node->left,hasEqual);
        rc = calcCoverage( node->right->start, t2, distance,
          node->right, hasEqual);
        result += MIN( lc, rc );
      }
    }
  }
  return result;
}

#endif
