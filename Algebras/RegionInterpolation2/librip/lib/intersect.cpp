/*
   1 Intersection checking

   A new method of intersection checking between moving segments based on
   projection

*/

#include "interpolate.h"

// Checks, if pt is left of line ls->le
// Ignore, if the line is degenerated to a point
static bool isLeft(Pt ls, Pt le, Pt pt) {
    return (ls == le) ||
            ((le.x*pt.y - le.x*ls.y - ls.x*pt.y + ls.x*ls.y -
              le.y*pt.x + le.y*ls.x + ls.y*pt.x - ls.y*ls.x) > 0);
}

static bool isLeft2(Pt ls, Pt le, Pt pt) {
    return (ls == le) ||
            ((le.x*pt.y - le.x*ls.y - ls.x*pt.y + ls.x*ls.y -
              le.y*pt.x + le.y*ls.x + ls.y*pt.x - ls.y*ls.x) > 0);
}

static bool isLeft3(Pt ls, Pt le, Pt pt) {
    return (ls == le) ||
            ((le.x*pt.y - le.x*ls.y - ls.x*pt.y + ls.x*ls.y -
              le.y*pt.x + le.y*ls.x + ls.y*pt.x - ls.y*ls.x) > 0);
}

static bool lineIntersectsMSeg(Pt ls, Pt le, MSeg ms) {
    Pt quad[4];
    Pt off = ls - le;

    quad[0] = ms.is;
    quad[1] = ms.ie;
    quad[2] = ms.fe + off;
    quad[3] = ms.fs + off;

    if (isLeft(quad[0], quad[1], quad[2]) && // Check, if quad is in ccw-order
        isLeft(quad[1], quad[2], quad[3]) && // Multiple checks needed, since
        isLeft(quad[2], quad[3], quad[0])) { // two points can be equal
    
        bool ret = isLeft(quad[0], quad[1], ls) &&
                isLeft(quad[1], quad[2], ls) &&
                isLeft(quad[2], quad[3], ls) &&
                isLeft(quad[3], quad[0], ls);
    if (ret) {
                  isLeft2(quad[0], quad[1], ls) &&
                  isLeft2(quad[1], quad[2], ls) &&
                  isLeft2(quad[2], quad[3], ls) &&
                  isLeft2(quad[3], quad[0], ls);
    }
    return ret;

    } else {
        bool ret = isLeft(quad[1], quad[0], ls) &&
                   isLeft(quad[2], quad[1], ls) &&
                   isLeft(quad[3], quad[2], ls) &&
                   isLeft(quad[0], quad[3], ls);
    if (ret) {
                   isLeft3(quad[1], quad[0], ls) &&
                   isLeft3(quad[2], quad[1], ls) &&
                   isLeft3(quad[3], quad[2], ls) &&
                   isLeft3(quad[0], quad[3], ls);
    }
    return ret;
    }
}

// Checks, if the linesegments l1s->l1e and l2s->l2e intersect. Touching or
// overlapping segments are not seen as intersecting
static bool lineIntersectsLine (Pt l1s, Pt l1e, Pt l2s, Pt l2e) {
    // If one segment is degenerated to a point, no intersection can occur
    if (l1s == l1e || l2s == l2e)
        return false;
    
    Pt u = (l1e - l1s);
    Pt v = (l2e - l2s);
    Pt w = (l1s - l2s);
    
    double D = u.cross(v);
    if (D) { // The segments are not parallel
        float i = u.cross(w) / D;
        if (i <= 0.0 || i >= 1.0)
            return false;
        i = v.cross(w) / D;
        if (i <= 0.0 || i >= 1.0)
            return false;
        return true;
    }
    
    // Segments are parallel, so they do not intersect
    return false;
}

bool trapeziumIntersects2(MSeg s1, MSeg s2, unsigned int& detailedResult) {
    detailedResult = 0;

    bool res = 
            lineIntersectsMSeg(s1.is, s1.fs, s2) ||
            lineIntersectsMSeg(s1.ie, s1.fe, s2) ||
            lineIntersectsMSeg(s2.is, s2.fs, s1) ||
            lineIntersectsMSeg(s2.ie, s2.fe, s1) ||
            lineIntersectsLine(s1.is, s1.ie, s2.is, s2.ie) ||
            lineIntersectsLine(s1.fs, s1.fe, s2.fs, s2.fe) ||
            false
            ;
    
    if (res) {
        DEBUG(2, "Found intersection of " << s1.ToString() << " => " <<
                s2.ToString());
    }

    return res;
}
