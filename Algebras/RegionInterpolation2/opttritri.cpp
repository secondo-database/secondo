/*
 opttritri.cpp
 
 Interim trapezium intersection test, used until specialTrapeziumIntersects
 in the MovingRegionAlgebra works properly. Change the define in the 
 file config.h accordingly then and delete this file!
 
 Based on a function from Tomas M[oe]ller:
 Tomas M[oe]ller. 1997. A fast triangle-triangle intersection test. J. Graph.
 Tools 2, 2 (November 1997), 25-30. DOI=10.1080/10867651.1997.10487472
 http://dx.doi.org/10.1080/10867651.1997.10487472
 
*/

#include "interpolate.h"

typedef double num;

#include <math.h>
#define FABS(x) (num(fabs(x))) 

#define USE_EPSILON_TEST TRUE
#define EPSILON 0.001


#define CROSS(dest,v1,v2){                     \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];}

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2){         \
            dest[0]=v1[0]-v2[0]; \
            dest[1]=v1[1]-v2[1]; \
            dest[2]=v1[2]-v2[2];}

#define SORT(a,b)       \
             if(a>b)    \
             {          \
               num c; \
               c=a;     \
               a=b;     \
               b=c;     \
             }


#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
      if(e>=0 && e<=f) return 1;                      \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return 1;                      \
    }                                                 \
  }

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  num Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
  Ax=V1[i0]-V0[i0];                            \
  Ay=V1[i1]-V0[i1];                            \
  /* test edge U0,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  num a,b,c,d0,d1,d2;                     \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=U1[i1]-U0[i1];                          \
  b=-(U1[i0]-U0[i0]);                       \
  c=-a*U0[i0]-b*U0[i1];                     \
  d0=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U2[i1]-U1[i1];                          \
  b=-(U2[i0]-U1[i0]);                       \
  c=-a*U1[i0]-b*U1[i1];                     \
  d1=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U0[i1]-U2[i1];                          \
  b=-(U0[i0]-U2[i0]);                       \
  c=-a*U2[i0]-b*U2[i1];                     \
  d2=a*V0[i0]+b*V0[i1]+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return 1;                 \
  }                                         \
}

static int coplanar_tri_tri(num N[3],num V0[3],num V1[3],num V2[3],
                     num U0[3],num U1[3],num U2[3])
{
   num A[3];
   short i0,i1;
   A[0]=FABS(N[0]);
   A[1]=FABS(N[1]);
   A[2]=FABS(N[2]);
   if(A[0]>A[1])
   {
      if(A[0]>A[2])
      {
          i0=1;
          i1=2;
      }
      else
      {
          i0=0;
          i1=1;
      }
   }
   else   /* A[0]<=A[1] */
   {
      if(A[2]>A[1])
      {
          i0=0;
          i1=1;
      }
      else
      {
          i0=0;
          i1=2;
      }
    }

    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);

    POINT_IN_TRI(V0,U0,U1,U2);
    POINT_IN_TRI(U0,V0,V1,V2);

    return 0;
}



#define NEWCOMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
{ \
        if(D0D1>0.0f) \
        { \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else if(D0D2>0.0f)\
        { \
            A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D1*D2>0.0f || D0!=0.0f) \
        { \
                A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
        } \
        else if(D1!=0.0f) \
        { \
                A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D2!=0.0f) \
        { \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else \
        { \
                return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2); \
        } \
}



static int TriangleIntersection(num V0[3],num V1[3],num V2[3],
                     num U0[3],num U1[3],num U2[3])
{
  num E1[3],E2[3];
  num N1[3],N2[3],d1,d2;
  num du0,du1,du2,dv0,dv1,dv2;
  num D[3];
  num isect1[2], isect2[2];
  num du0du1,du0du2,dv0dv1,dv0dv2;
  short index;
  num vp0,vp1,vp2;
  num up0,up1,up2;
  num bb,cc,max;

  SUB(E1,V1,V0);
  SUB(E2,V2,V0);
  CROSS(N1,E1,E2);
  d1=-DOT(N1,V0);
  /* plane equation 1: N1.X+d1=0 */

  du0=DOT(N1,U0)+d1;
  du1=DOT(N1,U1)+d1;
  du2=DOT(N1,U2)+d1;

#if USE_EPSILON_TEST==TRUE
  if(FABS(du0)<EPSILON) du0=0.0;
  if(FABS(du1)<EPSILON) du1=0.0;
  if(FABS(du2)<EPSILON) du2=0.0;
#endif
  du0du1=du0*du1;
  du0du2=du0*du2;

  if(du0du1>=0.0f && du0du2>=0.0f)
    return 0;

  SUB(E1,U1,U0);
  SUB(E2,U2,U0);
  CROSS(N2,E1,E2);
  d2=-DOT(N2,U0);

  dv0=DOT(N2,V0)+d2;
  dv1=DOT(N2,V1)+d2;
  dv2=DOT(N2,V2)+d2;

#if USE_EPSILON_TEST==TRUE
  if(FABS(dv0)<EPSILON) dv0=0.0;
  if(FABS(dv1)<EPSILON) dv1=0.0;
  if(FABS(dv2)<EPSILON) dv2=0.0;
#endif

  dv0dv1=dv0*dv1;
  dv0dv2=dv0*dv2;

  if(dv0dv1>=0.0f && dv0dv2>=0.0f)
    return 0;

  CROSS(D,N1,N2);

  max=(num)FABS(D[0]);
  index=0;
  bb=(num)FABS(D[1]);
  cc=(num)FABS(D[2]);
  if(bb>max) max=bb,index=1;
  if(cc>max) max=cc,index=2;

  vp0=V0[index];
  vp1=V1[index];
  vp2=V2[index];

  up0=U0[index];
  up1=U1[index];
  up2=U2[index];

  num a,b,c,x0,x1;
  NEWCOMPUTE_INTERVALS(vp0,vp1,vp2,dv0,dv1,dv2,dv0dv1,dv0dv2,a,b,c,x0,x1);

  num d,e,f,y0,y1;
  NEWCOMPUTE_INTERVALS(up0,up1,up2,du0,du1,du2,du0du1,du0du2,d,e,f,y0,y1);

  num xx,yy,xxyy,tmp;
  xx=x0*x1;
  yy=y0*y1;
  xxyy=xx*yy;

  tmp=a*xxyy;
  isect1[0]=tmp+b*x1*yy;
  isect1[1]=tmp+c*x0*yy;

  tmp=d*xxyy;
  isect2[0]=tmp+e*xx*y1;
  isect2[1]=tmp+f*xx*y0;

  SORT(isect1[0],isect1[1]);
  SORT(isect2[0],isect2[1]);
  
  if(isect1[1]<=isect2[0] || isect2[1]<=isect1[0]) return 0;
  return 1;
}

static bool _trapeziumIntersects2 (MSeg m, MSeg a) {
    if (!(m.is == m.ie) && !(m.fs == m.fe)) {
        MSeg ms1(m.is, m.ie, m.fs, m.fs);
        MSeg ms2(m.is, m.is, m.fs, m.fe);
        return _trapeziumIntersects2(ms1, a) || _trapeziumIntersects2(ms2, a);
    }

    if (!(a.is == a.ie) && !(a.fs == a.fe)) {
        MSeg ms1(a.is, a.ie, a.fs, a.fs);
        MSeg ms2(a.is, a.is, a.fs, a.fe);
        return _trapeziumIntersects2(m, ms1) || _trapeziumIntersects2(m, ms2);
    }

    num V0[3], V1[3], V2[3];
    num U0[3], U1[3], U2[3];

    if (m.is.x == m.ie.x && m.is.y == m.ie.y) {
        V0[0] = m.is.x;
        V0[1] = m.is.y;
        V0[2] = 0;
        V1[0] = m.fs.x;
        V1[1] = m.fs.y;
        V1[2] = 1;
        V2[0] = m.fe.x;
        V2[1] = m.fe.y;
        V2[2] = 1;
    } else if (m.fs.x == m.fe.x && m.fs.y == m.fe.y) {
        V0[0] = m.is.x;
        V0[1] = m.is.y;
        V0[2] = 0;
        V1[0] = m.ie.x;
        V1[1] = m.ie.y;
        V1[2] = 0;
        V2[0] = m.fs.x;
        V2[1] = m.fs.y;
        V2[2] = 1;
    } else {
        DEBUG(1, "ERROR: src-triangle is a trapezium!");
    }

    if (a.is.x == a.ie.x && a.is.y == a.ie.y) {
        U0[0] = a.is.x;
        U0[1] = a.is.y;
        U0[2] = 0;
        U1[0] = a.fs.x;
        U1[1] = a.fs.y;
        U1[2] = 1;
        U2[0] = a.fe.x;
        U2[1] = a.fe.y;
        U2[2] = 1;
    } else if (a.fs.x == a.fe.x && a.fs.y == a.fe.y) {
        U0[0] = a.is.x;
        U0[1] = a.is.y;
        U0[2] = 0;
        U1[0] = a.ie.x;
        U1[1] = a.ie.y;
        U1[2] = 0;
        U2[0] = a.fs.x;
        U2[1] = a.fs.y;
        U2[2] = 1;
    } else {
        DEBUG(1, "ERROR: dst-triangle is a trapezium!");
    }

    bool ret = TriangleIntersection(V0, V1, V2, U0, U1, U2);
    
    return ret;
}

bool trapeziumIntersects2 (MSeg m, MSeg a, unsigned int& detailedResult) {
    detailedResult = 0;
    
    return _trapeziumIntersects2(m, a);
}
