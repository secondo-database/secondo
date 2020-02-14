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
    tree.accelerate_distance_queries();
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
RList PMRegion::atinstant (Kernel::FT instant) {
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

        Kernel::FT deltat = (cur - prev) / 86400000;
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
        ret = ret + sqrt(CGAL::to_double(Kernel::Segment_2(he.source()->point(),
                        he.target()->point()).squared_length()));
    }

    return ret;
}

/* Operation "area" for a pmregion */
static Kernel::FT getAreaFromArrangement (Arrangement& arr);
MReal PMRegion::area () {
    MReal mreal;
    cerr << "Getting z events" << endl;
    // Get all z coordinates of the polyhedron in an ordered set
    std::set<Kernel::FT> zevents = getZEvents(polyhedron);
    cerr << "done" << endl;
    Kernel::FT prev;
    bool first = true;
    int i = 1;
    for (std::set<Kernel::FT>::iterator it = zevents.begin();
            it != zevents.end(); it++) {
        // For each consecutive pair of z coordinates ...
        if (!first) {
        cerr << "Number" << i++ << " / " << zevents.size() << endl;
            Kernel::FT cur = *it;

            // ... calculate the intersections with a cutting plane
            Arrangement a1 = zplanecut(polyhedron, prev+EPSILON);
            Arrangement a2 = zplanecut(polyhedron, cur-EPSILON);

        Kernel::FT deltat = (cur - prev) / 86400000;
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

vector<Polygon_with_holes_2> PMRegion::projectxy () {
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

    return oi;
}

/* Operation "traversedarea" for a pmregion */
RList PMRegion::traversedarea () {
    return Polygons2Region(projectxy());
}

void PMRegion::translate(Kernel::FT x, Kernel::FT y, Kernel::FT z) {
    Kernel::Vector_3 translate(x, y, z);
    for (Polyhedron::Vertex_iterator v = polyhedron.vertices_begin();
            v != polyhedron.vertices_end(); v++) {
        v->point() = v->point() + translate;
    }
}


/*************************************************************************/
/* Reimplementation of atinstant more optimized for large moving regions */
/*************************************************************************/

/* Represents a single 2D line segment */
class Seg {
    public:
        // start and end point
        Point_2 s, e;
        bool valid; // valid flag

        Seg(Point_2 s, Point_2 e) : s(s), e(e), valid(true) {};

        Seg() : valid(false) {};

        // 1, 0, -1 : p is leftOf, on, rightOf segment
        Kernel::FT sign (Point_2 p) {
            return p.x()*s.y() - p.x()*e.y() - e.x()*s.y() -
                p.y()*s.x() + p.y()*e.x() + e.y()*s.x();
        }

        // reverse this segment
        void reverse() {
            Point_2 tmp = s;
            s = e;
            e = tmp;
        }
};

/* represents a single 2d face */
class Face {
    public:
        // Enclosing parent face
        Face* parent;
        // Enclosed child faces
        vector<Face*> children;

        Face () : parent(NULL) {};

        // Convert this face to a nested list
        RList toRList() {
            RList ret;
            for (std::vector<Seg>::iterator si = segs.begin();
                    si != segs.end(); si++) {
                RList p;
                p.append(si->s.x());
                p.append(si->s.y());
                ret.append(p);
            }

            return ret;
        }

        // Append another segment
        void append (Seg s) {
            segs.push_back(s);
        }

        // Determines if point p is inside this face.
        // Uses the winding number algorithm
        bool inside (Point_2 p) {
            int wn = 0;
            // Check if the point is outside the bounding box of this face
            if (p.x() < llx || p.x() > urx || p.y() < lly || p.y() > ury)
                return false;

            for (std::vector<Seg>::iterator si = segs.begin();
                                                 si != segs.end(); si++) {
                if (si->s.y() <= p.y()) {
                    if (si->e.y() > p.y()) {
                        Kernel::FT sign = si->sign(p);
                        if (sign > 0) {
                            wn++;
                        } else if (sign == 0) {
                            return 0;
                        }
                    }
                } else {
                    if (si->e.y() <= p.y()) {
                        Kernel::FT sign = si->sign(p);
                        if (sign < 0) {
                            wn--;
                        } else if (sign == 0) {
                            return 0;
                        }
                    }
                }
            }
            return wn != 0;
        }

        // Returns true if Face f is inside this face
        bool inside (Face& f) {
            // Call inside for an arbitrary point of the face
            return inside(f.segs.begin()->s);
        }

        // Reverse the segments and the order of segments
        void reverse () {
            std::reverse(segs.begin(), segs.end());
            std::vector<Seg>::iterator si;
            for (si = segs.begin(); si != segs.end(); si++) {
                si->reverse();
            }
        }

        // Sort this face. After that, the face
        // - starts with the most lower left point
        // - is in counter clockwise order
        void sort() {
            std::vector<Seg>::iterator si, best = segs.begin();
            llx = urx = best->s.x();
            lly = ury = best->s.y();
            for (si = segs.begin()+1; si != segs.end(); si++) {
                if (si->s.x() < llx)
                    llx = si->s.x();
                else if (si->s.x() > urx)
                    urx = si->s.x();
                if (si->s.y() < lly)
                    lly = si->s.y();
                else if (si->s.y() > ury)
                    ury = si->s.y();
                if (si->s.y() < best->s.y() || 
                      ((si->s.y() == best->s.y())&&(si->s.x() < best->s.x())))
                    best = si;
            }
            vector<Seg> n;
            n.insert(n.end(), best, segs.end());
            if (best != segs.begin())
                n.insert(n.end(), segs.begin(), best-1);
            //           std::move(best, segs.end(), segs.begin());
            segs = n;
            if (ccw()) {
                // We want the segments to be clockwise for faces
                reverse();
            }
        }

        // Checks, if the face is in counter clockwise order
        bool ccw () {
            // The first and the last segments both contain the most
            // lower left point, so these are part of the convex hull
            // of this face.
            Seg first = segs.front();
            Seg last = segs.back();

            // Now if the start point of the last segment is left
            // of the first segment, the face is in ccw order.
            return first.sign(last.s) > 0;
        }

    private:
        // ordered list of face segments
        std::vector<Seg> segs;
        // bounding coordinates
        Kernel::FT llx, lly, urx, ury;

};

/* fixTopology establishes the topological "inside" relationship between
 * a given vector of faces. After this function, all faces point to their
 * parent (if appropriate) and have their direct children in a "children"
 * vector. Indirect children are not inside this vector. After this, the
 * given vector only contains the top-level faces.
 */
static void fixTopology (vector<Face*>& fcs) {
    // Compare each face with each other face. This might be accelerated with
    // some kind of sweepline algorithm.
    for (vector<Face*>::iterator fi = fcs.begin(); fi != fcs.end(); fi++) {
        for (vector<Face*>::iterator fj = fcs.begin();
                                                    fj != fcs.end(); fj++) {
            if (fi == fj)
                continue;
            if ((*fj)->inside(**fi)) { // fi is inside fj
                Face *parent = (*fi)->parent;
                if (parent) { // fi already has another parent
                    if (parent->inside(**fj)) {
                        // fj is inside the current parent, so this is our
                        // direct parent (or at least more direct than the
                        // current parent. If there is a direct parent, this
                        // will be fixed in future iterations). Now we remove
                        // the existing relationship and build a new one to
                        // the new parent fj.
                        parent->children.erase(std::remove(
                                    parent->children.begin(),
                                    parent->children.end(), *fj),
                                parent->children.end());
                        (*fi)->parent = *fj;
                        (*fj)->children.push_back(*fi);
                    }
                } else {
                    // Establish the newly-found relationship
                    (*fi)->parent = *fj;
                    (*fj)->children.push_back(*fi);
                }
            }
        }
    }

    // Now remove all faces, that are not on the top-level
    std::vector<Face*>::iterator fi = fcs.begin();
    while (fi != fcs.end()) {
        // If a face has a parent, it is not on top-level
        if ((*fi)->parent)
            fi = fcs.erase(fi);
        else {
            fi++;
        }
    }
}

// SegSet manages a set of 2d line segments and aids the reconstruction
// of faces.
class SegSet {
    public:
    // Add another segment to the segment set
        void add (Seg s) {
            m[s.s].insert(s.e);
            m[s.e].insert(s.s);
        }

    // Add another segment by specifying start and end point
        void add (Point_2 s, Point_2 e) {
            m[s].insert(e);
            m[e].insert(s);
        }

    // Returns true if this SegSet is empty
        bool isEmpty() {
            return m.empty();
        }

    // Get a vector of segments from the set that start at the given point
        vector<Seg> get (Point_2 s) {
            vector<Seg> ret;

            std::map<Point_2, std::set<Point_2> >::iterator mi = m.find(s);
            if (mi != m.end()) {
                std::set<Point_2>::iterator i;
                for (i = mi->second.begin(); i != mi->second.end(); i++) {
                    ret.push_back(Seg(s, *i));
                }
            }

            return ret;
        }

        // Get a successor of the current segment. Remove the returned segment
        // from the segment set.
        Seg getSuccessor (Seg s) {
            Seg ret;

            std::map<Point_2, std::set<Point_2> >::iterator mi = m.find(s.e);
            if (mi != m.end()) {
                for (std::set<Point_2>::iterator i = mi->second.begin();
                                                 i != mi->second.end(); i++) {
                    if (!(*i == s.s)) { // Do not return the reversed segment
                        ret = Seg(s.e, *i);
                        break;
                    }
                }
            }

            if (ret.valid)
                remove(ret);

            return ret;
        }

        // Get an arbitrary segment from this segment set. The segment
        // is removed from this set.
        Seg getSomeSeg () {
            Seg ret;

            if (!isEmpty()) {
                Point_2 s = *m.begin()->second.begin();
                Point_2 e = *m[s].begin();

                ret = Seg(s, e);
                remove(ret);
            }

            return ret;
        }

        // Remove the given segment from this segment set.
        void remove (Seg s) {
            std::map<Point_2, std::set<Point_2> >::iterator mi = m.find(s.s);
            if (mi != m.end()) {
                mi->second.erase(s.e);
                if (mi->second.size() == 0) {
                    m.erase(mi);
                }
            }

            mi = m.find(s.e);
            if (mi != m.end()) {
                mi->second.erase(s.s);
                if (mi->second.size() == 0) {
                    m.erase(mi);
                }
            }
        }

        // Construct faces from this segment set
        vector<Face*> getFaces() {
            vector<Face*> fcs;

            while (!isEmpty()) {
                Seg prev = getSomeSeg();
                Point_2 start = prev.s, last;
                Face *f = new Face();
                f->append(prev);
                while (!isEmpty()) {
                    Seg cur = getSuccessor(prev);
                    if (!cur.valid) {
                        cerr << "No successor found!" << endl;
                        break;
                    }

                    f->append(cur);
                    if (start == cur.e) {
                        f->sort();
                        fcs.push_back(f);
                        break;
                    }
                    prev = cur;
                }
            }

            fixTopology(fcs);

            return fcs;
        }

    private:
        // A map indexed by points and providing a
        // set of successor points.
        std::map<Point_2, std::set<Point_2> > m;
};

// Represents a polyhedron triangular facet. This class is used for
// fast projection of the triangle into 2d at a given z coordinate.
class Triangle {
    public:
        // The three 3D points of the triangle
        Point3d a, b, c;

        // Constructor, that orders the points according to their
        // z coordinate: p1 < p2 < p3
        Triangle (Point3d p1, Point3d p2, Point3d p3) {
            if (p1.z() < p2.z() && p1.z() < p3.z()) {
                a = p1;
                if (p2.z() < p3.z()) {
                    b = p2;
                    c = p3;
                } else {
                    b = p3;
                    c = p2;
                }
            } else if (p2.z() < p3.z()) {
                a = p2;
                if (p1.z() < p3.z()) {
                    b = p1;
                    c = p3;
                } else {
                    b = p3;
                    c = p1;
                }
            } else {
                a = p3;
                if (p1.z() < p2.z()) {
                    b = p1;
                    c = p2;
                } else {
                    b = p2;
                    c = p1;
                }
            }
        }

        // Tests, if the given z coordinate is  in the range of this triangle
        int between (Kernel::FT z) {
            return (z >= a.z() && z <= c.z());
        }

        // Projects the triangle to a 2d line segment at a given z coordinate
        Seg project (Kernel::FT z) {
            Kernel::FT p1x, p1y, p1z, p2x, p2y, p2z;
            Point3d p1, p2;

            assert (z >= a.z() && z <= c.z());
            if (z == a.z()) {
                p1 = a;
                if (z == b.z()) {
                    p2 = b;
                } else {
                    p2 = a;
                }
            } else if (z == c.z()) {
                p1 = c;
                if (z == b.z()) {
                    p2 = b;
                } else {
                    p2 = c;
                }
            } else {
                Kernel::FT frac1 = (z - a.z()) / (c.z() - a.z());
                p1x = a.x() + (c.x() - a.x())*frac1;
                p1y = a.y() + (c.y() - a.y())*frac1;
                p1z = z;

                Point3d t1, t2;
                if (z < b.z()) {
                    t1 = a;
                    t2 = b;
                } else {
                    t1 = b;
                    t2 = c;
                }
                Kernel::FT frac2 = (z - t1.z())/(t2.z() - t1.z());
                p2x = t1.x() + (t2.x() - t1.x())*frac2;
                p2y = t1.y() + (t2.y() - t1.y())*frac2;
                p2z = z;
                p1 = Point3d(p1x, p1y, p1z);
                p2 = Point3d(p2x, p2y, p2z);
            }

            return Seg(Point_2(p1.x(), p1.y()), Point_2(p2.x(), p2.y()));
        }
};


/* zplanecut2 calculates the intersection of
 * a polyhedron with a cutting plane at a given z coordinate */
SegSet zplanecut2 (Polyhedron polyhedron, Kernel::FT z) {
    SegSet ss;
    for (Facet_iterator f = polyhedron.facets_begin();
            f != polyhedron.facets_end(); f++) {
        Halfedge_facet_circulator hc = f->facet_begin();
        Point3d p1 = hc++->vertex()->point();
        Point3d p2 = hc++->vertex()->point();
        Point3d p3 = hc++->vertex()->point();
        assert(hc == f->facet_begin());
        if (p1.z() == p2.z() && p1.z() == p3.z())
            continue; // p1,p2,p3 are coplanar to a plane parallel to xy plane
        Triangle t(p1, p2, p3);
        if (t.between(z)) // is this triangle relevant for the projection?
            ss.add(t.project(z));
    }

    return ss;
}

// Append the face to the given nested list
static void addToRList (Face* fcs, RList& rl) {
    RList fc;

    // Add the main cycle
    fc.append(fcs->toRList());
    // Add all child cycles as holes
    for (std::vector<Face*>::iterator fi = fcs->children.begin();
                                       fi != fcs->children.end(); fi++) {
        // Holes have to be ccw, so change direction first
        (*fi)->reverse();
        fc.append((*fi)->toRList());
        // If a child cycle has children, add these as new faces
        for (std::vector<Face*>::iterator fj = (*fi)->children.begin();
                                     fj != (*fi)->children.end(); fj++) {
            addToRList(*fj, rl);
        }
    }

    rl.append(fc);
}

// Convert a segment set to a region in nested list format
static RList SegSet2Region(SegSet& ss) {
    vector<Face*> fcs = ss.getFaces();

    RList reg;
    for (std::vector<Face*>::iterator fi = fcs.begin(); fi != fcs.end(); fi++) {
        addToRList(*fi, reg);
        free(*fi);
    }

    return reg;
}


RList PMRegion::atinstant2 (Kernel::FT instant) {
    SegSet cut = zplanecut2(polyhedron, instant);

    // Convert the faces from the segment set to a region
    RList region = SegSet2Region(cut);

    return region.obj("region", "region");
}


}

/* vim: ts=4:sw=4:et */
