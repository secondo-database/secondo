/*
   poly.h defines an external polygon representation used for
   calculating area and intersection area of polygons.
   Currently utilizes the boost-library.
 
*/

#ifndef POLY_H
#define POLY_H

#include "interpolate.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>

#include <boost/foreach.hpp>

class Face;

typedef boost::geometry::model::
        polygon<boost::geometry::model::d2::point_xy<double> > Polygon;

class Poly { // external polygon
protected:
    Polygon polygon; // a boost polygon
    
public:
    Poly(Face& f, double offx, double offy,
         double scalex, double scaley, bool withholes);
    double Area(); // area of the polygon
    double IntersectionArea(Poly p); // intersection area of polygin with p
};

#endif  /* POLY_H */

