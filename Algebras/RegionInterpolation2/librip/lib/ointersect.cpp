/*
   1 ointersect.cpp: 3D trapezium intersection test.
 
   Based on a function from Tomas M[oe]ller:
   Tomas M[oe]ller. 1997. A fast triangle-triangle intersection test. J. Graph.
   Tools 2, 2 (November 1997), 25-30. DOI=10.1080/10867651.1997.10487472
   http://dx.doi.org/10.1080/10867651.1997.10487472

*/

#include "interpolate.h"


#include <math.h>

#define EPSILON 0.001

/*
   1.1 ~Vec~
   Helper class for 3d vector operations.

*/
class Vec {
public:
    double x, y, z;

    Vec() : x(0), y(0), z(0) {
    }

    Vec(double x, double y, double z) : x(x), y(y), z(z) {
    }

    Vec operator+(const Vec &v) const {
        return Vec(x + v.x, y + v.y, z + v.z);
    }

    Vec operator-(const Vec &v) const {
        return Vec(x - v.x, y - v.y, z - v.z);
    }

    double operator*(const Vec &v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec operator&(const Vec &v) const {
        return Vec(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
    double operator[](int index) const {
        return (index == 0) ? x : ((index == 1) ? y : z);
    }

    Vec abs() const {
        return Vec(fabs(x), fabs(y), fabs(z));
    }
};

static inline void sort(double v[]) {
    if (v[0] > v[1]) {
        double tmp = v[0];
        v[0] = v[1];
        v[1] = tmp;
    }
}

static inline bool compute_intervals(double vv0, double vv1, double vv2,
        double d0, double d1, double d2, double d0d1, double d0d2,
        double &a, double &b, double &c, double &x0, double &x1) {
    if (d0d1 > 0) {
        a = vv2;
        b = (vv0 - vv2) * d2;
        c = (vv1 - vv2) * d2;
        x0 = d2 - d0;
        x1 = d2 - d1;
    } else if (d0d2 > 0) {
        a = vv1;
        b = (vv0 - vv1) * d1;
        c = (vv2 - vv1) * d1;
        x0 = d1 - d0;
        x1 = d1 - d2;
    } else if (d1 * d2 > 0 || d0) {
        a = vv0;
        b = (vv1 - vv0) * d0;
        c = (vv2 - vv0) * d0;
        x0 = d0 - d1;
        x1 = d0 - d2;
    } else if (d1) {
        a = vv1;
        b = (vv0 - vv1) * d1;
        c = (vv2 - vv1) * d1;
        x0 = d1 - d0;
        x1 = d1 - d2;
    } else if (d2) {
        a = vv2;
        b = (vv0 - vv2) * d2;
        c = (vv1 - vv2) * d2;
        x0 = d2 - d0;
        x1 = d2 - d1;
    } else {
        return false;
    }
    return true;
}

static bool TriangleIntersection(Vec v0, Vec v1, Vec v2,
        Vec u0, Vec u1, Vec u2) {
    Vec e1, e2;
    Vec n1, n2;
    Vec dd;
    double d1, d2;
    double du0, du1, du2, dv0, dv1, dv2;
    double isect1[2], isect2[2];
    double du0du1, du0du2, dv0dv1, dv0dv2;
    double vp0, vp1, vp2;
    double up0, up1, up2;
    double bb, cc, max;

    e1 = v1 - v0;
    e2 = v2 - v0;
    n1 = e1 & e2;
    d1 = -(n1 * v0);

    du0 = n1 * u0 + d1;
    du1 = n1 * u1 + d1;
    du2 = n1 * u2 + d1;

#ifdef EPSILON
    if (fabs(du0) < EPSILON) du0 = 0.0;
    if (fabs(du1) < EPSILON) du1 = 0.0;
    if (fabs(du2) < EPSILON) du2 = 0.0;
#endif
    du0du1 = du0 * du1;
    du0du2 = du0 * du2;

    if (du0du1 >= 0 && du0du2 >= 0)
        return 0;

    e1 = u1 - u0;
    e2 = u2 - u0;
    n2 = e1 & e2;
    d2 = -(n2 * u0);

    dv0 = n2 * v0 + d2;
    dv1 = n2 * v1 + d2;
    dv2 = n2 * v2 + d2;

#ifdef EPSILON
    if (fabs(dv0) < EPSILON) dv0 = 0.0;
    if (fabs(dv1) < EPSILON) dv1 = 0.0;
    if (fabs(dv2) < EPSILON) dv2 = 0.0;
#endif

    dv0dv1 = dv0*dv1;
    dv0dv2 = dv0*dv2;

    if (dv0dv1 >= 0.0f && dv0dv2 >= 0.0f)
        return 0;

    dd = n1 & n2;

    max = fabs(dd.x);
    vp0 = v0.x;
    vp1 = v1.x;
    vp2 = v2.x;
    up0 = u0.x;
    up1 = u1.x;
    up2 = u2.x;
    bb = fabs(dd.y);
    cc = fabs(dd.z);
    if (bb > max) {
        max = bb;
        vp0 = v0.y;
        vp1 = v1.y;
        vp2 = v2.y;
        up0 = u0.y;
        up1 = u1.y;
        up2 = u2.y;
    }
    if (cc > max) {
        max = cc;
        vp0 = v0.z;
        vp1 = v1.z;
        vp2 = v2.z;
        up0 = u0.z;
        up1 = u1.z;
        up2 = u2.z;
    }

    double a, b, c, d, e, f, x0, y0, x1, y1;
    if (!compute_intervals(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2,
            a, b, c, x0, x1) ||
        !compute_intervals(up0, up1, up2, du0, du1, du2, du0du1, du0du2,
            d, e, f, y0, y1)) {
        return false;
    }

    double xx, yy, xxyy, tmp;
    xx = x0*x1;
    yy = y0*y1;
    xxyy = xx*yy;

    tmp = a*xxyy;
    isect1[0] = tmp + b * x1*yy;
    isect1[1] = tmp + c * x0*yy;

    tmp = d*xxyy;
    isect2[0] = tmp + e * xx*y1;
    isect2[1] = tmp + f * xx*y0;

    sort(isect1);
    sort(isect2);

    return (isect1[1] > isect2[0] && isect2[1] > isect1[0]);
}

static bool _trapeziumIntersects(MSeg m, MSeg a) {
    if (!(m.is == m.ie) && !(m.fs == m.fe)) {
        MSeg ms1(m.is, m.ie, m.fs, m.fs);
        MSeg ms2(m.is, m.is, m.fs, m.fe);
        return _trapeziumIntersects(ms1, a) || _trapeziumIntersects(ms2, a);
    }

    if (!(a.is == a.ie) && !(a.fs == a.fe)) {
        MSeg ms1(a.is, a.ie, a.fs, a.fs);
        MSeg ms2(a.is, a.is, a.fs, a.fe);
        return _trapeziumIntersects(m, ms1) || _trapeziumIntersects(m, ms2);
    }

    Vec v0, v1, v2;
    Vec u0, u1, u2;

    if (m.is.x == m.ie.x && m.is.y == m.ie.y) {
        v0 = Vec(m.is.x, m.is.y, 0);
        v1 = Vec(m.fs.x, m.fs.y, 1);
        v2 = Vec(m.fe.x, m.fe.y, 1);
    } else if (m.fs.x == m.fe.x && m.fs.y == m.fe.y) {
        v0 = Vec(m.is.x, m.is.y, 0);
        v1 = Vec(m.ie.x, m.ie.y, 0);
        v2 = Vec(m.fs.x, m.fs.y, 1);
    } else {
        cerr << "ERROR: src-triangle is a trapezium!\n";
        assert(false);
    }

    if (a.is.x == a.ie.x && a.is.y == a.ie.y) {
        u0 = Vec(a.is.x, a.is.y, 0);
        u1 = Vec(a.fs.x, a.fs.y, 1);
        u2 = Vec(a.fe.x, a.fe.y, 1);
    } else if (a.fs.x == a.fe.x && a.fs.y == a.fe.y) {
        u0 = Vec(a.is.x, a.is.y, 0);
        u1 = Vec(a.ie.x, a.ie.y, 0);
        u2 = Vec(a.fs.x, a.fs.y, 1);
    } else {
        cerr << "ERROR: dst-triangle is a trapezium!\n";
        assert(false);
    }

    bool ret = TriangleIntersection(v0, v1, v2, u0, u1, u2);

    return ret;
}

/*
  1.2 ~trapeziumIntersects~ checks, if the moving segments ~m~ and ~a~,
  which can be taken as 3d trapezoids, intersect.

*/
bool trapeziumIntersects(MSeg m, MSeg a, unsigned int& detailedResult) {
    detailedResult = 0;

    return _trapeziumIntersects(m, a);
}
