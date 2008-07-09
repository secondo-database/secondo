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

#ifndef __NEARESTNEIGHBORS_ALGEBRA_H__
#define __NEARESTNEIGHBORS_ALGEBRA_H__

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


extern NestedList* nl;
extern QueryProcessor* qp;


template <unsigned dim, class LeafInfo>
void R_Tree<dim, LeafInfo>::FirstDistancesScan( const BBox<dim>& box, 
                                   NNpriority_queue* pq )
{
  R_TreeNode<dim, TupleId> *tmp = GetNode( RootRecordId(), 
                     false, 
                     MinEntries( 0 ), 
                     MaxEntries( 0 ) );

  pq->push( DistanceElement<TupleId>( RootRecordId(), false, -1, 
      tmp->BoundingBox().Distance(box), 0));
  delete tmp;
}

template <unsigned dim, class LeafInfo>
bool R_Tree<dim, LeafInfo>::NextDistancesScan( const BBox<dim>& box, 
                                   NNpriority_queue* pq, LeafInfo& result )
{
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
