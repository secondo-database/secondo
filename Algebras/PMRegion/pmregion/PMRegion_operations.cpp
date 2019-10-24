/* 
 * This file is part of libpmregion
 * 
 * File:   PMRegion\_operations.cpp
 * Author: Florian Heinz <fh@sysv.de>

 1 Operations
   Implementation of the operations on PMRegions

*/

#include "PMRegion_internal.h"
#include <sstream>
#include <iomanip>
#include <map>
#include <ctime>

namespace pmr {

/*
 *                   O P E R A T I O N S

*/

/* Operation "union" of two pmregions */
PMRegion PMRegion::operator+ (PMRegion &reg) {
    // Convert polyhedra to Nef polyhedra
    Nef_polyhedron p1(polyhedron), p2(reg.polyhedron);
    // Compute the union
    Nef_polyhedron p12 = p1 + p2;

    PMRegion ret; // Convert it back to normal polyhedra
    ret.polyhedron = nef2polyhedron(p12);

    return ret;
}

/* Operation "intersection" of two pmregions */
PMRegion PMRegion::operator* (PMRegion &reg) {
    // Convert polyhedra to Nef polyhedra
    Nef_polyhedron p1(polyhedron), p2(reg.polyhedron);
    // Compute the intersection
    Nef_polyhedron p12 = p1 * p2;

    PMRegion ret; // Convert it back to normal polyhedra
    ret.polyhedron = nef2polyhedron(p12);

    return ret;
}


/* Operation "difference" of two pmregions */
PMRegion PMRegion::operator- (PMRegion &reg) {
    // Convert polyhedra to Nef polyhedra
    Nef_polyhedron p1(polyhedron), p2(reg.polyhedron);
    // Compute the difference
    Nef_polyhedron p12 = p1 - p2;

    PMRegion ret; // Convert it back to normal polyhedra
    ret.polyhedron = nef2polyhedron(p12);

    return ret;
}

/* Helper function, which calculates the intersection of
 * a polyhedron with a cutting plane at a given z coordinate */
static Arrangement zplanecut (Polyhedron ph, Kernel::FT z) {
    // Build an AABB tree from the polyhedron
    Tree tree(faces(ph).first, faces(ph).second, ph);
    // Create the cutting plane
    Plane pl(Point3d(0, 0, z), Vector(0, 0, 1));
    std::list<Plane_intersection> pis;
    // Calculate all intersections between plane and polyhedron
    tree.all_intersections(pl, std::back_inserter(pis));

    // Insert all intersections (segments) into a 2D-Arrangement
    Arrangement arr;
    for (std::list<Plane_intersection>::iterator it = pis.begin();
            it != pis.end(); ++it) {
        Segment *s = boost::get<Segment>(&((*it)->first));
        if (s != NULL && !s->is_degenerate())
            CGAL::insert(arr, Segment_2(Point_2(s->source().x(),
                s->source().y()), Point_2(s->target().x(), s->target().y())));
    }

    return arr;
}

/* Operation "atinstant" for a pmregion */
RList PMRegion::atinstant (double instant) {
    // Calculate the intersections with a cutting plane
    Arrangement cut = zplanecut(polyhedron, instant);

    RList region;
    // Convert the faces from the arrangement to a region
    Arrangement2Region(cut.unbounded_face(), region);

    return region.obj("region", "region");
}

/* Operation "mpointinside" for a pmregion */
MBool PMRegion::mpointinside(RList& mpoint) {
    // Convert the mpoint units to 3D segments
    vector<Segment> segs = mpoint2segments(mpoint);

    // Build an AABB-Tree
    Tree tree(faces(polyhedron).first, faces(polyhedron).second, polyhedron);
    // Create a "point inside polyhedron" tester
    Point_inside inside_tester(tree);

    MBool mbool;
    for (unsigned int i = 0; i < segs.size(); i++) {
        std::list<Segment_intersection> sis;
        // Calculate all intersections (points) between segments and polyhedra
        tree.all_intersections(segs[i], std::back_inserter(sis));
        std::set<Kernel::FT> events;
        // Insert the z coordinates of these points into an ordered set
        for (std::list<Segment_intersection>::iterator it = sis.begin();
                it != sis.end(); ++it) {
            Segment_intersection si = *it;
            if (Point3d *p = boost::get<Point3d>(&(si->first))) 
                events.insert(p->z());
        }
        // Also add the final z coordinate of the segment
        events.insert(segs[i].target().z());

        // Begin with the start z coordinate of the segment
        Kernel::FT prev = segs[i].source().z();
        // Check, if we start inside or outside the polyhedron
        bool inside = inside_tester(segs[i].source()) == CGAL::ON_BOUNDED_SIDE;
        for (set<Kernel::FT>::iterator it = events.begin();
                it != events.end(); it++) {
            // Append a moving boolean unit with the current "inside" value
            mbool.append(prev, *it, inside);
            // Next segment is outside, if current is inside and vice-versa
            inside = !inside;
            prev = *it;
        }
    }

    return mbool;
}

/* Operation "perimeter" for a pmregion */
Kernel::FT getPerimeterFromArrangement (Arrangement& arr);
MReal PMRegion::perimeter () {
    MReal mreal;
    // Get all z coordinates of the polyhedron in an ordered set
    std::set<Kernel::FT> zevents = getZEvents(polyhedron);
    Kernel::FT prev;
    bool first = true;
    for (std::set<Kernel::FT>::iterator it = zevents.begin();
            it != zevents.end(); it++) {
        // For each consecutive pair of z coordinates ...
        if (!first) {
            Kernel::FT cur = *it;

            // ... calculate the intersections with a cutting plane
            Arrangement a1 = zplanecut(polyhedron, prev+EPSILON);
            Arrangement a2 = zplanecut(polyhedron, cur-EPSILON);
	    Kernel::FT deltat = (cur - prev)/86400000;

            // ... calculate the perimeter lengths
            Kernel::FT a1perimeter = getPerimeterFromArrangement(a1);
            Kernel::FT a2perimeter = getPerimeterFromArrangement(a2);
            Kernel::FT diff = a2perimeter - a1perimeter;

            // create a mreal representing the perimeter development
            mreal.append(prev, cur, 0, diff/deltat, a1perimeter);
        } else
            first = false;
        prev = *it;
    }

    return mreal;
}
// Helper function to calculate the perimeter length of an arrangement
Kernel::FT getPerimeterFromArrangement (Arrangement& arr) {
    Kernel::FT ret = 0;
    // Iterate over all segments in this arrangement
    for (Arrangement::Edge_iterator ei = arr.edges_begin();
            ei != arr.edges_end(); ei++) {
        Arrangement::Halfedge he = *ei;
        // Add up the lengths of the segments
        ret = ret + sqrt(to_double(Kernel::Segment_2(he.source()->point(),
                        he.target()->point()).squared_length()));
    }

    return ret;
}

/* Operation "area" for a pmregion */
static Kernel::FT getAreaFromArrangement (Arrangement& arr);
MReal PMRegion::area () {
    MReal mreal;
    // Get all z coordinates of the polyhedron in an ordered set
    std::set<Kernel::FT> zevents = getZEvents(polyhedron);
    Kernel::FT prev;
    bool first = true;
    for (std::set<Kernel::FT>::iterator it = zevents.begin();
            it != zevents.end(); it++) {
        // For each consecutive pair of z coordinates ...
        if (!first) {
            Kernel::FT cur = *it;

            // ... calculate the intersections with a cutting plane
            Arrangement a1 = zplanecut(polyhedron, prev+EPSILON);
            Arrangement a2 = zplanecut(polyhedron, cur-EPSILON);

	    Kernel::FT deltat = (cur - prev)/86400000;
            // ... calculate the areas
            Kernel::FT a1area = getAreaFromArrangement(a1);
            Kernel::FT a2area = getAreaFromArrangement(a2);
            Kernel::FT diff = a2area - a1area;

            // create a mreal representing the area development
            mreal.append(prev, cur, diff/deltat/deltat, 0, a1area);
        } else
            first = false;
        prev = *it;
    }

    return mreal;
}
// Helper functions to calculate the area from an arrangement
static Kernel::FT getAreaFromFace (Arrangement& arr,
        Arrangement::Face_handle f) {
    Kernel::FT area = 0;
    // Calculate the area of this face ...
    if (f->has_outer_ccb()) {
        Polygon p;
        Arrangement::Ccb_halfedge_circulator c = f->outer_ccb(), ec(c);
        do {
            p.push_back(c->source()->point());
        } while (++c != ec);
        area = p.area();
    }

    // ... but subtract the area of the holes
    for (Arrangement::Hole_iterator hi = f->holes_begin();
            hi != f->holes_end(); hi++) {
        area -= getAreaFromFace(arr, (*hi)->twin()->face());
    }

    return area;
}
static Kernel::FT getAreaFromArrangement (Arrangement& arr) {
    return -getAreaFromFace(arr, arr.unbounded_face());
}

/* Operation "intersects" for a pmregion */
MBool PMRegion::intersects (PMRegion& pmr) {
    MBool mbool;

    // Convert the polyhedra to Nef polyhedra
    Nef_polyhedron np1(polyhedron), np2(pmr.polyhedron);
    // Calculate the intersection of the polyhedra
    Nef_polyhedron np3 = np1 * np2;

    // Iterate over all segments and calculate the unified ranges of
    // the z coordinates spanned by the segments
    Range<Kernel::FT> is;
    for (Halfedge_const_iterator e = np3.halfedges_begin();
            e != np3.halfedges_end(); ++e) {
        Kernel::FT z1 = e->source()->point().z();
        Kernel::FT z2 = e->target()->point().z();
        is.addrange(z1, z2);
    }

    // Append an "mbool" unit for each subrange
    std::map<Kernel::FT, Kernel::FT>::iterator it = is.range.begin();
    while (it != is.range.end()) {
        mbool.append(it->first, it->second, true);
        it++;
    }

    return mbool;
}

/* Operation "traversedarea" for a pmregion */
RList PMRegion::traversedarea () {
    RList ret;

    vector<Polygon_2> ii;
    vector<Polygon_with_holes_2> oi;
    for (Facet_iterator s = polyhedron.facets_begin();
            s != polyhedron.facets_end(); ++s) {
        Halfedge_facet_circulator h = s->facet_begin(), he(h);

        Polygon_2 polygon;
        do {
            Point3d p1 = h->vertex()->point();
            polygon.insert(polygon.vertices_end(), Point_2(p1.x(), p1.y()));
        } while (++h != he);
        if (!polygon.is_simple()) {
            // Empty polygon after projection
            continue;
        }
        if (polygon.orientation() == CGAL::NEGATIVE)
            polygon.reverse_orientation();
        ii.push_back(polygon);
    }
    CGAL::join(ii.begin(), ii.end(), std::back_inserter(oi));

    return Polygons2Region(oi);
}

void PMRegion::translate(Kernel::FT x, Kernel::FT y, Kernel::FT z) {
    Kernel::Vector_3 translate(x, y, z);
    for (Polyhedron::Vertex_iterator v = polyhedron.vertices_begin();
            v != polyhedron.vertices_end(); v++) {
        v->point() = v->point() + translate;
    }
}

}
