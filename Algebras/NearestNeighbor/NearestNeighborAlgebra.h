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
#include <vector>
#include <queue>

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

#define NN_DIM 2


//template <unsigned dim>
class DistanceElement
{
  private:
    //int level;
    long nodeId;
    BBox<NN_DIM> MBR;
    //long fatherId;
    bool leaf;
    long tpId;
    double distance;

  public:

    bool IsLeaf() const { return leaf; }
    long TupleId() const { return tpId; }
    long NodeId() const { return nodeId; }

    struct Near : public binary_function< DistanceElement, 
                        DistanceElement, bool >
    {
        bool operator()(const DistanceElement e1, 
                              const DistanceElement e2) const
        {
          return e1.distance >= e2.distance;
        }
    };

    DistanceElement():
      nodeId( -1 ),
      leaf( true ),
      tpId( -1 ),
      distance( -1 )
      {}

    DistanceElement( long node, BBox<NN_DIM> box, bool l, long tid, 
                      double dist ):
      nodeId( node ),
      MBR( box ),
      leaf( l ),
      tpId( tid ),
      distance( dist )
    {}

    virtual ~DistanceElement()
    {}
};

typedef vector< class DistanceElement > NNVector;
typedef priority_queue< DistanceElement, 
      vector<class DistanceElement>,
      DistanceElement::Near > NNpriority_queue;



#endif
