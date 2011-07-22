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

/*
template function compare

*/

inline bool operator == ( const Vect2D& c1, const Vect2D& c2 )
{
    if ( fabs( c1.X() - c2.X() ) < ZERO &&
         fabs( c1.Y() - c2.Y() ) < ZERO )

        return true;
    else
        return false;
}


inline bool operator < ( const Vect2D& c1, const Vect2D& c2 )
{
    if ( fabs( c1.X() - c2.X() ) > ZERO )
    {
        if ( c1.X() + ZERO < c2.X() )
            return true;
        else 
            return false;
    }
    else
    {
        if ( fabs( c1.Y() - c2.Y() ) > ZERO )
        {
            if ( c1.Y() + ZERO < c2.Y() )
                return true;
            else 
                return false;
        }
        else
        {
            return false;
        }
    }
}




inline bool operator == ( const Vect3D& c1, const Vect3D& c2 )
{
    if ( fabs( c1.X() - c2.X() ) < ZERO &&
         fabs( c1.Y() - c2.Y() ) < ZERO &&
         fabs( c1.Z() - c2.Z() ) < ZERO )

        return true;
    else
        return false;
}


inline bool operator < ( const Vect3D& c1, const Vect3D& c2 )
{
    if ( fabs( c1.X() - c2.X() ) > ZERO )
    {
        if ( c1.X() + ZERO < c2.X() )
            return true;
        else 
            return false;
    }
    else
    {
        if ( fabs( c1.Y() - c2.Y() ) > ZERO )
        {
            if ( c1.Y() + ZERO < c2.Y() )
                return true;
            else 
                return false;
        }
        else
        {
            if ( fabs( c1.Z() - c2.Z() ) > ZERO )
            {
                if ( c1.Z() + ZERO < c2.Z() )
                    return true;
                else 
                    return false;
            }
            else
            {
                return false;
            }
        }
    }
}




/*
specialization of the Vect operators for DIM=1,2,3,4

*/


//////////////////////////////////////////////////////////////////////
// Vect1D
//////////////////////////////////////////////////////////////////////
template <>
inline Vect1D operator*( const MGFloat& comp, const Vect1D& vec)
{
    return Vect1D( vec.mtab[0] * comp );
}


template <>
inline Vect1D operator*( const Vect1D& vec, const MGFloat& comp)
{
    return Vect1D( vec.mtab[0] * comp );
}


template <>
inline Vect1D operator/( const Vect1D& vec, const MGFloat& comp)
{
    return Vect1D( vec.mtab[0] / comp );
}


template <>
inline MGFloat operator*( const Vect1D& vec1, const Vect1D& vec2)
{
    return vec1.mtab[0] * vec2.mtab[0];
}


template <>
inline Vect1D operator+( const Vect1D& vec1, const Vect1D& vec2)
{
    return Vect1D( vec1.mtab[0] + vec2.mtab[0] );
}


template <>
inline Vect1D operator-( const Vect1D& vec1, const Vect1D& vec2)
{
    return Vect1D( vec1.mtab[0] - vec2.mtab[0] );
}




//////////////////////////////////////////////////////////////////////
// Vect2D
//////////////////////////////////////////////////////////////////////
template <>
inline Vect2D operator*( const MGFloat& comp, const Vect2D& vec)
{
    return Vect2D( vec.mtab[0] * comp, vec.mtab[1] * comp );
}


template <>
inline Vect2D operator*( const Vect2D& vec, const MGFloat& comp)
{
    return Vect2D( vec.mtab[0] * comp, vec.mtab[1] * comp );
}


template <>
inline Vect2D operator/( const Vect2D& vec, const MGFloat& comp)
{
    return Vect2D( vec.mtab[0] / comp, vec.mtab[1] / comp );
}


template <>
inline MGFloat operator*( const Vect2D& vec1, const Vect2D& vec2)
{
    return vec1.mtab[0] * vec2.mtab[0] + vec1.mtab[1] * vec2.mtab[1];
}


template <>
inline Vect2D operator+( const Vect2D& vec1, const Vect2D& vec2)
{
    return Vect2D( vec1.mtab[0] + vec2.mtab[0], vec1.mtab[1] + vec2.mtab[1] );
}


template <>
inline Vect2D operator-( const Vect2D& vec1, const Vect2D& vec2)
{
    return Vect2D( vec1.mtab[0] - vec2.mtab[0], vec1.mtab[1] - vec2.mtab[1] );
}


/* 
Vect3D

*/

template <>
inline Vect3D operator*( const MGFloat& comp, const Vect3D& vec)
{
    return Vect3D( vec.mtab[0] * comp, vec.mtab[1] * comp, vec.mtab[2] * comp );
}


template <>
inline Vect3D operator*( const Vect3D& vec, const MGFloat& comp)
{
    return Vect3D( vec.mtab[0] * comp, vec.mtab[1] * comp, vec.mtab[2] * comp );
}


template <>
inline Vect3D operator/( const Vect3D& vec, const MGFloat& comp)
{
    return Vect3D( vec.mtab[0] / comp, vec.mtab[1] / comp, vec.mtab[2] / comp );
}


template <>
inline MGFloat operator*( const Vect3D& vec1, const Vect3D& vec2)
{
    return vec1.mtab[0]*vec2.mtab[0] + vec1.mtab[1]*vec2.mtab[1] + 
    vec1.mtab[2]*vec2.mtab[2];
}


template <>
inline Vect3D operator+( const Vect3D& vec1, const Vect3D& vec2)
{
    return Vect3D( vec1.mtab[0] + vec2.mtab[0], vec1.mtab[1] + vec2.mtab[1], 
                   vec1.mtab[2] + vec2.mtab[2] );
}


template <>
inline Vect3D operator-( const Vect3D& vec1, const Vect3D& vec2)
{
    return Vect3D( vec1.mtab[0] - vec2.mtab[0], vec1.mtab[1] - vec2.mtab[1], 
                   vec1.mtab[2] - vec2.mtab[2] );
}


template <>
inline Vect3D operator%( const Vect3D& vec1, const Vect3D& vec2)
{
    MGFloat x,y,z;
    x = vec1.mtab[1]*vec2.mtab[2] - vec1.mtab[2]*vec2.mtab[1];
    y = vec1.mtab[2]*vec2.mtab[0] - vec1.mtab[0]*vec2.mtab[2];
    z = vec1.mtab[0]*vec2.mtab[1] - vec1.mtab[1]*vec2.mtab[0];
    return Vect3D(x,y,z);
}



/* 
Vect4D 

*/

template <>
inline Vect4D operator*( const MGFloat& comp, const Vect4D& vec)
{
    return Vect4D( vec.mtab[0] * comp, vec.mtab[1] * comp, vec.mtab[2] * comp, 
                   vec.mtab[3] * comp );
}


template <>
inline Vect4D operator*( const Vect4D& vec, const MGFloat& comp)
{
    return Vect4D( vec.mtab[0] * comp, vec.mtab[1] * comp, vec.mtab[2] * comp, 
                   vec.mtab[3] * comp );
}


template <>
inline Vect4D operator/( const Vect4D& vec, const MGFloat& comp)
{
    return Vect4D( vec.mtab[0] / comp, vec.mtab[1] / comp, vec.mtab[2] / comp, 
                   vec.mtab[3] / comp );
}


template <>
inline MGFloat operator*( const Vect4D& vec1, const Vect4D& vec2)
{
    return vec1.mtab[0]*vec2.mtab[0] + vec1.mtab[1]*vec2.mtab[1] + 
    vec1.mtab[2]*vec2.mtab[2] + vec1.mtab[3]*vec2.mtab[3];
}


template <>
inline Vect4D operator+( const Vect4D& vec1, const Vect4D& vec2)
{
    return Vect4D( vec1.mtab[0] + vec2.mtab[0], vec1.mtab[1] + 
    vec2.mtab[1], vec1.mtab[2] + vec2.mtab[2], vec1.mtab[3] + vec2.mtab[3] );
}


template <>
inline Vect4D operator-( const Vect4D& vec1, const Vect4D& vec2)
{
    return Vect4D( vec1.mtab[0] - vec2.mtab[0], vec1.mtab[1] - vec2.mtab[1], 
                   vec1.mtab[2] - vec2.mtab[2], vec1.mtab[3] - vec2.mtab[3] );
}

/*
operator function for template class vector

*/

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE,DIM> operator-( const Vect<ELEM_TYPE,DIM>& vec1, 
                                      const Vect<ELEM_TYPE,DIM>& vec2)
{
    Vect<ELEM_TYPE,DIM> v;
    for ( MGInt i=0; i<DIM; ++i)
        v.mtab[i] = vec1.mtab[i] - vec2.mtab[i];
    return v;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE,DIM> operator+( const Vect<ELEM_TYPE,DIM>& vec1, 
                                      const Vect<ELEM_TYPE,DIM>& vec2)
{
    Vect<ELEM_TYPE,DIM> v;
    for ( MGInt i=0; i<DIM; ++i)
        v.mtab[i] = vec1.mtab[i] + vec2.mtab[i];
    return v;
}

template <class ELEM_TYPE, Dimension DIM> 
inline ELEM_TYPE operator*( const Vect<ELEM_TYPE,DIM>& vec1, 
                            const Vect<ELEM_TYPE,DIM>& vec2)
{
    ELEM_TYPE   e(0);
    for ( MGInt i=0; i<DIM; ++i)
        e += vec1.mtab[i] * vec2.mtab[i];
    return e;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE,DIM> operator*( const ELEM_TYPE& e, 
                                      const Vect<ELEM_TYPE,DIM>& vec)
{
    Vect<ELEM_TYPE,DIM> v;
    for ( MGInt i=0; i<DIM; ++i)
        v.mtab[i] = e * vec.mtab[i];
    return v;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE,DIM> operator*( const Vect<ELEM_TYPE,DIM>& vec, 
                                      const ELEM_TYPE& e)
{
    Vect<ELEM_TYPE,DIM> v;
    for ( MGInt i=0; i<DIM; ++i)
        v.mtab[i] = e * vec.mtab[i];
    return v;
}

template <class ELEM_TYPE, Dimension DIM> 
inline Vect<ELEM_TYPE,DIM> operator/( const Vect<ELEM_TYPE,DIM>& vec, 
                                      const ELEM_TYPE& e)
{
    Vect<ELEM_TYPE,DIM> v;
    for ( MGInt i=0; i<DIM; ++i)
        v.mtab[i] = vec.mtab[i] / e;
    return v;
}


/* 
class HPolygon
structur for polygon triangulation 

*/

class HPolygon
{
public:
    HPolygon()      {}
    ~HPolygon()     {}

    void    Init( const char name[]);
    void    Triangulate();
    void    WriteTEC( const char name[]);


    int Triangulation2(int ncontours, int cntr[], 
                             vector<double>& vertices_x,
                   vector<double>& vertices_y);
    void    Init2(int ncontours, int cntr[], vector<double>& vertices_x,
                   vector<double>& vertices_y);
    int    OutPut();
    
    
    vector<Vect2D>  mtabPnt;
    vector<MGInt>   mtabSize;
    vector<HTri>    mtabCell;
    vector<int>     p_id_list;
    
};



#define THIS_FILE __FILE__  // defines name of header or implementation file


const MGInt EX_FILE_CODE        = 101;
const MGInt EX_MATH_CODE        = 102;
const MGInt EX_MEMORY_CODE      = 103;
const MGInt EX_INTERNAL_CODE    = 104;
const MGInt EX_ASSERT_CODE      = 105;
const MGInt EX_REXP_CODE        = 106;

const char EX_FILE_STR[]        = "FILE";
const char EX_MATH_STR[]        = "MATH";
const char EX_MEMORY_STR[]      = "MEM";
const char EX_INTERNAL_STR[]    = "INTERNAL";
const char EX_ASSERT_STR[]      = "ASSERT";
const char EX_REXP_STR[]        = "REXP";


// name of file used for tracing
const char TRACE_FILE_NAME[]    = "trace.txt";



/* 
class Trace, for debuging, trace the program

*/

class TRI_Trace
{
public:
    TRI_Trace() { FILE *ftrc = fopen( TRACE_FILE_NAME, "wt"); 
    Verify(ftrc); fclose( ftrc); }

    FILE*   Open() { FILE *ftrc = fopen( TRACE_FILE_NAME, "at"); 
    Verify(ftrc); return ftrc; }
    void    Close( FILE *f) { fclose( f);}
    void    Verify( FILE *f);
    void    Out( char *sfile, MGInt nline);
};

inline void TRI_Trace::Verify( FILE *f)
{
    if ( !f)
        printf( "mgtrace file '%s' opening error\n", TRACE_FILE_NAME);
}

inline void TRI_Trace::Out( char *sfile, MGInt nline)
{   
    FILE *ftrc = Open(); 
    fprintf( ftrc, "FILE:%s - LINE:%d;  ", sfile, nline);
    Close( ftrc);   
}


//////////////////////////////////////////////////////////////////////
// macros for traceing
//////////////////////////////////////////////////////////////////////

//-----------------------------------------
#ifdef _DEBUG
//-----------------------------------------
#define INIT_TRACE  TRI_Trace mgtrace

#define TM_TRACE(sz) \
    { \
        FILE *f = mgtrace.Open(); \
        mgtrace.Out(THIS_FILE, __LINE__); \
        fprintf( f, sz); \
        fprintf( f, "\n"); \
        mgtrace.Close( f); \
    }

#define TM_TRACE1(sz, x1) \
    { \
        FILE *f = mgtrace.Open(); \
        mgtrace.Out(THIS_FILE, __LINE__); \
        fprintf( f, sz, x1); \
        fprintf( f, "\n"); \
        mgtrace.Close( f); \
    }

#define TM_TRACE2(sz, x1, x2) \
    { \
        FILE *f = mgtrace.Open(); \
        mgtrace.Out(THIS_FILE, __LINE__); \
        fprintf( f, sz, x1, x2); \
        fprintf( f, "\n"); \
        mgtrace.Close( f); \
    }

#define TM_TRACE_EXCEPTION(e) \
    { \
        FILE *f = mgtrace.Open(); \
        (e).WriteInfo( f); \
        mgtrace.Close( f); \
    }


class Trace mgtrace;

//-----------------------------------------
#else // _DEBUG
//-----------------------------------------

#define INIT_TRACE

#define TM_TRACE(sz) \
    { \
    }

#define TM_TRACE1(sz, x1) \
    { \
    }

#define TM_TRACE2(sz, x1, x2) \
    { \
    }

#define TM_TRACE_EXCEPTION(e) \
    { \
    }

//-----------------------------------------
#endif // _DEBUG
//-----------------------------------------





#define TM_TRACE_TO_STDERR(e) \
    { \
        (e).WriteInfo( stderr); \
    }

#define TM_TRACE_TO_CERR TM_TRACE_TO_STDERR


/* 
class Except - base, abstract class for all exceptions

*/

class Except
{
public:
    Except():mComment(""),mFileName(""),mLineNo(0){}
    Except( const Except& ex) 
        : mComment(ex.mComment), mFileName(ex.mFileName), mLineNo(ex.mLineNo){};
        
    Except( MGString com, MGString fname, MGInt line) 
        : mComment(com), mFileName(fname), mLineNo(line) {};
        
    virtual ~Except()   {};

    Except& operator = (const Except& ex);
    
    virtual MGInt       GetExType()      const  = 0;
    virtual MGString    GetExPrefix()    const  = 0;
    
    virtual void        WriteInfo( FILE *f);

protected:
    MGString    mComment;   
    MGString    mFileName;
    MGInt       mLineNo;
    
};


inline Except& Except::operator = (const Except& ex)
{ 
    mFileName = ex.mFileName;
    mLineNo   = ex.mLineNo;
    mComment  = ex.mComment;
    return *this; 
}

inline void Except::WriteInfo( FILE *f)
{
    fprintf( f, "FILE:%s - LINE:%d;\n", mFileName.c_str(), mLineNo);
    fprintf( f, "    %s: %s\n", (GetExPrefix()).c_str(), mComment.c_str() );
}

/* 
class ExceptFile
  used when some problems occur with opening, reading and parsing
  information from files


*/

class ExceptFile : public Except
{   
protected:
    MGString    mFileInfo;
    
public:
    ExceptFile()                        { mFileInfo = "";}
    ExceptFile( MGString com, MGString info, MGString fname, MGInt line)
        : Except( com, fname, line)     {mFileInfo = info;};
    virtual ~ExceptFile()   {};
    
    virtual MGInt       GetExType()     const   { return EX_FILE_CODE;}
    virtual MGString    GetExPrefix()   const   { return EX_FILE_STR;}

    virtual void        WriteInfo( FILE *f);

};

inline void ExceptFile::WriteInfo( FILE *f)
{
    fprintf( f, "FILE:%s - LINE:%d;\n", mFileName.c_str(), mLineNo);
    fprintf( f, "    %s: %s '%s'\n", (GetExPrefix()).c_str(), 
             mComment.c_str(), mFileInfo.c_str() );
}



/* 
class ExceptMath

*/

class ExceptMath : public Except
{   
public:
    ExceptMath()                {};
    ExceptMath( MGString com, MGString fname, MGInt line) 
        : Except( com, fname, line) {};
        
    virtual ~ExceptMath()   {};
    
    virtual MGInt   GetExType()     const { return EX_MATH_CODE;}
    virtual MGString    GetExPrefix()   const { return EX_MATH_STR;}
};


/* 
class ExceptMem
  for exception caused by memory (problems with alloc, memory corrupt.)
  C++ bad alloc is switched off (at least for obj allocated with GGNEW)
  because badalloc does not return info. where except. occured

*/

class ExceptMem : public Except
{   
public:
    ExceptMem()             {};
    ExceptMem( MGString com, MGString fname, MGInt line) 
        : Except( com, fname, line) {};
        
    virtual ~ExceptMem()    {};
    
    virtual MGInt   GetExType()     const { return EX_MEMORY_CODE;}
    virtual MGString    GetExPrefix()   const { return EX_MEMORY_STR;}
};


/* 
class ExceptAssert used in ASSERT macro

*/

class ExceptAssert : public Except
{   
public:
    ExceptAssert()          {};
    ExceptAssert( MGString com, MGString fname, MGInt line) 
        : Except( com, fname, line) {};
        
    virtual ~ExceptAssert() {};
    
    virtual MGInt   GetExType()     const { return EX_ASSERT_CODE;}
    virtual MGString    GetExPrefix()   const { return EX_ASSERT_STR;}
};


/* 
class InternalException
  for handling abnormal events specific for mesh gen. program

*/

class ExceptInter : public Except
{
public:
    ExceptInter()           {};
    ExceptInter( MGString com, MGString fname, MGInt line) 
        : Except( com, fname, line) {};
        
    virtual ~ExceptInter()  {};
    
    virtual MGInt   GetExType()     const { return EX_INTERNAL_CODE;}
    virtual MGString    GetExPrefix()   const { return EX_INTERNAL_STR;}
};


/* 
class ExceptRExp
  for handling regular expresion exceptions

*/

class ExceptRExp : public Except
{   
public:
    ExceptRExp()                {};
    ExceptRExp( MGString com, MGString fname, MGInt line) 
        : Except( com, fname, line) {};
        
    virtual ~ExceptRExp()   {};
    
    virtual MGInt   GetExType()     const { return EX_REXP_CODE;}
    virtual MGString    GetExPrefix()   const { return EX_REXP_STR;}
};






/* 
macros for throwing Exceptions 

*/


#ifdef assert
    // if compiler provides a assert macro in "assert.h"
    #define ASSERT assert
#else
 // assert macro which does not depend on compiler; uses exception ExceptAssert
    #ifdef _DEBUG
        #define ASSERT(f) \
        { \
        if (!(f)) \
  throw new ExceptAssert( "assertion failed: '" #f "'", THIS_FILE, __LINE__ ); \
        }
    #else
        #define ASSERT(f) \
        { \
        }
    #endif // _DEBUG
#endif // assert


#define THROW_ALLOC(f) \
    { \
    if (!(f)) \
     throw new ExceptMem( "allocation error: '" #f "'", THIS_FILE, __LINE__ ); \
    }

#define THROW_MEMORY(f) \
    { \
        throw new ExceptMem( f, THIS_FILE, __LINE__ ); \
    }

#define THROW_FILE(f1, f2) \
    { \
        throw new ExceptFile( f1, f2, THIS_FILE, __LINE__ ); \
    }

#define THROW_MATH(f) \
    { \
        throw new ExceptMath( f, THIS_FILE, __LINE__ ); \
    }

#define THROW_INTERNAL(f) \
    { \
        throw new ExceptInter( f, THIS_FILE, __LINE__ ); \
    }

#define THROW_REGEXP(f) \
    { \
        throw new ExceptRExp( f, THIS_FILE, __LINE__ ); \
    }


/*
rectangle structure 

*/
class HRect
{
public:
    HRect() {}
    HRect( const MGFloat& xmin, const MGFloat& ymin,
          const MGFloat& xmax, const MGFloat& ymax) 
          : mvMin( Vect2D(xmin,ymin)), mvMax( Vect2D(xmax,ymax))    {}

    ~HRect()    {}

    void ExportTEC( FILE *f);

    bool    IsInside( const Vect2D& vct) const;
    bool    IsOverlapping( const HRect& rec) const;
    
    Vect2D  Center()                { return (mvMin + mvMax)/2.0;}

    const Vect2D&   VMin() const    { return mvMin;}
          Vect2D&   rVMin()         { return mvMin;}
          
    const Vect2D&   VMax() const    { return mvMax;}
          Vect2D&   rVMax()         { return mvMax;}

    const MGFloat&  XMin() const    { return mvMin.X();}
    const MGFloat&  YMin() const    { return mvMin.Y();}
    const MGFloat&  XMax() const    { return mvMax.X();}
    const MGFloat&  YMax() const    { return mvMax.Y();}
          MGFloat&  rXMin()         { return mvMin.rX();}
          MGFloat&  rYMin()         { return mvMin.rY();}
          MGFloat&  rXMax()         { return mvMax.rX();}
          MGFloat&  rYMax()         { return mvMax.rY();}

    
protected:
    Vect2D  mvMin;
    Vect2D  mvMax;
};


inline bool HRect::IsInside( const Vect2D& vct) const
{
    if ( vct.X() <= mvMax.X() && vct.X() >= mvMin.X() &&
         vct.Y() <= mvMax.Y() && vct.Y() >= mvMin.Y() )

        return true;
    else
        return false;
}

inline bool HRect::IsOverlapping( const HRect& rec) const
{
    if ( rec.IsInside( Vect2D( XMin(), YMin() )) ||
         rec.IsInside( Vect2D( XMax(), YMin() )) ||
         rec.IsInside( Vect2D( XMin(), YMax() )) ||
         rec.IsInside( Vect2D( XMax(), YMax() )) )
    {
        return true;
    }
    else if ( IsInside( Vect2D( rec.XMin(), rec.YMin() )) ||
              IsInside( Vect2D( rec.XMax(), rec.YMin() )) ||
              IsInside( Vect2D( rec.XMin(), rec.YMax() )) ||
              IsInside( Vect2D( rec.XMax(), rec.YMax() )) )
    {
        return true;
    }
    else if ( rec.XMin() > XMin() && rec.XMax() < XMax() &&
              rec.YMin() < YMin() && rec.YMax() > YMax() )
    {
        return true;
    }
    else if ( rec.XMin() < XMin() && rec.XMax() > XMax() &&
              rec.YMin() > YMin() && rec.YMax() < YMax() )
    {
        return true;
    }
    else
        return false;
}




inline void HRect::ExportTEC( FILE *f)
{
    fprintf( f, "VARIABLES = \"X\",\"Y\"\n");
    fprintf( f, "ZONE I=%d, F=POINT\n", 5);
    fprintf( f, "%lg %lg\n", mvMin.X(), mvMin.Y() );
    fprintf( f, "%lg %lg\n", mvMax.X(), mvMin.Y() );
    fprintf( f, "%lg %lg\n", mvMax.X(), mvMax.Y() );
    fprintf( f, "%lg %lg\n", mvMin.X(), mvMax.Y() );
    fprintf( f, "%lg %lg\n", mvMin.X(), mvMin.Y() );
}



#endif
