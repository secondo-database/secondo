/* 
 * This file is part of libpmregion
 * 
 * File:   PMRegion\_coverduration.cpp
 * Author: Florian Heinz <fh@sysv.de>

 1 Coverduration
   Calculates the coverduration for all points covered by a PMRegion
   and compiles a new PMRegion from that.
 
*/

#include "PMRegion_internal.h"

#ifndef PMREGION_DISABLE_COVERDURATION

#include <CGAL/Surface_sweep_2_algorithms.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <fstream>

using namespace pmr;
using namespace std;

namespace pmr {

void PMRegion::zthicknessprepare () {
    // Create a copy of the polyhedron and triangulate the faces
    zthicknesstmp = polyhedron;
    CGAL::Polygon_mesh_processing::triangulate_faces(zthicknesstmp);

    // Build an AABB tree for fast intersection tests
    zthicknesstree = new Tree(faces(zthicknesstmp).first,
            faces(zthicknesstmp).second, zthicknesstmp);
    zthicknesstree->accelerate_distance_queries();

    // Create a tester to check, if a point is within the polyhedron boundary
    inside_tester = new Point_inside(*zthicknesstree);
}

/* zthickness calculates the thickness of the polyhedron wrt the
 * z axis at * the given xy coordinates.                         */
Kernel::FT PMRegion::zthickness (Point2d p2d) {
    if (ztcache.count(p2d) > 0) {
        return ztcache[p2d];
    }
    Kernel::FT x = p2d.x();
    Kernel::FT y = p2d.y();

    // Create a line orthogonal to the z plane through point x/y
    Point3d p1(x, y, 0), p2(x, y, 1);
    Kernel::Line_3 line(p1, p2);

    // Calculate intersections between the line and the polyhedron boundary
    list<Segment_intersection> sis;
    zthicknesstree->all_intersections(line, back_inserter(sis));

    // Add all intersection points to an ordered set
    set<Kernel::FT> zs;
    for (list<Segment_intersection>::iterator it = sis.begin();
            it != sis.end(); ++it) {
        if (Point3d *p = boost::get<Point3d>(&((*it)->first))) {
            zs.insert(p->z());
        }
    }

    Kernel::FT zthickness = 0;
    Kernel::FT prev;
    bool prevvalid = false;
    // Iterate over all intersection points in z order
    for (set<Kernel::FT>::iterator it = zs.begin();
            it != zs.end(); ++it) {
        Kernel::FT cur = *it;
        if (prevvalid) {
        // For two consecutive points, calculate the middle point and test,
        // if it is inside the polyhedron.
            Point3d p3(x, y, (prev+cur)/2);
            if ((*inside_tester)(p3) == CGAL::ON_BOUNDED_SIDE) {
            // It is inside, so the length of the segment contributes
        // to the zthickness
                zthickness += (cur - prev);
            }
        }
        prev = cur;
        prevvalid = true;
    }
    ztcache[p2d] = zthickness;

    return zthickness;
}

/* The operation coverduration returns, for which time a given point is
   covered by a moving region. This function operates on a pre-calculated
   cdpolyhedron */
Kernel::FT PMRegion::coverduration (Kernel::FT x, Kernel::FT y) {
    // Create a line orthogonal to the z plane through point x/y
    Point3d p1(x, y, 0), p2(x, y, 1);
    Kernel::Line_3 line(p1, p2);

    // Create a copy of the polyhedron and triangulate the faces
    Polyhedron poly = polyhedron;
    Tree tree(faces(poly).first,faces(poly).second,poly);
    
    // Calculate intersections between the line and the polyhedron boundary
    list<Segment_intersection> sis;
    tree.all_intersections(line, back_inserter(sis));

    // Due to the specific characteristics of a cdpolyhedron, there can be
    // at most two intersections. One at (x,y,0) and the other at (x,y,z),
    // where z is the result value.
    for (list<Segment_intersection>::iterator it = sis.begin();
            it != sis.end(); ++it) {
        if (Point3d *p = boost::get<Point3d>(&((*it)->first))) {
            if (p->z() != 0)
                return p->z();
        }
    }

    // The line was outside the polyhedron or intersected the polyhedron in
    // a single point, which has to be z=0 then.
    return 0;
}

/* projects the 3d facets of the polyhedron to 2d segments in the xy plane */
set<Segment_2, seg_compare> getprojectedsegments(Polyhedron polyhedron) {
    set<Segment_2, seg_compare> segs;

    // Iterate over all polyhedron facets
    for (Facet_iterator f = polyhedron.facets_begin();
            f != polyhedron.facets_end(); f++) {
        Halfedge_facet_circulator h = f->facet_begin(), he(h);

        Point_2 prev;
        bool prevvalid = false;
        do {
            Point3d p3 = h->vertex()->point();
            Point_2 p(p3.x(), p3.y());
            if (prevvalid && prev != p) {
        // Create a 2d line segment for each polygon segment 
        // Define an unambiguous order to prevent duplicates
            if (prev > p) {
            Segment_2 seg(p, prev);
            segs.insert(seg);
            } else {
            Segment_2 seg(prev, p);
            segs.insert(seg);
            }
            }
            prev = p;
            prevvalid = true;
        } while (++h != he);

    // Add last segment which closes the polygon
        Point3d p3 = he->vertex()->point();
        Point_2 p(p3.x(), p3.y());
        if (p != prev) {
        if (prev > p) {
            Segment_2 seg(p, prev);
            segs.insert(seg);
        } else {
            Segment_2 seg(prev, p);
            segs.insert(seg);
        }
        }
    }

    return segs;
}

/* Create non-intersecting subsegments from a set of intersecting 2d segments.
 * This creates for example 4 segments from 2 intersecting line segments    */
list<Segment_2> getsubsegments (set<Segment_2, seg_compare> segs) {
    list<Segment_2> subsegs;

    CGAL::compute_subcurves(segs.begin(), segs.end(), back_inserter(subsegs));

    return subsegs;
}

/* Build an arrangement of non-intersecting polygons from a set of segments */
Arrangement getarrangementfromsegments (list<Segment_2> subsegs) {
    Arrangement arr;

    for (list<Segment_2>::iterator it = subsegs.begin();
                                      it != subsegs.end(); it++) {
        CGAL::insert(arr, *it);
    }

    return arr;
}

/* Create 3d facets from an arrangement of 2d polygons. The z value of each
   vertex is the z thickness of the original polyhedron */
static pair<vector<Point3d>, vector<vector<size_t> > >
       create3dfacets (Arrangement arr, PMRegion *p) {

    int _idx = 0;
    Arrangement::Face_const_iterator            fit;
    Arrangement::Ccb_halfedge_const_circulator  curr;
    map<Point2d, Point3d> points;
    map<Point3d, int> indices;
    vector<Point3d> xpoints;
    vector<vector<size_t> > xpolygons;

    p->zthicknessprepare();

    // Iterate over all 2d polygons in the interior of the arrangement
    for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
        if (!fit->is_unbounded()) {
            curr = fit->outer_ccb();
            Polygon poly;
            vector<size_t> facet;
        // Iterate over all vertices of a polygon
            do {
                Point2d p2d = curr->target()->point();
                poly.push_back(p2d);
                Point3d p3d;
                int idx;
        // Check, if the point was already processed
                if (points.count(p2d) == 0) {
            // Extend the 2d point to a 3d point with
            // the z-thickness of the polyhedron as
            // z coordinate
                    Kernel::FT z = p->zthickness(p2d);
                    p3d = Point3d(p2d.x(), p2d.y(), z);
                    points[p2d] = p3d;
                    indices[p3d] = idx = _idx++;
            // Insert it in the list of points
                    xpoints.push_back(p3d);
                } else {
            // Point already exists, retrieve it and its index
                    p3d = points[p2d];
                    idx = indices[p3d];
                }
        // Add the index of the point to the current facet
                facet.push_back(idx);
                ++curr;
            } while (curr != fit->outer_ccb());
        // Add the facet to the set of 3d polygons
            xpolygons.push_back(facet);
        }
    }

    // The outer boundary of the arrangement is added with all z
    // coordinates set to 0. This will be the baseplate of the
    // cdpolyhedron
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

    return pair<vector<Point3d>,
           vector<vector<size_t> > >(xpoints, xpolygons);
}

/* Create a polyhedron from a list of points and facets */
static Polyhedron createpolyhedronfromfacets (vector<Point3d> points,
        vector<vector<size_t> > polygons) {
    Polyhedron p;
    CGAL::Polygon_mesh_processing::orient_polygon_soup(points, polygons);
    CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points,
            polygons, p);
   CGAL::Polygon_mesh_processing::triangulate_faces(p);

    return p;
}

/* Calculate a coverduration polyhedron from a polyhedron representing a
 * moving region.                                                        */
PMRegion PMRegion::createcdpoly() {
    // Project all edges of the polyhedron's boundary to 2d
    set<Segment_2, seg_compare> segs = getprojectedsegments(polyhedron);
    
    // Create non-intersecting subsegments
    list<Segment_2> subsegs = getsubsegments(segs);

    // Build a 2d arrangement from these segments
    Arrangement arr = getarrangementfromsegments(subsegs);

    // Create 3d facets from the 2d polygons in the arrangement
    pair<vector<Point3d>,
        vector<vector<size_t> > > facets = create3dfacets(arr, this);

    // Build a polyhedron from these 3d facets
    Polyhedron p = createpolyhedronfromfacets(facets.first, facets.second);

    PMRegion ret(p);

    return ret;
}

/* Calculate a coverduration polyhedron from a polyhedron representing a
 * moving region related to a given baseregion. */
PMRegion PMRegion::createcdpoly(RList baseregion) {
    PMRegion cdpoly = createcdpoly();
    Kernel::FT maxz = cdpoly.minmaxz().second;
    PMRegion baseregpoly = PMRegion::fromRegion(baseregion, -1, maxz+1, 0);

    return cdpoly * baseregpoly;
}

/* Change the base region for a coverduration polyhedron */
PMRegion PMRegion::restrictcdpoly(RList baseregion) {
    Kernel::FT maxz = minmaxz().second;
    PMRegion baseregpoly = PMRegion::fromRegion(baseregion, -1, maxz+1, 0);

    return *this * baseregpoly;
}

PMRegion PMRegion::createccdpoly() {
    return createccdpoly(traversedarea());
}

/* Calculate a complementary coverduration polyhedron from a polyhedron
 * representing a moving region related to a given baseregion. */
PMRegion PMRegion::createccdpoly(RList baseregion) {
    // First, create a coverduration polyhedron
    PMRegion cdpoly = createcdpoly();

    // Then extrude the base region to the height of the cdpolyhedron
    // (and slightly above, to keep the polyhedron simple)
    Kernel::FT maxz = cdpoly.minmaxz().second + 1;
    PMRegion baseregpoly = PMRegion::fromRegion(baseregion, 0, maxz, 0);

    return baseregpoly - cdpoly;
    
    // Create the intersection between the cdpolyhedron and the extruded
    // base region
    PMRegion tmp = cdpoly * baseregpoly;

    // and finally return the difference between the baseregion polyhedron
    // and the restricted cdpolyhedron
    return baseregpoly - tmp;
}

/* Calculate an interval coverduration polyhedron from a polyhedron
 * representing a moving region related to a given baseregion. */
PMRegion PMRegion::createicdpoly(Kernel::FT duration, RList baseregion) {
    // First, create a coverduration polyhedron restricted
    // to the given baseregion
    PMRegion cdpoly = createcdpoly(baseregion);

    // Copy the cdpolyhedron and translate it according to the given duration
    PMRegion tmp;
    tmp.polyhedron = cdpoly.polyhedron;
    tmp.translate(0, 0, -duration);

    // Subtract the cdpolyhedron with the translated version of itself
    return cdpoly - tmp;
}

/* Calculate an interval coverduration polyhedron from a polyhedron
 * representing a moving region. */
PMRegion PMRegion::createicdpoly(Kernel::FT duration) {
    // First, create a coverduration polyhedron
    PMRegion cdpoly = createcdpoly();

    // Copy the cdpolyhedron and translate it according to the given duration
    PMRegion tmp;
    tmp.polyhedron = cdpoly.polyhedron;
    tmp.translate(0, 0, -duration);

    // Subtract the cdpolyhedron with the translated version of itself
    return cdpoly - tmp;
}


// The next three functions do all the same, the only difference is, on
// which type of coverduration polyhedron (cd, ccd or icd) they are
// applied.

/* This function operates on a coverduration polyhedron (cdpolyhedron) and
   returns the area, which is covered at least for the given duration     */
RList PMRegion::coveredlonger(Kernel::FT duration) {
    return atinstant(duration);
}

/* This function operates on a complementary coverduration polyhedron
 * (ccdpolyhedron) and returns the area, which is covered at most for
   the given duration                                                */
RList PMRegion::coveredshorter(Kernel::FT duration) {
    return atinstant(duration);
}

/* This function operates on an interval coverduration polyhedron
   (icdpolyhedron) and returns the area, which is covered at least for
   the given duration and at most for duration + t, where t is the
   duration specified when the icdpolyhedron was created.             */
RList PMRegion::intervalcovered(Kernel::FT duration) {
    return atinstant(duration);
}

/* This function calculates the average cover time of the (moving) region */
Kernel::FT PMRegion::avgcover() {
    // Calculate the volume of the cdpolyhedron
    Kernel::FT v = CGAL::Polygon_mesh_processing::volume(polyhedron);

    // Calculate the area of the "traversed area"
    Kernel::FT a = ::area(projectxy());

    return v/a;
}

/* This function calculates the average cover time of the given base region
   by the (moving) region. The same base region must have been specified when
   creating the cdpolyhedron. */
Kernel::FT PMRegion::avgcover(RList basereg) {
    // Calculate the volume of the cdpolyhedron
    Kernel::FT v = CGAL::Polygon_mesh_processing::volume(polyhedron);
    
    // Calculate the area of the base region
    Kernel::FT a = ::area(Region2Polygons(basereg));

    // The average cover duration is the volume divided by the area
    return v/a;
}

}

#endif /* PMREGION_DISABLE_COVERDURATION */

