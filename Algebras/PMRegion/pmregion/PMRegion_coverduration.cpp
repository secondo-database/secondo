/* 
 * This file is part of libpmregion
 * 
 * File:   PMRegion\_coverduration.cpp
 * Author: Florian Heinz <fh@sysv.de>

 1 Coverduration
   Calculates the coverduration for all points covered by a PMRegion
   and compiles a new PMRegion from that.
 
*/


#include "PMRegion.h"

#include <CGAL/Surface_sweep_2_algorithms.h>

using namespace pmr;

namespace pmr {

struct seg_compare {
    bool operator() (const Segment2d &l, const Segment2d &r) const {
        Point_2 ls = l.source(), lt = l.target();
        Point_2 rs = r.source(), rt = r.target();
        return ((ls < rs) || ((ls == rs) && (lt < rt)));
    }
};

Kernel::FT PMRegion::thickness (Point_2 p) {
    Point3d p1(p.x(), p.y(), 0);
    Point3d p2(p.x(), p.y(), 1);
    Kernel::Line_3 seg(p1, p2);

    Tree tree(faces(polyhedron).first,faces(polyhedron).second,polyhedron);
    tree.accelerate_distance_queries();
    std::list<Segment_intersection> sis;
    tree.all_intersections(seg, std::back_inserter(sis));

    std::set<Kernel::FT> zs;

    for (std::list<Segment_intersection>::iterator it = sis.begin();
            it != sis.end(); ++it) {
        if (Point3d *p = boost::get<Point3d>(&((*it)->first))) {
            zs.insert(p->z());
        }
    }

    Point_inside inside_tester(tree);

    Kernel::FT thickness = 0;
    Kernel::FT prev;
    bool prevvalid = false;
    for (std::set<Kernel::FT>::iterator it = zs.begin();
            it != zs.end(); ++it) {
        Kernel::FT cur = *it;
        if (prevvalid) {
            Point3d p3(p.x(), p.y(), (prev+cur)/2);
            if (inside_tester(p3) == CGAL::ON_BOUNDED_SIDE) {
                thickness += (cur - prev);
            }
        }
        prev = cur;
        prevvalid = true;
    }


    return thickness;
}


Plane PMRegion::calculate_plane(Polygon p) {
    vector<Point3d> p3d;
    for (unsigned int i = 0; i < 3; i++) {
        Kernel::FT t = thickness(p[i]);
        p3d.push_back(Point3d(p[i].x(), p[i].y(), t));
    }
    Plane pl(p3d[0], p3d[1], p3d[2]);

    return pl;
}

ScalarField PMRegion::coverduration() {
    set<Segment2d, seg_compare> segs;

    for (Facet_iterator f = polyhedron.facets_begin();
            f != polyhedron.facets_end(); f++) {
        Halfedge_facet_circulator h = f->facet_begin(), he(h);

        Point_2 prev;
        bool prevvalid = false;
        do {
            Point3d p3 = h->vertex()->point();
            Point_2 p(p3.x(), p3.y());
            if (prevvalid) {
                Segment2d seg(prev, p);
                if (prev > p)
                    seg = seg.opposite();
                segs.insert(seg);
            }
            prev = p;
            prevvalid = true;
        } while (++h != he);
        Point3d p3 = he->vertex()->point();
        Segment2d seg(Point_2(p3.x(), p3.y()), prev);
        if (seg.source() > seg.target())
            seg = seg.opposite();
        segs.insert(seg);
    }

    std::list<Segment2d> subsegs;
    CGAL::compute_subcurves(segs.begin(), segs.end(),
            std::back_inserter(subsegs));

    Arrangement arr;

    for (std::list<Segment2d>::iterator it = subsegs.begin();
            it != subsegs.end(); it++) {
        CGAL::insert(arr, *it);
    }

    Arrangement::Face_const_iterator            fit;
    Arrangement::Ccb_halfedge_const_circulator  curr;
    ScalarField scalarfield;
    for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
        if (!fit->is_unbounded()) {
            curr = fit->outer_ccb();
            Polygon poly;
            do {
                poly.push_back(curr->target()->point());
                ++curr;
            } while (curr != fit->outer_ccb());
            Plane plane = calculate_plane(poly);
            scalarfield.add(poly, plane);
        }
    }

    return scalarfield;
}


PMRegion PMRegion::coverduration2() {
    set<Segment2d, seg_compare> segs;

    for (Facet_iterator f = polyhedron.facets_begin();
            f != polyhedron.facets_end(); f++) {
        Halfedge_facet_circulator h = f->facet_begin(), he(h);

        Point_2 prev;
        bool prevvalid = false;
        do {
            Point3d p3 = h->vertex()->point();
            Point_2 p(p3.x(), p3.y());
            if (prevvalid) {
                Segment2d seg(prev, p);
                if (prev > p)
                    seg = seg.opposite();
                segs.insert(seg);
            }
            prev = p;
            prevvalid = true;
        } while (++h != he);
        Point3d p3 = he->vertex()->point();
        Segment2d seg(Point_2(p3.x(), p3.y()), prev);
        if (seg.source() > seg.target())
            seg = seg.opposite();
        segs.insert(seg);
    }

    std::list<Segment2d> subsegs;
    CGAL::compute_subcurves(segs.begin(), segs.end(),
                            std::back_inserter(subsegs));

    Arrangement arr;
    for (std::list<Segment2d>::iterator it = subsegs.begin();
                                      it != subsegs.end(); it++) {
        CGAL::insert(arr, *it);
    }

    PMRegion ret;
    int _idx = 0;
    Arrangement::Face_const_iterator            fit;
    Arrangement::Ccb_halfedge_const_circulator  curr;
    map<Point2d, Point3d> points;
    map<Point3d, int> indices;
    std::vector<Point3d> xpoints;
    std::vector<std::vector<std::size_t> > xpolygons;
    for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
        if (!fit->is_unbounded()) {
            curr = fit->outer_ccb();
            Polygon poly;
            vector<size_t> facet;
            do {
                Point2d p2d = curr->target()->point();
                poly.push_back(p2d);
                Point3d p3d;
                int idx;
                if (points.count(p2d) == 0) {
                    Kernel::FT z = thickness(p2d);
                    p3d = Point3d(p2d.x(), p2d.y(), z);
                    points[p2d] = p3d;
                    indices[p3d] = idx = _idx++;
                    xpoints.push_back(p3d);
                } else {
                    p3d = points[p2d];
                    idx = indices[p3d];
                }
                facet.push_back(idx);
                ++curr;
            } while (curr != fit->outer_ccb());
            xpolygons.push_back(facet);
        }
    }

    Arrangement::Face_iterator fi = arr.unbounded_face();
    for (Arrangement::Hole_iterator fs = fi->holes_begin();
            fs != fi->holes_end(); fs++) {
        Arrangement::Ccb_halfedge_circulator c = *fs, ec(c);
        vector<size_t> facet;
        do {
            Point2d p2d = c->source()->point();
            Point3d p3d(p2d.x(), p2d.y(), 0);
            facet.push_back(indices[p3d]);
        } while (++c != ec);
        xpolygons.push_back(facet);
    }


    CGAL::Polygon_mesh_processing::orient_polygon_soup(xpoints, xpolygons);
    CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(xpoints,
            xpolygons, ret.polyhedron);
    CGAL::Polygon_mesh_processing::triangulate_faces(ret.polyhedron);

    return ret;
}

void ScalarField::add(Polygon polygon, Plane plane) {
    polygons.push_back(polygon);
    vector<Kernel::FT> co;
    co.push_back(plane.a());
    co.push_back(plane.b());
    co.push_back(plane.c());
    co.push_back(plane.d());
    coeffs.push_back(co);
}

ScalarField ScalarField::fromRList(RList rl) {
    ScalarField ret;

    RList& obj = rl.items[4];

    for (unsigned int i = 0; i < obj.items.size(); i++) {
        RList& part = obj.items[i];
        RList& face = part.items[0];
        RList& coeffs = part.items[1];
        Polygon poly;
        for (unsigned int j = 0; j < face.items.size(); j++) {
            Point2d p(face.items[j].items[0].getNr(),
                      face.items[j].items[1].getNr());
            poly.push_back(p);
        }
        vector<Kernel::FT> co;
        co.push_back(coeffs.items[0].getNr());
        co.push_back(coeffs.items[1].getNr());
        co.push_back(coeffs.items[2].getNr());
        co.push_back(coeffs.items[3].getNr());
        ret.polygons.push_back(poly);
        ret.coeffs.push_back(co);
    }

    return ret;

}

RList ScalarField::toRList() {
    RList scalarfield;

    for (unsigned int i = 0; i < polygons.size(); i++) {
        RList plane, face, coeff;

        Polygon p = polygons[i];
        vector<Kernel::FT> co = coeffs[i];
        for (unsigned int j = 0; j < p.size(); j++) {
            Point2d p2d = p[j];
            RList point;
            point.append(::CGAL::to_double(p2d.x()));
            point.append(::CGAL::to_double(p2d.y()));
            face.append(point);
        }
        coeff.append(::CGAL::to_double(co[0]));
        coeff.append(::CGAL::to_double(co[1]));
        coeff.append(::CGAL::to_double(co[2]));
        coeff.append(::CGAL::to_double(co[3]));
        plane.append(face);
        plane.append(coeff);
        scalarfield.append(plane);
    }

    return scalarfield.obj("scalarfield", "scalarfield");
}

Kernel::FT ScalarField::value(Point2d point) {
    for (unsigned int i = 0; i < polygons.size(); i++) {
        Polygon& polygon = polygons[i];
        if (polygon.bounded_side(point) != CGAL::ON_UNBOUNDED_SIDE) {
            vector<Kernel::FT> coeff = coeffs[i];
            return -(coeff[0]*point.x()+coeff[1]*point.y()+coeff[3])/coeff[2];
        }
    }

    return 0;
}


}
