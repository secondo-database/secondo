/*
 1 Poly.cpp is an interface for external polygon libraries.
 This is mainly used to calculate polygon area and intersections.
 Currently, boost is utilized to perform these calculations.
 
*/

#include "interpolate.h"
#include "poly_boost.h"
#include <boost/geometry/geometry.hpp> 
#include <boost/geometry/geometries/register/point.hpp> 
#include <boost/geometry/geometries/register/ring.hpp> 
#include <boost/geometry/geometry.hpp> 
#include <boost/foreach.hpp>

/*
  1.1 Constructor, which creates a polygon from the given face and
  transformations.

*/
Poly::Poly(Face& f, double offx, double offy, double scalex, double scaley,
           bool withholes) {
    using boost::geometry::append;
    using boost::geometry::correct;
    
  // Boost wants the polygon to be clockwise oriented, so push in reverse order
    for (unsigned int i = f.v.size(); i > 0; i--) {
        double x = (f.v[i-1].s.x - offx) * scalex / SCALEOUT;
        double y = (f.v[i-1].s.y - offy) * scaley / SCALEOUT;
        boost::geometry::model::d2::point_xy<double> p(x, y);
        boost::geometry::append(polygon, p);
    }
   
   boost::geometry::correct(polygon);
}

/*
   1.2 ~Area~ returns the area of this Poly object.

*/
double Poly::Area() {
    return boost::geometry::area(polygon);
}

/*
   1.3 ~IntersectionArea~ returns the intersecting area of this Poly object
   with the given Poly object.

*/
double Poly::IntersectionArea (Poly p) {
    double ret = 0;
    std::deque<Polygon> out;
   
    boost::geometry::intersection(polygon, p.polygon, out);
    
    BOOST_FOREACH(Polygon const& p2, out) {
        ret += boost::geometry::area(p2);
    }
    
    return ret;
}
