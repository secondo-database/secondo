/* 
 * This file is part of libpmregion
 * 
 * File:   PMRegion\_conversions.cpp
 * Author: Florian Heinz <fh@sysv.de>

 1 Conversions
   Conversions from PMRegion to other formats
   like MRegion and OFF and vice versa
 
*/


#include "PMRegion_internal.h"
#include <sstream>
#include <iomanip>
#include <map>
#include <ctime>

using namespace pmr;

namespace pmr {

/*
             C O N V E R S I O N S

*/


/*
   Conversion functions from and to OFF file format.
   This is directly handled by CGAL

*/

PMRegion PMRegion::fromOFF(std::string off) {
    PMRegion ret;
    std::stringstream ss;

    ss << off;
    ss >> ret.polyhedron;

    return ret;
}   

std::string PMRegion::toOFF() {
    std::stringstream ss;

    ss << setprecision(PRECISION) << polyhedron;

    return ss.str();
}





/*
  Conversion functions from and to mregions.
  The mregion must be in RList format.

*/

// build_from_mregion builds a polyhedron from a list of surfaces
template <class HDS> class build_from_mregion:public CGAL::Modifier_base<HDS> {
    public:
        RList& mc;
        double starttime, endtime;

        build_from_mregion(RList& _reg, double _st, double _et) :
            mc(_reg), starttime(_st), endtime(_et) {}

        void operator() (HDS& hds) {
            int _idx = 0;
            CGAL::Polyhedron_incremental_builder_3<HDS> pb(hds, true);

            vector<int> start, end;
            pb.begin_surface(10, 10, 10, Polyhedron_builder::ABSOLUTE_INDEXING);

            map<Point3d,int> pm;

            for (unsigned int l = 0; l < mc.items.size(); l++) {
                RList& mseg = mc.items[l];

                Point3d p(mseg.items[0].getNr(), mseg.items[1].getNr(),
                        starttime);
                Point3d q(mseg.items[2].getNr(), mseg.items[3].getNr(),
                        endtime);

                if (pm.count(p) == 0) {
                    pm[p] = _idx++;
                    pb.add_vertex(p);
                }

                if (pm.count(q) == 0) {
                    pm[q] = _idx++;
                    pb.add_vertex(q);
                }
            }

            for (unsigned int l = 0; l < mc.items.size(); l++) {
                RList& mseg = mc.items[l];
                RList& prev = ((l == 0)?mc.items[mc.items.size()-1] : 
                        mc.items[l-1]);
                Point3d p(mseg.items[0].getNr(), mseg.items[1].getNr(),
                        starttime);
                Point3d q(mseg.items[2].getNr(), mseg.items[3].getNr(),
                        endtime);
                Point3d prevp(prev.items[0].getNr(), prev.items[1].getNr(),
                        starttime);
                Point3d prevq(prev.items[2].getNr(), prev.items[3].getNr(),
                        endtime);

                if (!(prevp == p)) {
                    start.push_back(pm[p]);
                }
                if (!(prevq == q)) {
                    end.push_back(pm[q]);
                }


                pb.begin_facet();
                if (prevp == p) {
                    pb.add_vertex_to_facet(pm[p]);
                    pb.add_vertex_to_facet(pm[q]);
                    pb.add_vertex_to_facet(pm[prevq]);
                } else if (prevq == q) {
                    pb.add_vertex_to_facet(pm[p]);
                    pb.add_vertex_to_facet(pm[q]);
                    pb.add_vertex_to_facet(pm[prevp]);
                } else {
                    pb.add_vertex_to_facet(pm[p]);
                    pb.add_vertex_to_facet(pm[q]);
                    pb.add_vertex_to_facet(pm[prevq]);
                    pb.add_vertex_to_facet(pm[prevp]);
                }
                pb.end_facet();
            }
            if (start.size() >= 3) {
                pb.begin_facet();
                for (int i = start.size()-1; i >= 0; i--) {
                    pb.add_vertex_to_facet(start[i]);
                }
                pb.end_facet();
            }
            if (end.size() >= 3) {
                pb.begin_facet();
                for (unsigned int i = 0; i < end.size(); i++)
                    pb.add_vertex_to_facet(end[i]);
                pb.end_facet();
            }
            pb.end_surface();
        }
};

PMRegion PMRegion::fromMRegion (RList reg) {
    Nef_polyhedron np;

    double off = parsetime(reg.items[4].items[0].items[0].items[0].getString());

    RList& urs = reg.items[4];

    for (unsigned int i = 0; i < urs.items.size(); i++) {
        RList& ur = urs.items[i];
        RList& iv = ur.items[0];


        double starttime = parsetime(iv.items[0].getString()) - off;
        double endtime = parsetime(iv.items[1].getString()) - off;


        RList& fcs = ur.items[1];
        for (unsigned int j = 0; j < fcs.items.size(); j++) {
            RList& fc = fcs.items[j];
            for (unsigned int k = 0; k < fc.items.size(); k++) {
                RList& mc = fc.items[k];
                build_from_mregion<Polyhedron::HalfedgeDS> bm(mc, starttime,
                                     endtime);
                Polyhedron p;

                p.delegate(bm);
                Nef_polyhedron nnp(p);
                if (k == 0)
                    np = np + nnp;
                else
                    np = np - nnp;
            }
        }
    }

    Polyhedron p;
    if (np.is_simple())
        np.convert_to_polyhedron(p);
    PMRegion pmreg(p);

    return pmreg;
}



/* Convert a PMRegion to a classical (unit-based) MRegion. */

/* MovSeg represents a moving segment */
class MovSeg {
    public:
        Point3d is, ie, fs, fe;
        bool valid;
        MovSeg(Point3d is, Point3d ie, Point3d fs, Point3d fe) :
            is(is), ie(ie), fs(fs), fe(fe), valid(true) {}
        MovSeg() : valid(false) {}

        bool isnext(MovSeg& ms) {
            return ms.is == ie && ms.fs == fe;
        }
        bool isreversenext (MovSeg& ms) {
            return ms.ie == ie && ms.fe == fe;
        }
        void reverse () {
            Point3d tmp;
            tmp = is;
            is = ie;
            ie = tmp;
            tmp = fs;
            fs = fe;
            fe = tmp;
        }
            
        std::vector<MovSeg>::iterator findNext (std::vector<MovSeg>& vec) {
            for (std::vector<MovSeg>::iterator it = vec.begin();
                        it != vec.end(); it++) {
                if (isnext(*it))
                    return it;
                if (isreversenext(*it)) {
                    it->reverse();
                    return it;
                }
            }
            return vec.end();
        }
        string ToString() {
            std::stringstream ss;
            ss << is << " / " << ie << "  ---  " << fs << " / " << fe << endl;
            return ss.str();
        }

};
static std::vector<std::vector<MovSeg> >sortmcycle(std::vector<MovSeg> movsegs);
static MovSeg createMSegFromFacet (SHalfedge_const_handle h,
                          Kernel::FT min, Kernel::FT max);
RList PMRegion::toMRegion () {
    bool first = true;
    Kernel::FT prev;
    Nef_polyhedron np(polyhedron);
    set<Kernel::FT> zevents;

    for (Vertex_const_iterator v = np.vertices_begin();
                         v != np.vertices_end(); ++v) {
        zevents.insert(v->point().z());
    }

    RList uregs;
    for (std::set<Kernel::FT>::iterator it = zevents.begin();
                        it != zevents.end(); it++) {
        if (!first) {
            RList ureg;

            RList iv;

            iv.append(timestr(CGAL::to_double(prev)));
            iv.append(timestr(CGAL::to_double(*it)));
            iv.append(true);
            iv.append(std::distance(it, zevents.end()) == 1);
            ureg.append(iv);

            Nef_polyhedron result = np
                .intersection(Plane(Point3d(0, 0, prev), Vector(0, 0, -1)),
                               Nef_polyhedron::OPEN_HALFSPACE)
                .intersection(Plane(Point3d(0, 0,  *it), Vector(0, 0,  1)),
                                Nef_polyhedron::OPEN_HALFSPACE);

            vector<MovSeg> cycle;
            for (Halffacet_const_iterator f = result.halffacets_begin();
                                f != result.halffacets_end(); f++) {
                if (f->is_twin()) continue;
                for (Halffacet_cycle_const_iterator fc =f->facet_cycles_begin();
                     fc != f->facet_cycles_end(); fc++) {
                    if (fc.is_shalfedge()) {
                        MovSeg ms = createMSegFromFacet(fc, prev, *it);
                        if (ms.valid)
                            cycle.push_back(ms);
                    }
                }
            }
            std::vector<std::vector<MovSeg> > cycles = sortmcycle(cycle);
            RList mfaces;
            for (unsigned int i = 0; i < cycles.size(); i++) {
                RList mfacewithholes;
                RList mface;
                std::vector<MovSeg>& cyc = cycles[i];
                for (unsigned int j = 0;  j < cyc.size(); j++) {
                    MovSeg& ms = cyc[j];
                    RList msrl;
                    msrl.append(CGAL::to_double(ms.is.x()));
                    msrl.append(CGAL::to_double(ms.is.y()));
                    msrl.append(CGAL::to_double(ms.fs.x()));
                    msrl.append(CGAL::to_double(ms.fs.y()));
                    mface.append(msrl);
                }
                mfacewithholes.append(mface);
                mfaces.append(mfacewithholes);
            }
            ureg.append(mfaces);
            if (mfaces.items.size() > 0)
                uregs.append(ureg);
        }
        first = false;
        prev = *it;
    }

    return uregs.obj("mregion", "mregion");
}

/* Creates a moving segment from a polyhedron surface */
static MovSeg createMSegFromFacet (SHalfedge_const_handle h, Kernel::FT min,
                                                     Kernel::FT max) {
    SHalfedge_around_facet_const_circulator hc(h), he(hc);
    Kernel::FT prevz;
    bool found = false;
    for (int i = 0, dist = circulator_distance(hc, he); i < dist+1; i++) {
        Kernel::FT z = hc->source()->source()->point().z();
        if (i > 0 && prevz == max && z == min) {
            found = true;
            break;
        }
        hc++;
        prevz = z;
    }
    if (!found)
        return MovSeg();
    he = hc;
    Point3d is = hc->source()->source()->point(), ie;
    do {
        ie = hc->source()->source()->point();
        hc++;
    } while (hc->source()->source()->point().z() == min);
    Point3d fe = hc->source()->source()->point(), fs;
    do {
        fs = hc->source()->source()->point();
        hc++;
    } while (hc->source()->source()->point().z() == max);

    return MovSeg(is, ie, fs, fe);
}

/* Sorts the moving segments to cycles */
static std::vector<std::vector<MovSeg> > sortmcycle
                                           (std::vector<MovSeg> movsegs) {
    std::vector<std::vector<MovSeg> > ret;
    
    while (!movsegs.empty()) {
        std::vector<MovSeg> cycle;
        MovSeg first = *(movsegs.begin());
        cycle.push_back(first);
        MovSeg cur = first;
        movsegs.erase(movsegs.begin());
        Polygon_2 pi, pf;
        Point_2 piprev(0, 0), pifinal(0, 0);
        Point_2 pfprev(0, 0), pffinal(0, 0);
        do {
            std::vector<MovSeg>::iterator it = cur.findNext(movsegs);
            if (it == movsegs.end()) break; //  XXX
            assert(it != movsegs.end());
            cur = *it;
            cycle.push_back(cur);
            movsegs.erase(it);
            Point_2 pip(cur.is.x(), cur.is.y());
            if ((pip != piprev) && (pip != pifinal)) {
                pi.push_back(pip);
                if (pi.size() == 1)
                    pifinal = pip;
            }
            piprev = pip;
            Point_2 pfp(cur.fs.x(), cur.fs.y());
            if ((pfp != pfprev) && (pfp != pffinal)) {
                pf.push_back(pfp);
                if (pf.size() == 1)
                    pffinal = pfp;
            }
            pfprev = pfp;

            if (cur.isnext(first))
                break;
        } while (1);
        if ((pi.is_simple() && pi.orientation() == CGAL::CLOCKWISE) ||
        (pf.is_simple() && pf.orientation() == CGAL::CLOCKWISE)) {
            std::reverse(cycle.begin(), cycle.end());
            for (unsigned int i = 0; i < cycle.size(); i++)
                cycle[i].reverse();
        }
        ret.push_back(cycle);
    }

    return ret;
}





/*
  Conversion function from and to native pmregion rlist
  format

*/

PMRegion PMRegion::fromRList (RList rl) {
    RList& obj = rl.items[4];
    RList& points = obj.items[0];
    RList& facets = obj.items[1];
    std::stringstream off;

    off << "OFF" << endl;
    off << points.items.size() << " " << facets.items.size() << " 0" << endl;
    for (unsigned int i = 0; i < points.items.size(); i++) {
        RList& point = points.items[i];
        off << setprecision(100) << point.items[0].getNr() << " " <<
        point.items[1].getNr() << " " << point.items[2].getNr() << endl;
    }

    for (unsigned int i = 0; i < facets.items.size(); i++) {
        RList& facet = facets.items[i];
        off << facet.items.size();
        for (unsigned int j = 0; j < facet.items.size(); j++) {
            off << " " << ((int) facet.items[j].getNr());
        }
        off << endl;
    }

    return PMRegion::fromOFF(off.str());
}

RList PMRegion::toRList () {
    RList pmreg;

    std::map<Point3d, int> pmap;
    int idx = 0;
    RList points;
    for (Vertex_iterator v = polyhedron.vertices_begin();
                                  v != polyhedron.vertices_end(); ++v) {
        RList point;

        Point3d p = v->point();
        pmap[p] = idx++;
        point.append(CGAL::to_double(p.x()));
        point.append(CGAL::to_double(p.y()));
        point.append(CGAL::to_double(p.z()));
        points.append(point);
    }
    pmreg.append(points);

    RList faces;
    for (Facet_iterator f = polyhedron.facets_begin();
                                   f != polyhedron.facets_end(); f++) {
        Halfedge_facet_circulator h = f->facet_begin(), he(h);

        RList face;
        do {
            Point3d p = h->vertex()->point();
            face.append((double)pmap[p]);
        } while (++h != he);
        faces.append(face);
    }
    pmreg.append(faces);

    return pmreg.obj("pmregion", "pmregion");
}

void Arrangement2Region(Arrangement::Face_iterator fi, RList& region) {
    for (Arrangement::Hole_iterator fs = fi->holes_begin();
            fs != fi->holes_end(); fs++) {
        RList face;

        RList maincycle;
        Arrangement::Ccb_halfedge_circulator c = *fs, ec(c);
        do {
            RList point;
            point.append(CGAL::to_double(c->source()->point().x()));
            point.append(CGAL::to_double(c->source()->point().y()));
            maincycle.append(point);
        } while (++c != ec);
        face.append(maincycle);

        Arrangement::Face_handle ft = c->twin()->face();
        for (Arrangement::Hole_iterator hi = ft->holes_begin();
                hi != ft->holes_end(); hi++) {
            RList holecycle;
            Arrangement::Ccb_halfedge_circulator hs = *hi, he = hs;
            do {
                RList point;
                point.append(CGAL::to_double(hs->source()->point().x()));
                point.append(CGAL::to_double(hs->source()->point().y()));
                holecycle.append(point);
            } while (++hs != he);
            face.append(holecycle);

            Arrangement2Region(hs->twin()->face(), region);
        }
        region.prepend(face);
    }
}

RList Polygons2Region (vector<Polygon_with_holes_2> polygons) {
    RList faces;

    for (unsigned int i = 0; i < polygons.size(); i++) {
        RList face;
        RList cycle;
        Polygon_with_holes_2& p = polygons[i];
        for (PG2VI vi = p.outer_boundary().vertices_begin();
                                vi != p.outer_boundary().vertices_end(); vi++) {
            RList point;
            point.append(to_double(vi->x()));
            point.append(to_double(vi->y()));
            cycle.append(point);
        }
        face.append(cycle);
        for (PG2H h = p.holes_begin(); h != p.holes_end(); h++) {
            Polygon_2 po = *h;
            RList hole;
            for (PG2VI vi = po.vertices_begin(); vi != po.vertices_end();vi++) {
                RList point;
                point.append(to_double(vi->x()));
                point.append(to_double(vi->y()));
                hole.append(point);
            }
            face.append(hole);
        }
        faces.append(face);
    }

    return faces.obj("region", "region");
}

vector<Segment> mpoint2segments (RList& obj) {
    vector<Segment> segs;
    RList& upoints = obj.items[4];

    for (unsigned int i = 0; i < upoints.items.size(); i++) {
        RList& upoint = upoints.items[i];

        RList& iv = upoint.items[0];
        double z1 = parsetime(iv.items[0].getString());
        double z2 = parsetime(iv.items[1].getString());

        RList& points = upoint.items[1];
        double x1 = points.items[0].getNr();
        double y1 = points.items[1].getNr();
        double x2 = points.items[2].getNr();
        double y2 = points.items[3].getNr();

        Point3d p(x1, y1, z1);
        Point3d q(x2, y2, z2);

                segs.push_back(Segment(p, q));
        }

        return segs;
}

}
