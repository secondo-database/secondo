/*
 * This file is part of libpmregion
 * 
 * File:   PMRegion\_internal.h
 * Author: Florian Heinz <fh@sysv.de>

 1 Internal Header
   Internal Headerfile used by the library sources

*/

#ifndef PMREGION_INTERNAL_HXX
#define PMREGION_INTERNAL_HXX

#define VERSION "1.3"

#include <istream>
#include <string>
#include <vector>
#include <cassert>

#include <CGAL/version.h>

#if CGAL_VERSION_NR < 1041200000
#  warning "CGAL library too old, disabling coverduration operations"
#  define PMREGION_DISABLE_COVERDURATION 1
#endif

#include <CGAL/Polyhedron_3.h>
#include <CGAL/Side_of_triangle_mesh.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
// #include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Modifier_base.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h>
#include <CGAL/Extended_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/refine.h>
#include <CGAL/Polygon_mesh_processing/fair.h>
// #include <CGAL/Polygon_mesh_processing/measure.h>

//typedef CGAL::Extended_cartesian< CGAL::Lazy_exact_nt<CGAL::Gmpq> > Kernel;
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
//typedef CGAL::Exact_predicates_exact_constructions_kernel_with_sqrt Kernel;
//typedef CGAL::Extended_cartesian< CGAL::Lazy_exact_nt<CGAL::Gmpq> > Kernel;
//typedef CGAL::Simple_cartesian<double> Kernel;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;

typedef Kernel::Vector_3 Vector;
typedef Kernel::Point_3 Point3d;
typedef Kernel::Point_2 Point2d;
typedef Kernel::Plane_3 Plane;
typedef Kernel::Segment_3 Segment;
typedef Kernel::Segment_2 Segment2d;
typedef CGAL::Polygon_2<Kernel> Polygon;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits_2;
// typedef CGAL::Arr_non_caching_segment_traits_2<Kernel> Traitsnc_2;
typedef Traits_2::Point_2 Point_2;
typedef Traits_2::X_monotone_curve_2 Segment_2;
typedef CGAL::Arrangement_2<Traits_2> Arrangement;
// typedef CGAL::Arrangement_2<Traitsnc_2> Arrangementnc;

typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron> Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;
typedef boost::optional< Tree::Intersection_and_primitive_id<Plane>::Type >
                                                           Plane_intersection;
typedef boost::optional< Tree::Intersection_and_primitive_id<Segment>::Type >
                                                         Segment_intersection;
typedef CGAL::Polyhedron_incremental_builder_3<Polyhedron::HalfedgeDS>
                                                           Polyhedron_builder;
typedef Polyhedron::Vertex_iterator Vertex_iterator;
typedef Polyhedron::Point_iterator Point_iterator;
typedef Polyhedron::Facet_iterator Facet_iterator;
typedef Polyhedron::Halfedge_around_facet_circulator Halfedge_facet_circulator;
typedef Polyhedron::Halfedge_iterator Halfedge_iterator;
typedef Nef_polyhedron::Volume_const_iterator Volume_const_iterator;
typedef Nef_polyhedron::Vertex_const_iterator Vertex_const_iterator;
typedef Nef_polyhedron::Halfedge_const_iterator Halfedge_const_iterator;
typedef Nef_polyhedron::Halffacet_const_iterator Halffacet_const_iterator;
typedef Nef_polyhedron::Halffacet_cycle_const_iterator
                                                Halffacet_cycle_const_iterator;
typedef Nef_polyhedron::SHalfedge_const_handle SHalfedge_const_handle;
typedef Nef_polyhedron::SHalfedge_around_facet_const_circulator
                                       SHalfedge_around_facet_const_circulator;
typedef CGAL::Side_of_triangle_mesh<Polyhedron, Kernel> Point_inside;
typedef CGAL::Projection_traits_xy_3<Kernel> Projection;
typedef CGAL::Polygon_2<Kernel> Polygon_2;
typedef CGAL::Polygon_with_holes_2<Kernel> Polygon_with_holes_2;
typedef Polygon_with_holes_2::Hole_const_iterator Hole_const_iterator;
typedef Polygon_2::Vertex_iterator PG2VI;
typedef Polygon_with_holes_2::Hole_const_iterator PG2H;

#define PRECISION 15
#define EPSILON Kernel::FT(0.0000001)
//#define EPSILON Kernel::FT(0)

namespace pmr {
std::string timestr (Kernel::FT t);
double parsetime (std::string str);

#ifndef RLIST_TYPES
#define RLIST_TYPES
enum {
    NL_LIST = 1,
    NL_STRING,
    NL_SYM,
    NL_DOUBLE,
    NL_FT,
    NL_BOOL
};
#endif

template <typename T> class Range {
        public:
                std::map<T, T> range;
                void addrange(T a, T b);
                void print();
};

struct seg_compare {
    bool operator() (const Segment2d &l, const Segment2d &r) const {
        Point_2 ls = l.source(), lt = l.target();
        Point_2 rs = r.source(), rt = r.target();
        return ((ls < rs) || ((ls == rs) && (lt < rt)));
    }
};
}

#include "PMRegion.h"

namespace pmr {
void Arrangement2Region(Arrangement::Face_iterator fi, pmr::RList& region);
void Arrangement2Region2(Arrangement& arr, pmr::RList& region);
std::vector<Segment> mpoint2segments (pmr::RList& obj);
std::set<Kernel::FT> getZEvents (Polyhedron p, Kernel::FT mindiff);
std::set<Kernel::FT> getZEvents (Polyhedron p);
pmr::RList Polygons2Region (std::vector<Polygon_with_holes_2> polygons);
Polyhedron nef2polyhedron (Nef_polyhedron np);
std::vector<Polygon_with_holes_2> projectxy (PMRegion pm);
Polyhedron extrude (std::vector<Polygon_with_holes_2> ta,
        Kernel::FT start, Kernel::FT end);
Kernel::FT area (Polygon_with_holes_2 p);
Kernel::FT area (std::vector<Polygon_with_holes_2> ps);
Kernel::FT area (RList region);
Nef_polyhedron region2nefpolyhedron(RList reg, Kernel::FT instant1,
        Kernel::FT instant2, Kernel::FT xoff);

std::vector<Polygon_with_holes_2> Region2Polygons (RList region);
void do_deter1(std::string fname);
void translatedelta(Kernel::FT limit, bool x, bool y, bool z, Polyhedron& p);

template <class T>
class MyPoint {
    public:
    T _x, _y, _z;

    MyPoint () : _x(0), _y(0), _z(0) {}

    MyPoint (T x, T y, T z) : _x(x), _y(y), _z(z) {
    }

    MyPoint (Point3d& p) {
        _x = p.x();
        _y = p.y();
        _z = p.z();
    }

    T x() { return _x; }
    T y() { return _y; }
    T z() { return _z; }

    bool operator< (const MyPoint<T>& p) const {
        if (_x < p._x)
            return true;
        else if (_x == p._x && _y < p._y)
            return true;
        else if (_x == p._x && _y == p._y && _z < p._z)
            return true;

        return false;
    }

    bool operator== (const MyPoint<T>& p) const {
        return _x == p._x && _y == p._y && _z == p._z;
    }

};

typedef MyPoint<Kernel::FT> Point_3;
typedef Kernel::FT Num;

class Seg {
    public:
        // start and end point
        Point_2 s, e;
        bool valid; // valid flag

        Seg(Point_2 s, Point_2 e) : s(s), e(e), valid(true) {};
        Seg(Point_3 _s, Point_3 _e) : valid(true) {
            s = Point_2(_s.x(), _s.y());
            e = Point_2(_e.x(), _e.y());
        };

        Seg() : valid(false) {};

        // 1, 0, -1 : p is leftOf, on, rightOf segment
        Kernel::FT sign (Point_2 p) {
            return p.x()*s.y() - p.x()*e.y() - e.x()*s.y() -
                p.y()*s.x() + p.y()*e.x() + e.y()*s.x();
        }

        std::string ToString () {
            std::stringstream ss;
            ss << s << " - " << e;
            return ss.str();
        }

        // reverse this segment
        void reverse() {
            Point_2 tmp = s;
            s = e;
            e = tmp;
        }
};

class Seg3d {
    public:
        Point_3 s;
        Point_3 e;

        Seg3d() { s = Point_3(0,0,0); e = Point_3(0,0,0); }
        Seg3d(Point_3 s, Point_3 e) : s(s), e(e) {}

        void reverse () {
            Point_3 tmp = s;
            s = e;
            e = tmp;
        }

        bool operator< (const Seg3d& m) const {
            return ((s < m.s) || ((s == m.s) && (e < m.e)));
        }

        bool operator== (const Seg3d& m) const {
            return ((s == m.s) && (e == m.e));
        }
};

class MovSeg {
    public:
        Seg3d i, f;
        bool valid;

        MovSeg(Point_3 is, Point_3 ie, Point_3 fs, Point_3 fe) : valid(true) {
            i = Seg3d(is, ie);
            f = Seg3d(fs, fe);
        }

        MovSeg(Seg3d i, Seg3d f) : i(i), f(f), valid(true) {
        }

        MovSeg() : valid(false) {}

        void reverse ();
        bool isnext(MovSeg* ms) { return ms->i.s == i.e && ms->f.s == f.e; }
        bool isnext(MovSeg ms) { return ms.i.s == i.e && ms.f.s == f.e; }
        bool isreversenext (MovSeg* ms) { return ms->i.e==i.e&&ms->f.e==f.e; }
        bool isreversenext (MovSeg ms) { return ms.i.e==i.e&&ms.f.e==f.e; }
        static bool isnear (Point_3 a, Point_3 b, Num delta);
        bool isdegenerated () { return i.s == i.e && f.s == f.e; }
        bool isdegenerated(double d){return isnear(i.s,i.e,d)&&
            isnear(f.s,f.e,d);}
        bool operator<(const MovSeg m)const{return(i<m.i)||((i==m.i)&&f<m.f);}
        bool operator== (const MovSeg ms) const {return i==ms.i && f==ms.f;}

        std::vector<MovSeg *>::iterator findNext (std::vector<MovSeg *>& vec);
        std::vector<MovSeg>::iterator findNext (std::vector<MovSeg>& vec);
        std::string ToString() const;
};

static inline bool isnear (Point_2 p1, Point_2 p2, Kernel::FT d) {
    return abs(p1.x()-p2.x()) < d && abs(p1.y()-p2.y()) < d;
}


class Triangle {
    public:
        Point_3 a, b, c;

        Triangle (Point_3 p1, Point_3 p2, Point_3 p3);
        int between (Num z);
        int sign (Num n); 
        Seg3d project (Num z);
        MovSeg *project (Num z1, Num z2);
};


}



#endif /* PMREGION_INTERNAL_HXX */
