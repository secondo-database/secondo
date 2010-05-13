/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the Triangulation

May, 2010 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef POLYGONTRIANGULATOR_H
#define POLYGONTRIANGULATOR_H


#include <vector>
#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <assert.h>

using namespace std;

#define TRUE  1
#define FALSE 0

#define FIRSTPT 1       /* checking whether pt. is inserted */
#define LASTPT  2


#define SEGSIZE 200000     /* max# of segments. Determines how */
                /* many points can be specified as */
                /* input. If your datasets have large */
                /* number of points, increase this */
                /* value accordingly. */
#define QSIZE   8*SEGSIZE   /* maximum table sizes */
#define TRSIZE  4*SEGSIZE   /* max# trapezoids */

#define T_X     1
#define T_Y     2
#define T_SINK  3

#define MYINFINITY 1<<30
#define C_EPS 1.0e-7        /* tolerance value: Used for making */
                /* all decisions about collinearity or */
                /* left/right of segment. Decrease */
                /* this value if the input points are */
                /* spaced very close together */
#define S_LEFT 1        /* for merge-direction */
#define S_RIGHT 2


#define ST_VALID 1      /* for trapezium state */
#define ST_INVALID 2


#define SP_SIMPLE_LRUP 1    /* for splitting trapezoids */
#define SP_SIMPLE_LRDN 2
#define SP_2UP_2DN     3
#define SP_2UP_LEFT    4
#define SP_2UP_RIGHT   5
#define SP_2DN_LEFT    6
#define SP_2DN_RIGHT   7
#define SP_NOSPLIT    -1

#define TR_FROM_UP 1        /* for traverse-direction */
#define TR_FROM_DN 2

#define TRI_LHS 1
#define TRI_RHS 2


//#define MAX(a, b) (((a) > (b)) ? (a) : (b))
//#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define CROSS(v0, v1, v2) (((v1).x - (v0).x)*((v2).y - (v0).y) - \
               ((v1).y - (v0).y)*((v2).x - (v0).x))

#define DOT(v0, v1) ((v0).x * (v1).x + (v0).y * (v1).y)

#define FP_EQUAL(s, t) (fabs(s - t) <= C_EPS)

#define CROSS_SINE(v0, v1) ((v0).x * (v1).y - (v1).x * (v0).y)
#define LENGTH(v0) (sqrt((v0).x * (v0).x + (v0).y * (v0).y))


typedef struct {
  double x, y;
} point_t, vector_t;


/* Segment attributes */

typedef struct {
  point_t v0, v1;       /* two endpoints */
  int is_inserted;      /* inserted in trapezoidation yet ? */
  int root0, root1;     /* root nodes in Q */
  int next;         /* Next logical segment */
  int prev;         /* Previous segment */
} segment_t;

/* Trapezoid attributes */

typedef struct {
  int lseg, rseg;       /* two adjoining segments */
  point_t hi, lo;       /* max/min y-values */
  int u0, u1;
  int d0, d1;
  int sink;         /* pointer to corresponding in Q */
  int usave, uside;     /* I forgot what this means */
  int state;
} trap_t;


/* Node attributes for every node in the query structure */

typedef struct {
  int nodetype;         /* Y-node or S-node */
  int segnum;
  point_t yval;
  int trnum;
  int parent;           /* doubly linked DAG */
  int left, right;      /* children */
}node_t;


typedef struct {
  int vnum;
  int next;         /* Circularly linked list  */
  int prev;         /* describing the monotone */
  int marked;           /* polygon */
} monchain_t;



typedef struct {
  point_t pt;
  int vnext[4];         /* next vertices for the 4 chains */
  int vpos[4];          /* position of v in the 4 chains */
  int nextfree;
} vertexchain_t;


int triangulate_polygon(int ncontours, int cntr[], vector<double> vertices_x,
vector<double> vertices_y, int (*triangles)[3]);

//#define DEBUG

#endif
