/*
   poly.h defines an external polygon representation used for
   calculating area and intersection area of polygons.
   Currently utilizes the boost-library.
 
*/

#ifndef POLY2_H
#define POLY2_H

#include "interpolate.h"

class Box {
 public:
   Pt min, max;
   Box(Pt min, Pt max) : min(min), max(max) {};
};

class Rng {
 public:
   int mn, mx;
   Rng(int mn, int mx) : mn(mn), mx(mx) {};
   Rng() {};
};

class IPoint {
 public:
   long x, y;
};

class Vertex {
public:
   IPoint ip;
   Rng rx, ry;
   int in;
};

const double gamut = 500000000.;
const double mid = gamut / 2.;

class Poly { // external polygon
 public:
   vector<Pt> points;
   long ssss;
   double sclx, scly;
   
   Poly(Face& f, double offx, double offy,
           double scalex, double scaley, bool withholes);
   double Area(); // area of the polygon
   double IntersectionArea(Poly& p); // intersection area of polygon with p
   
   static void range(vector<Pt>& points, int c, Box& bbox);
   static double area(IPoint a, IPoint p, IPoint q);
   static bool ovl(Rng p, Rng q);
   void cntrib(int f_x, int f_y, int t_x, int t_y, int w);
   void fit(vector<Pt>& x, int cx, vector<Vertex>& ix, int fudge, Box& b);
   void cross(Vertex& a, Vertex& b, Vertex& c, Vertex& d,
        double a1, double a2, double a3, double a4);
   void inness(vector<Vertex>& P, int cP, vector<Vertex>& Q, int cQ);
   double inter(vector<Pt> a, vector<Pt> b);
};

#endif  /* POLY2_H */

