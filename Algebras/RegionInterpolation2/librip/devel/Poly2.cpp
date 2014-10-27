/*
 1 Poly.cpp is an interface for external polygon libraries.
 This is mainly used to calculate polygon area and intersections.
 Currently, boost is utilized to perform these calculations.
 
*/

#include "interpolate.h"
#include <cfloat>

/*
  1.1 Constructor, which creates a polygon from the given face and
  transformations.

*/
Poly::Poly(Face& f, double offx, double offy, double scalex, double scaley,
        bool withholes) {
    Pt off(offx, offy);
    Pt scale(scalex, scaley);
    
    for (unsigned int i = 0; i < f.v.size(); i++) {
        Pt p = ((f.v[i].s) - off) * scale / SCALEOUT;
        points.push_back(p);
    }
}

/*
   1.2 ~Area~ returns the area of this Poly object.

*/
double Poly::Area() {
    return inter(points, points);
}

/*
   1.3 ~IntersectionArea~ returns the intersecting area of this Poly object
   with the given Poly object.

*/
double Poly::IntersectionArea(Poly& p) {
    cerr << "Calling inter...\n";
    
    double ret = inter(p.points, points);
    
    cerr << "Finished inter...\n";
    
    return ret;
}

void Poly::range(vector<Pt>& points, int c, Box& bbox) {
    while (c-- > 0) {
        bbox.min.x = min(bbox.min.x, points[c].x);
        bbox.min.y = min(bbox.min.y, points[c].y);
        bbox.max.x = max(bbox.max.x, points[c].x);
        bbox.max.y = max(bbox.max.y, points[c].y);
    }
}

double Poly::area(IPoint a, IPoint p, IPoint q) {
    return (long)p.x * q.y - (long)p.y * q.x +
            (long)a.x * (p.y - q.y) + (long)a.y * (q.x - p.x);
}

bool Poly::ovl(Rng p, Rng q) {
    return (p.mn < q.mx) && (q.mn < p.mx);
}

void Poly::cntrib(int f_x, int f_y, int t_x, int t_y, int w) {
    ssss += (long) w * (t_x - f_x)*(t_y + f_y) / 2;
}

void Poly::fit(vector<Pt>& x, int cx, vector<Vertex>& ix, int fudge, Box& b) {
    int c = cx;
    while (c-- > 0) {
        Vertex v;
        v.ip.x = ((int)((x[c].x - b.min.x) * sclx - mid) & ~7) | fudge|(c & 1);
        v.ip.y = ((int)((x[c].y - b.min.y) * scly - mid) & ~7) | fudge;
        ix[c] = v;
    }

    ix[0].ip.y += (cx&1);
    ix[cx] = ix[0];

    c = cx;
    while (c-- > 0) {
        ix[c].rx = ix[c].ip.x < ix[c + 1].ip.x ?
            Rng(ix[c].ip.x, ix[c + 1].ip.x) : Rng(ix[c + 1].ip.x, ix[c].ip.x);
        ix[c].ry = ix[c].ip.y < ix[c + 1].ip.y ?
            Rng(ix[c].ip.y, ix[c + 1].ip.y) : Rng(ix[c + 1].ip.y, ix[c].ip.y);
        ix[c].in = 0;
    }
}

void Poly::cross(Vertex& a, Vertex& b, Vertex& c, Vertex& d,
        double a1, double a2, double a3, double a4) {
    double r1 = a1 / (a1 + a2);
    double r2 = a3 / (a3 + a4);
    cntrib((int) (a.ip.x + r1 * (b.ip.x - a.ip.x)),
            (int) (a.ip.y + r1 * (b.ip.y - a.ip.y)),
            b.ip.x, b.ip.y, 1);
    cntrib(d.ip.x, d.ip.y,
            (int) (c.ip.x + r2 * (d.ip.x - c.ip.x)),
            (int) (c.ip.y + r2 * (d.ip.y - c.ip.y)),
            1);
    a.in++;
    c.in--;
}

void Poly::inness(vector<Vertex>& P, int cP, vector<Vertex>& Q, int cQ) {
    int s = 0;
    int c = cQ;
    IPoint p = P[0].ip;

    while (c-- > 0) {
        if (Q[c].rx.mn < p.x && p.x < Q[c].rx.mx) {
            bool sgn = 0 < area(p, Q[c].ip, Q[c + 1].ip);
            s += (sgn != (Q[c].ip.x < Q[c + 1].ip.x)) ? 0 : (sgn ? -1 : 1);
        }
    }

    for (int j = 0; j < cP; j++) {
        if (s != 0)
            cntrib(P[j].ip.x, P[j].ip.y, P[j + 1].ip.x, P[j + 1].ip.y, s);
        s += P[j].in;
    }
}

double Poly::inter(vector<Pt> a, vector<Pt> b) {
    sclx = scly = ssss = 0.0;
    int na = a.size();
    int nb = b.size();
    vector<Vertex> ipa(na + 1, Vertex());
    vector<Vertex> ipb(nb + 1, Vertex());
    Box bbox(Pt(DBL_MAX, DBL_MAX), Pt(-DBL_MAX, -DBL_MAX));

    if (na < 3 || nb < 3)
        return 0;
    range(a, na, bbox);
    range(b, nb, bbox);

    double rngx = bbox.max.x - bbox.min.x;
    sclx = gamut / rngx;
    double rngy = bbox.max.y - bbox.min.y;
    scly = gamut / rngy;
    double ascale = sclx * scly;

    fit(a, na, ipa, 0, bbox);
    fit(b, nb, ipb, 2, bbox);

    for (int j = 0; j < na; j++) {
        for (int k = 0; k < nb; k++) {
            if (ovl(ipa[j].rx, ipb[k].rx) && ovl(ipa[j].ry, ipb[k].ry)) {
                long a1 = -area(ipa[j].ip, ipb[k].ip, ipb[k + 1].ip);
                long a2 = area(ipa[j + 1].ip, ipb[k].ip, ipb[k + 1].ip);
                bool o = a1 < 0;
                if (o == (a2 < 0)) {
                    long a3 = area(ipb[k].ip, ipa[j].ip, ipa[j + 1].ip);
                    long a4 = -area(ipb[k + 1].ip, ipa[j].ip,
                            ipa[j + 1].ip);
                    if ((a3 < 0) == (a4 < 0)) {
                        if (o)
                            cross(ipa[j], ipa[j + 1], ipb[k], ipb[k + 1],
                                a1, a2, a3, a4);
                        else
                            cross(ipb[k], ipb[k + 1], ipa[j], ipa[j + 1],
                                a3, a4, a1, a2);
                    }
                }
            }
        }
    }

    inness(ipa, na, ipb, nb);
    inness(ipb, nb, ipa, na);

    return ssss / ascale;
}
