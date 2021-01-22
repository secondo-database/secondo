/* 
 * This file is part of libpmregion
 * 
 * File:   PMRegion\_helpers.cpp
 * Author: Florian Heinz <fh@sysv.de>
 
 1 Helper functions
   Date/Time conversion 
   Range processing

*/

#include "PMRegion_internal.h"
#include <sstream>
#include <iomanip>
#include <map>
#include <ctime>

using namespace std;
using namespace pmr;

namespace pmr {

/* 
   Helper functions for converting date/time strings into a double
   representing the ms since unix epoch

*/

static double utctime (struct tm *tm) {
    char *tz;
    double ret;

    tz = getenv("TZ");
    setenv("TZ", "UTC", 1);
    tzset();
    ret = mktime(tm);
    if (tz)
        setenv("TZ", tz, 1);
    else
        unsetenv("TZ");
    tzset();

    return ret;
}

double parsetime (std::string str) {
    struct tm tm;
    unsigned int msec;
    char sep; // Separator, space or -

    tm.tm_year = tm.tm_mon = tm.tm_mday = 0;
    tm.tm_sec = tm.tm_min = tm.tm_hour = tm.tm_isdst = msec = 0;

    int st = sscanf(str.c_str(), "%u-%u-%u%c%u:%u:%u.%u",
            &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &sep,
            &tm.tm_hour, &tm.tm_min, &tm.tm_sec,
            &msec);
    if (st < 3) {
        return atof(str.c_str());
    }

    tm.tm_year -= 1900; // struct tm expects years since 1900
    tm.tm_mon--; // struct tm expects months to be numbered from 0 - 11

    double ret = utctime(&tm) * 1000 + msec;

    return ret;
}

string timestr(double t) {
    struct tm *tm;
    char buf[32], ret[40];
    time_t ti;
    ti = t/1000+3600;

    tm = gmtime(&ti);
    strftime(buf, sizeof(buf), "%F-%T", tm);
    sprintf(ret, "%s.%03d", buf, (int) fmod(t,1000));
    return ret;
}

string timestr(Kernel::FT t) {
    return timestr(::CGAL::to_double(t));
}

template <typename T> void Range<T>::addrange(T a, T b) {
    if (a > b)
        std::swap(a, b);
    typename std::map<T, T>::iterator it1 = range.upper_bound(a);
    if (it1 != range.begin() && (--it1,it1++)->second >= a)
        it1--;
    if (it1 != range.end() && it1->first < a)
        a = it1->first;
    typename std::map<T, T>::iterator it2(it1);
    while (it2 != range.end() && it2->first <= b)
        it2++;
    if (it2 != range.begin() && (--it2,it2++)->second > b)
        b = (--it2,it2++)->second;
    range.erase(it1, it2);
    range[a] = b;
}

template <typename T> void Range<T>::print() {
    typename std::map<T, T>::iterator it = range.begin();
    while (it != range.end()) {
        cerr << it->first << " - " << it->second << endl;
        it++;
    }
}

std::set<Kernel::FT> getZEvents (Polyhedron p, Kernel::FT mindiff) {
    std::set<Kernel::FT> zevents;
    Kernel::FT max;
    bool first = true;
    for (Vertex_iterator v = p.vertices_begin(); v != p.vertices_end(); ++v) {
        Kernel::FT z = v->point().z();
        if (first) {
            first = false;
            max = z;
        }
        if (max < z) max = z;
        zevents.insert(v->point().z());
    }

    first = true;
    Kernel::FT prev;
    for (std::set<Kernel::FT>::iterator it = zevents.begin();
            it != zevents.end(); ) {
        Kernel::FT cur = *it;
        if (!first && cur - prev < mindiff) {
            it = zevents.erase(it);
            continue;
        }
        first = false;
        prev = cur;
        ++it;
    }
    zevents.insert(max);

    return zevents;
}

std::set<Kernel::FT> getZEvents (Polyhedron p) {
    return getZEvents(p, -1);
}

pair<Kernel::FT, Kernel::FT> PMRegion::minmaxz() {
    Kernel::FT min = 0, max = 0;

    for (Vertex_iterator v = polyhedron.vertices_begin();
                                 v != polyhedron.vertices_end(); ++v) {
        Point3d p = v->point();
        Kernel::FT z = p.z();
        if (z != 0 && (z < min || min == 0))
            min = z;
        if (z > max)
            max = z;
    }
    return pair<Kernel::FT, Kernel::FT>(min, max);
}

pair<Point3d, Point3d> PMRegion::boundingbox() {
    Kernel::FT minx, miny, minz, maxx, maxy, maxz;
    bool first = true;

    for (Vertex_iterator v = polyhedron.vertices_begin();
                                 v != polyhedron.vertices_end(); ++v) {
        Point3d p = v->point();
        Kernel::FT x = p.x();
        Kernel::FT y = p.y();
        Kernel::FT z = p.z();
    if (first) {
            minx = x;
            maxx = x;
            miny = y;
            maxy = y;
            minz = z;
            maxz = z;
        first = false;
    } else {
        if (x < minx) minx = x;
        if (x > maxx) maxx = x;
        if (y < miny) miny = y;
        if (y > maxy) maxy = y;
        if (z < minz) minz = z;
        if (z > maxz) maxz = z;
    }
    }
    return pair<Point3d, Point3d>(Point3d(minx, miny, minz),
            Point3d(maxx, maxy, maxz));
}

void PMRegion::analyze() {
    vector<pair<Kernel::FT,Kernel::FT> > ranges;
    for (Facet_iterator f = polyhedron.facets_begin();
            f != polyhedron.facets_end(); f++) {
        Halfedge_facet_circulator hc = f->facet_begin();
        Kernel::FT min, max;
        min = max = hc->vertex()->point().z();
        while (++hc != f->facet_begin()) {
            Kernel::FT z = hc->vertex()->point().z();
            if (z < min)
                min = z;
            if (z > max)
                max = z;
        }
        if (min != max) 
            ranges.push_back(pair<Kernel::FT, Kernel::FT>(min, max));
    }
    std::set<Kernel::FT> zevents = getZEvents(polyhedron);
    std::set<Kernel::FT>::iterator it = zevents.begin();
    Kernel::FT prev = *it;
    int sum = 0;
    while (++it != zevents.end()) {
        Kernel::FT z = *it;
        Kernel::FT med = (z+prev)/2;
        int count = 0;
        for (std::vector<pair<Kernel::FT,Kernel::FT> >::iterator ri =
                ranges.begin(); ri != ranges.end(); ++ri) {
            if (ri->first < med && ri->second > med) {
                count++;
                sum++;
            }
        }
        prev = z;
    }
    int vertices = distance(polyhedron.vertices_begin(),
            polyhedron.vertices_end());
    int facets = distance(polyhedron.facets_begin(), polyhedron.facets_end());
    int units = zevents.size()-1;
    int msegs = sum;
    cerr << vertices << ", " << facets << ", "<< units << ", " << msegs << endl;
}

set<Segment_2, seg_compare> getprojectedsegments(Polyhedron polyhedron);
list<Segment_2> getsubsegments (set<Segment_2, seg_compare> segs);
Arrangement getarrangementfromsegments(list<Segment_2> segs);
void PMRegion::openscad(string filename) {
    Kernel::FT zmin, zmax;

    for (Point_iterator pi = polyhedron.points_begin();
            pi != polyhedron.points_end(); ++pi) {
        Kernel::FT z = pi->z();
        if (pi == polyhedron.points_begin()) {
            zmin = z;
            zmax = z;
        } else {
            if (z < zmin)
                zmin = z;
            if (z > zmax)
                zmax = z;
        }
    }
    ofstream scad;
    scad.open(filename + ".scad");
    scad << "scale([1, 1, 0.0005]) {" << endl
         << "    color(polycol) import(\"" << filename << ".off\");" << endl
         << "}" << endl
         ;
    scad.close();

    scad.open(filename + "poles.scad");
    scad << "polecol = \"red\";" << endl
         << "polediameter = 2;" << endl
         << "polycol = \"yellow\";" << endl
         ;
    scad << "module pole (x, y, zmin, zmax) {" << endl
         << "  color(polecol)" << endl
         << "    translate([x, y, zmin-(zmax-zmin)*0.05])" << endl
         << "        cylinder(d=polediameter,h=(zmax-zmin)*1.1);" << endl
         << "}" << endl << endl
         ;
    scad << "scale([1, 1, 0.0005]) {" << endl;
    for (Point_iterator pi = polyhedron.points_begin();
            pi != polyhedron.points_end(); ++pi) {
        Kernel::FT x = pi->x();
        Kernel::FT y = pi->y();
        scad << "    pole(" << x << ", " << y << ", " << zmin << ", " <<
            zmax << ");" << endl;
    }
    scad << "}" << endl;
    scad.close();
    scad.open(filename + ".off");
    scad << polyhedron;
    scad.close();

    set<Segment_2, seg_compare> _segs = getprojectedsegments(polyhedron);
    list<Segment_2> segs = getsubsegments(_segs);
    scad.open(filename + "mesh.scad");
    scad << "include <mypoly.scad>" << endl;

    Arrangement arr = getarrangementfromsegments(segs);
    Arrangement::Face_const_iterator            fit;
    Arrangement::Ccb_halfedge_const_circulator  curr;

    for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
        if (!fit->is_unbounded()) {
            curr = fit->outer_ccb();
            Polygon poly;
            scad << "mypoly([";
            do {
                Point2d p2d = curr->target()->point();
                scad << "[" << p2d.x() << "," << p2d.y() << "]";
                ++curr;
                if (curr != fit->outer_ccb()) {
                    scad << ",";
                }
            } while (curr != fit->outer_ccb());
            scad << "]);" << endl;
        }
    }

    scad.close();
}

void PMRegion::toFile(string filename) {
    ofstream out;
    out.open(filename);
    out << toRList().ToString();
    out.close();
}


// Keep the linker happy
void rangeinstance () {
    Range<Kernel::FT> dummy;
    dummy.addrange(0, 0);
}

}
