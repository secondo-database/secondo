/*
 1 Class Seg represents a 2D-Segment
 
*/

#include "interpolate.h"
#include <string>

Seg::Seg() : valid(0) {
}

Seg::Seg(Pt s, Pt e) : s(s), e(e), valid(1) {
}

/*
 1.1 ~less-operator~ establishes a well-defined order of the segments,
 preferring lower-left points. This is mainly used to compare two lists of
 segments and finding and/or removing duplicates.
 
*/
bool Seg::operator<(const Seg& a) const {
    if (s.y < a.s.y)
        return true;
    else if (s.y > a.s.y)
        return false;
    else if (s.x < a.s.x)
        return true;
    else if (s.x > a.s.x)
        return false;
    else if (e.y < a.e.y)
        return true;
    else if (e.y > a.e.y)
        return false;
    else if (e.x < a.e.x)
        return true;
    else if (e.x > a.e.x)
        return false;
    else
        return false;
}

/*
 1.2 ~ChangeDir~ changes the orientation of this segment
 
*/
void Seg::ChangeDir() {
    Pt t = s;
    s = e;
    e = t;
}

/*
 1.3 ~equality-operator~: Two segments are equal if their endpoints match.
  
*/
bool Seg::operator==(const Seg& a) const {
    return ((s == a.s) && (e == a.e));
}

/*
 1.4 ~angle~ calculates the angle of a Segment relative to the x-axis and
 returns it in degrees (0-360)
 
*/
long double Seg::angle() const {
    long double ret;
    long double dx = e.x - s.x;
    long double dy = e.y - s.y;

    if (e.x == s.x) {
        ret = (e.y < s.y) ? -M_PI / 2 : M_PI / 2;
    } else {
        ret = atan(dy / dx);
    }

    ret = ret * 90.0 / (M_PI / 2);
    if (dx < 0)
        ret += 180;
    else if (dy < 0)
        ret += 360;

    return ret;
}

/*
 1.5 ~ToString~ generates a textual representation of this object for
 debugging purposes.
 
*/
string Seg::ToString() const {
    std::ostringstream ss;

    ss << "Seg: " << s.ToString() << " " << e.ToString()
            << " " << " Angle: " << angle();

    return ss.str();
}

// helper-function isOnLine checks, if the point b is on the Segment (a c) under
// the precondition, that the three points a, b and c are collinear. Used by
// Seg::intersects
static inline bool isOnLine(Pt a, Pt b, Pt c) {
    return (b.x <= max(a.x, c.x) && b.x >= min(a.x, c.x) &&
            b.y <= max(a.y, c.y) && b.y >= min(a.y, c.y));
}

// sign determines the order of the points a, b, c (clockwise, collinear or
// counterclockwise)
static inline int sign(Pt a, Pt b, Pt c) {
    double s = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
    return (s > 0) ? 1 : ((s < 0) ? -1 : 0);
}

/*
 1.6 ~intersects~ checks, if this segment intersects with segment a.
 
*/
bool Seg::intersects(const Seg& a) const {
    if ((s == e) || (a.s == a.e))
        return false; // If any segment is degenerated, it cannot intersect
    
    // The two segments completely overlap
    if ((*this == a) || ((s == a.s) && (e == a.e)))
        return true;
    
    // If they only touch in one point it is not considered an intersection
    if ((e == a.s) || (s == a.e) || (s == a.s) || (e == a.e))
        return false;
    
    int s1 = sign(s, e, a.s);
    int s2 = sign(s, e, a.e);
    int s3 = sign(a.s, a.e, s);
    int s4 = sign(a.s, a.e, e);

    if ((s1 != s2) && (s3 != s4)) {
        // The segments intersect
        return true;
    }

    // One point of a segment is on the other segment, this is an intersection
    if ((s1 == 0 && isOnLine(s, a.s, e)) ||
            (s2 == 0 && isOnLine(s, a.e, e)) ||
            (s3 == 0 && isOnLine(a.s, s, a.e)) ||
            (s4 == 0 && isOnLine(a.s, e, a.e))) {
        return true;
    }

    return false; // no intersection otherwise
}
