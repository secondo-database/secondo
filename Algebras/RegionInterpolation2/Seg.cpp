/*
*/

#include "MovingRegionAlgebra.h"
#include "interpolate.h"
#include <string>

Seg::Seg() : valid(0) {
}

Seg::Seg(Pt s, Pt e) : s(s), e(e), valid(1) {
}

bool Seg::operator<(const Seg& a) const {
    return ((s.y < a.s.y) || ((s.y == a.s.y) && (s.x < a.s.x)));
}

void Seg::ChangeDir() {
    Pt t = s;
    s = e;
    e = t;
}

bool Seg::operator==(const Seg& a) const {
    return ((s == a.s) && (e == a.e));
}

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

string Seg::ToString() const {
    std::ostringstream ss;

    ss << "Seg: " << s.ToString() << " " << e.ToString()
            << " " << " Angle: " << angle();

    return ss.str();
}

static inline bool isOnLine(Pt a, Pt b, Pt c) {
    return (b.x <= max(a.x, c.x) && b.x >= min(a.x, c.x) &&
            b.y <= max(a.y, c.y) && b.y >= min(a.y, c.y));
}

static inline int sign(Pt a, Pt b, Pt c) {
    double s = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);

    if (!s)
        return 0;
    else
        return (s > 0) ? 1 : -1;
}

bool Seg::intersects(const Seg& a) const {
    
    if ((this->e == a.s) || (this->s == a.e) ||
            (this->s == a.s) || (this->e == a.e))
        return false;
    
    int s1 = sign(s, e, a.s);
    int s2 = sign(s, e, a.e);
    int s3 = sign(a.s, a.e, s);
    int s4 = sign(a.s, a.e, e);

    if ((s1 != s2) && (s3 != s4)) {
        return true;
    }

    if ((s1 == 0 && isOnLine(s, a.s, e)) ||
            (s2 == 0 && isOnLine(s, a.e, e)) ||
            (s3 == 0 && isOnLine(a.s, s, a.e)) ||
            (s4 == 0 && isOnLine(a.s, e, a.e))) {
        return true;
    }
    

    return false;
}

