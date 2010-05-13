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

[1] Source File of the Transportation Mode Algebra

May, 2010 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations of
doing triangulation for polygon with and without holes.
The original implementation is from Atul Narkhede and Dinesh Manocha

[TOC]

1 Overview

2 Defines and includes

*/

#include "Triangulate.h"


static int choose_idx;
static int permute[SEGSIZE];

node_t qs[QSIZE];       /* Query structure */
trap_t tr[TRSIZE];      /* Trapezoid structure */
segment_t seg[SEGSIZE];     /* Segment table */

static int q_idx;
static int tr_idx;


/* Table to hold all the monotone
   polygons . Each monotone polygon
   is a circularly linked list */
static monchain_t mchain[TRSIZE];

/* chain init. information. This
   is used to decide which
   monotone polygon to split if
   there are several other
   polygons touching at the same
   vertex  */
static vertexchain_t vert[SEGSIZE];

/* contains position of any vertex in
   the monotone chain for the polygon */
static int mon[SEGSIZE];
static int visited[TRSIZE];
static int chain_idx, op_idx, mon_idx;


/*
Generate a random permutation of the segments 1..n

*/
int generate_random_ordering(int n)
{
  struct timeval tval;
  struct timezone tzone;
  register int i;
  int m, st[SEGSIZE], *p;

  choose_idx = 1;
  gettimeofday(&tval, &tzone);
  srand48(tval.tv_sec);

  for (i = 0; i <= n; i++)
    st[i] = i;

  p = st;
  for (i = 1; i <= n; i++, p++)
    {
//      m = lrand48() % (n + 1 - i) + 1;
      m = i % (n + 1 - i) + 1;
      permute[i] = p[m];
      if (m != 1)
    p[m] = p[1];
    }
  return 0;
}

static int initialise(int n)
{
  register int i;

  for (i = 1; i <= n; i++)
    seg[i].is_inserted = FALSE;

  generate_random_ordering(n);

  return 0;
}


/*
Return the next segment in the generated random ordering of all the
segments in S

*/

int choose_segment()
{
//  int i;

#ifdef DEBUG
  fprintf(stderr, "choose_segment: %d\n", permute[choose_idx]);
#endif
  return permute[choose_idx++];
}

/*
Return a new node to be added into the query tree

*/
static int newnode()
{
  if (q_idx < QSIZE)
    return q_idx++;
  else
    {
      fprintf(stderr, "newnode: Query-table overflow\n");
      return -1;
    }
}

/*
Return the maximum of the two points into the yval structure

*/
static int _max(point_t *yval, point_t *v0, point_t *v1)
{
  if (v0->y > v1->y + C_EPS)
    *yval = *v0;
  else if (FP_EQUAL(v0->y, v1->y))
    {
      if (v0->x > v1->x + C_EPS)
    *yval = *v0;
      else
    *yval = *v1;
    }
  else
    *yval = *v1;

  return 0;
}


/*
Return the minimum of the two points into the yval structure

*/
static int _min(point_t *yval, point_t *v0, point_t *v1)
{
  if (v0->y < v1->y - C_EPS)
    *yval = *v0;
  else if (FP_EQUAL(v0->y, v1->y))
    {
      if (v0->x < v1->x)
    *yval = *v0;
      else
    *yval = *v1;
    }
  else
    *yval = *v1;

  return 0;
}

/*
Return a free trapezoid

*/
static int newtrap()
{
  if (tr_idx < TRSIZE)
    {
      tr[tr_idx].lseg = -1;
      tr[tr_idx].rseg = -1;
      tr[tr_idx].state = ST_VALID;
      return tr_idx++;
    }
  else
    {
      fprintf(stderr, "newtrap: Trapezoid-table overflow\n");
      return -1;
    }
}

/*
 Initilialise the query structure (Q) and the trapezoid table (T)
  when the first segment is added to start the trapezoidation. The
  query-tree starts out with 4 trapezoids, one S-node and 2 Y-nodes

*/

static int init_query_structure(int segnum)

{
  int i1, i2, i3, i4, i5, i6, i7, root;
  int t1, t2, t3, t4;
  segment_t *s = &seg[segnum];

  q_idx = tr_idx = 1;
  memset((void *)tr, 0, sizeof(tr));
  memset((void *)qs, 0, sizeof(qs));

  i1 = newnode();
  qs[i1].nodetype = T_Y;
  _max(&qs[i1].yval, &s->v0, &s->v1); /* root */
  root = i1;

  qs[i1].right = i2 = newnode();
  qs[i2].nodetype = T_SINK;
  qs[i2].parent = i1;

  qs[i1].left = i3 = newnode();
  qs[i3].nodetype = T_Y;
  _min(&qs[i3].yval, &s->v0, &s->v1); /* root */
  qs[i3].parent = i1;

  qs[i3].left = i4 = newnode();
  qs[i4].nodetype = T_SINK;
  qs[i4].parent = i3;

  qs[i3].right = i5 = newnode();
  qs[i5].nodetype = T_X;
  qs[i5].segnum = segnum;
  qs[i5].parent = i3;

  qs[i5].left = i6 = newnode();
  qs[i6].nodetype = T_SINK;
  qs[i6].parent = i5;

  qs[i5].right = i7 = newnode();
  qs[i7].nodetype = T_SINK;
  qs[i7].parent = i5;

  t1 = newtrap();       /* middle left */
  t2 = newtrap();       /* middle right */
  t3 = newtrap();       /* bottom-most */
  t4 = newtrap();       /* topmost */

  tr[t1].hi = tr[t2].hi = tr[t4].lo = qs[i1].yval;
  tr[t1].lo = tr[t2].lo = tr[t3].hi = qs[i3].yval;
  tr[t4].hi.y = (double) (MYINFINITY);
  tr[t4].hi.x = (double) (MYINFINITY);
  tr[t3].lo.y = (double) -1* (MYINFINITY);
  tr[t3].lo.x = (double) -1* (MYINFINITY);
  tr[t1].rseg = tr[t2].lseg = segnum;
  tr[t1].u0 = tr[t2].u0 = t4;
  tr[t1].d0 = tr[t2].d0 = t3;
  tr[t4].d0 = tr[t3].u0 = t1;
  tr[t4].d1 = tr[t3].u1 = t2;

  tr[t1].sink = i6;
  tr[t2].sink = i7;
  tr[t3].sink = i4;
  tr[t4].sink = i2;

  tr[t1].state = tr[t2].state = ST_VALID;
  tr[t3].state = tr[t4].state = ST_VALID;

  qs[i2].trnum = t4;
  qs[i4].trnum = t3;
  qs[i6].trnum = t1;
  qs[i7].trnum = t2;

  s->is_inserted = TRUE;
  return root;
}

int math_N(int n, int h)
{
  register int i;
  double v;

  for (i = 0, v = (int) n; i < h; i++)
    v = log2(v);

  return (int) ceil((double) 1.0*n/v);
}

/*Get log*n for given n*/

int math_logstar_n(int n)
{
  register int i;
  double v;

  for (i = 0, v = (double) n; v >= 1; i++)
    v = log2(v);

  return (i - 1);
}

int _greater_than(point_t *v0, point_t *v1)
{
  if (v0->y > v1->y + C_EPS)
    return TRUE;
  else if (v0->y < v1->y - C_EPS)
    return FALSE;
  else
    return (v0->x > v1->x);
}


int _equal_to(point_t *v0, point_t *v1)

{
  return (FP_EQUAL(v0->y, v1->y) && FP_EQUAL(v0->x, v1->x));
}

int _greater_than_equal_to(point_t *v0, point_t *v1)

{
  if (v0->y > v1->y + C_EPS)
    return TRUE;
  else if (v0->y < v1->y - C_EPS)
    return FALSE;
  else
    return (v0->x >= v1->x);
}

int _less_than(point_t *v0, point_t *v1)

{
  if (v0->y < v1->y - C_EPS)
    return TRUE;
  else if (v0->y > v1->y + C_EPS)
    return FALSE;
  else
    return (v0->x < v1->x);
}

/*
Returns true if the corresponding endpoint of the given segment is
 already inserted into the segment tree. Use the simple test of
 whether the segment which shares this endpoint is already inserted

*/

static int inserted(int segnum, int whichpt)

{
  if (whichpt == FIRSTPT)
    return seg[seg[segnum].prev].is_inserted;
  else
    return seg[seg[segnum].next].is_inserted;
}

