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
#include <string.h>
#include <stdio.h>

#include <vector>
#include <list>
#include <map>
#include <stack>

#include <stdlib.h>
#include <stdarg.h>

using namespace std;

#define TRUE  1
#define FALSE 0

#define FIRSTPT 1       /* checking whether pt. is inserted */
#define LASTPT  2


//#define SEGSIZE 500000

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


//////////////////////////////////////////////////////////////////////////////
/////////// another implementation of triangulation /////////////////////////
/////////// 2011.7 from code project/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#define ASSERT assert

enum Dimension
{
    DIM_NONE    = 0,
    DIM_1D      = 1,
    DIM_2D      = 2,
    DIM_3D      = 3,
    DIM_4D      = 4
};

#define MGNEW new           // old standard

// definition of basic types
typedef double      MGFloat;
typedef int         MGInt;
typedef string      MGString;


typedef vector<MGFloat> MGFloatArr;
typedef vector<MGInt>   MGIntArr;


//const MGFloat ZERO = 1.0e-12;
//const MGFloat ZERO = 1.0e-7;
const MGFloat ZERO = 1.0e-8;


// constants below have to be defined by preprocessor command #define
// because of dupliate naming in UNIX system; In UNIX those constants are 
// already defined using preprocessor
#ifndef M_E
#define M_E         2.7182818284590452354
#endif
#ifndef M_LOG2E
#define M_LOG2E     1.4426950408889634074
#endif
#ifndef M_LOG10E
#define M_LOG10E    0.43429448190325182765
#endif
#ifndef M_LN2
#define M_LN2       0.69314718055994530942
#endif
#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_1_PI
#define M_1_PI      0.31830988618379067154
#endif
#ifndef M_PI_4
#define M_PI_4      0.78539816339744830962
#endif
#ifndef M_2_PI
#define M_2_PI      0.63661977236758134308
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.70710678118654752440
#endif

#ifndef TRI_PI                      // as in stroustrup 
#define TRI_PI  M_PI
#endif
#ifndef TRI_PI2
#define TRI_PI2  M_PI_2
#endif



//////////////////////////////////////////////////////////////////////
// class GlobDim
//////////////////////////////////////////////////////////////////////
class GlobDim
{
public:
    GlobDim()                       { mDim = DIM_NONE;}
    GlobDim( const Dimension& dim)  { assert( mDim == DIM_NONE); mDim = dim;}
    
    static const Dimension& Dim()   { return mDim;}
    static Dimension&       rDim()  { return mDim;}
    
protected:
    static Dimension    mDim;
};


/* 
class HTri for triangulation

*/

class HTri
{
public:
    const MGInt&    Index( const MGInt& i) const    { return mtabInd[i];}
    MGInt&          rIndex( const MGInt& i)         { return mtabInd[i];}

public:
    MGInt   mtabInd[3];
};


/*
template function for 1D,2D,3D,4D

*/
template <class ELEM_TYPE, Dimension DIM> 
class Vect;

template <class ELEM_TYPE, Dimension DIM> 
Vect<ELEM_TYPE,DIM> operator *( const ELEM_TYPE&,  const Vect<ELEM_TYPE,DIM>&);

template <class ELEM_TYPE, Dimension DIM> 
Vect<ELEM_TYPE,DIM> operator *( const Vect<ELEM_TYPE,DIM>&, const ELEM_TYPE&);

template <class ELEM_TYPE, Dimension DIM> 
Vect<ELEM_TYPE,DIM> operator /( const Vect<ELEM_TYPE,DIM>&, const ELEM_TYPE&);

template <class ELEM_TYPE, Dimension DIM> 
ELEM_TYPE   operator *( const Vect<ELEM_TYPE,DIM>&, const Vect<ELEM_TYPE,DIM>&);
// dot product

template <class ELEM_TYPE, Dimension DIM> 
Vect<ELEM_TYPE,DIM> operator +( const Vect<ELEM_TYPE,DIM>&, 
                                const Vect<ELEM_TYPE,DIM>&);

template <class ELEM_TYPE, Dimension DIM> 
Vect<ELEM_TYPE,DIM> operator -( const Vect<ELEM_TYPE,DIM>&, 
                                const Vect<ELEM_TYPE,DIM>&);

template <class ELEM_TYPE, Dimension DIM> 
Vect<ELEM_TYPE,DIM> operator %( const Vect<ELEM_TYPE,DIM>&, 
                                const Vect<ELEM_TYPE,DIM>&);   
                                // vector product




//////////////////////////////////////////////////////////////////////
//  class Vect
//////////////////////////////////////////////////////////////////////

template <class ELEM_TYPE, Dimension DIM> 
class Vect
{
public:
    //////////////////////////////////////////////////////////////////
    // constructors
    Vect( const ELEM_TYPE& x);
    Vect( const ELEM_TYPE& x, const ELEM_TYPE& y);
    Vect( const ELEM_TYPE& x, const ELEM_TYPE& y, const ELEM_TYPE& z);
    Vect( const ELEM_TYPE& x, const ELEM_TYPE& y, const ELEM_TYPE& z, 
          const ELEM_TYPE& w);
    Vect( const Vect<ELEM_TYPE, DIM>& vec);
    Vect();

    //////////////////////////////////////////////////////////////////
    // declaration of friend two argument operators
    friend Vect<ELEM_TYPE, DIM> operator  *<>( const ELEM_TYPE&,  
                                               const Vect<ELEM_TYPE, DIM>&);
    friend Vect<ELEM_TYPE, DIM> operator  *<>( const Vect<ELEM_TYPE, DIM>&, 
                                               const ELEM_TYPE&);
    friend Vect<ELEM_TYPE, DIM> operator  /<>( const Vect<ELEM_TYPE, DIM>&,
                                               const ELEM_TYPE&);
    friend ELEM_TYPE            operator  *<>( const Vect<ELEM_TYPE, DIM>&, 
                                               const Vect<ELEM_TYPE, DIM>&);
                                               //scalar mult.
    friend Vect<ELEM_TYPE, DIM> operator  +<>( const Vect<ELEM_TYPE, DIM>&, 
                                               const Vect<ELEM_TYPE, DIM>&);
    friend Vect<ELEM_TYPE, DIM> operator  -<>( const Vect<ELEM_TYPE, DIM>&, 
                                               const Vect<ELEM_TYPE, DIM>&);
    friend Vect<ELEM_TYPE, DIM> operator  %<>( const Vect<ELEM_TYPE, DIM>&, 
                                               const Vect<ELEM_TYPE, DIM>&);   
                                               //Vect mult.

    //////////////////////////////////////////////////////////////////
    // one argument operators
    Vect<ELEM_TYPE, DIM>&   operator  =( const Vect<ELEM_TYPE, DIM> &vec);
    Vect<ELEM_TYPE, DIM>&   operator +=( const Vect<ELEM_TYPE, DIM>&);
    Vect<ELEM_TYPE, DIM>&   operator -=( const Vect<ELEM_TYPE, DIM>&);
    Vect<ELEM_TYPE, DIM>&   operator *=( const ELEM_TYPE&);
    Vect<ELEM_TYPE, DIM>&   operator /=( const ELEM_TYPE&);

    //////////////////////////////////////////////////////////////////
    // misc functions
    ELEM_TYPE               module() const;
    Vect<ELEM_TYPE, DIM>    versor() const;
    
    
    const ELEM_TYPE&    X( const MGInt& i) const    { return mtab[i];}
    ELEM_TYPE&          rX( const MGInt& i)         { return mtab[i];}

    const ELEM_TYPE&    X() const   { return mtab[0];}
    const ELEM_TYPE&    Y() const   { ASSERT(DIM>1); return mtab[1];}
    const ELEM_TYPE&    Z() const   { ASSERT(DIM>2); return mtab[2];}
    const ELEM_TYPE&    W() const   { ASSERT(DIM>3); return mtab[3];}
    
    ELEM_TYPE&          rX()        { return mtab[0];}
    ELEM_TYPE&          rY()        { ASSERT(DIM>1); return mtab[1];}
    ELEM_TYPE&          rZ()        { ASSERT(DIM>2); return mtab[2];}
    ELEM_TYPE&          rW()        { ASSERT(DIM>3); return mtab[3];}

protected:
    ELEM_TYPE mtab[DIM];
};


//////////////////////////////////////////////////////////////////////
typedef Vect<MGFloat,DIM_1D> Vect1D;
typedef Vect<MGFloat,DIM_2D> Vect2D;
typedef Vect<MGFloat,DIM_3D> Vect3D;
typedef Vect<MGFloat,DIM_4D> Vect4D;




/*
template insert function

*/

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>::Vect( const Vect<ELEM_TYPE, DIM> &vec)
{
    ASSERT( DIM>0 && DIM<5);
    for ( MGInt i=0; i<DIM; ++i)
        mtab[i] = vec.mtab[i];
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>::Vect( const ELEM_TYPE& x)
{
        ASSERT( DIM == DIM_1D);
        mtab[0] = x;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>::Vect( const ELEM_TYPE& x, const ELEM_TYPE& y)
{
        ASSERT( DIM == DIM_2D);
        mtab[0] = x;
        mtab[1] = y;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>::Vect( const ELEM_TYPE& x, const ELEM_TYPE& y, 
                                   const ELEM_TYPE& z)
{
        ASSERT( DIM == DIM_3D);
        mtab[0] = x;
        mtab[1] = y;
        mtab[2] = z;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>::Vect( const ELEM_TYPE& x, const ELEM_TYPE& y, 
                                   const ELEM_TYPE& z, const ELEM_TYPE& w)
{
        ASSERT( DIM == DIM_4D);
        mtab[0] = x;
        mtab[1] = y;
        mtab[2] = z;
        mtab[3] = w;
}


template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>::Vect()
{
    ASSERT( DIM > DIM_NONE && DIM <= DIM_4D);
    for ( MGInt i=0; i<DIM; mtab[i++]=(ELEM_TYPE)0.0);
}


template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>& Vect<ELEM_TYPE, DIM>::operator =( 
const Vect<ELEM_TYPE, DIM> &vec)
{
    for ( MGInt i=0; i<DIM; ++i)
        mtab[i] = vec.mtab[i];
    return *this;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>& Vect<ELEM_TYPE, DIM>::operator+=( 
const Vect<ELEM_TYPE, DIM>& vec)
{
    for ( MGInt i=0; i<DIM; ++i)
        mtab[i] += vec.mtab[i];
    return *this;
}


template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>& Vect<ELEM_TYPE, DIM>::operator-=( 
const Vect<ELEM_TYPE, DIM>& vec)
{
    for ( MGInt i=0; i<DIM; ++i)
        mtab[i] -= vec.mtab[i];
    return *this;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>& Vect<ELEM_TYPE, DIM>::operator*=( 
const ELEM_TYPE& doub)
{
    for ( MGInt i=0; i<DIM; ++i)
        mtab[i] *= doub;
    return *this;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM>& Vect<ELEM_TYPE, DIM>::operator/=( 
const ELEM_TYPE& doub)
{
    for ( MGInt i=0; i<DIM; ++i)
        mtab[i] /= doub;
    return *this;
}

template <class ELEM_TYPE, Dimension DIM> 
inline ELEM_TYPE Vect<ELEM_TYPE, DIM>::module() const
{
    return sqrt( (*this)*(*this));
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE, DIM> Vect<ELEM_TYPE, DIM>::versor() const
{
    return (*this / module() );
}


#endif
