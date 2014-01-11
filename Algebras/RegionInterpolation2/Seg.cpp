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

double Seg::angle() const {
    double ret;
    double dx = e.x - s.x;
    double dy = e.y - s.y;

    if (e.x == s.x) {
        ret = (e.y < s.y) ? -M_PI / 2 : M_PI / 2;
    } else {
        ret = atan((double(dy)) / (double(dx)));
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

vector<Seg> Seg::sortSegs(vector<Seg> v) {
    vector<Seg> ret;

    if (v.size() == 0)
        return ret;
    
    int start = -1, start2 = -1;
    double minx = 0, miny = 0;
    Seg minseg1, minseg2;
    

    // Find the lowest point
    for (unsigned int i = 0; i < v.size(); i++) {
        if ((v[i].s.y < miny) || ((v[i].s.y == miny) &&
                (v[i].s.x < minx)) || (start < 0)) {
            start = i;
            miny = v[i].s.y;
            minx = v[i].s.x;
            minseg1 = v[i];
        }
    }

    for (unsigned int i = 0; i < v.size(); i++) {
        if ((v[i].e.x == minx) && (v[i].e.y == miny)) {
            start2 = i;
            minseg2 = v[i];
        }
    }

    minseg2.ChangeDir();
    if (minseg2.angle() < minseg1.angle()) {
        for (unsigned int i = 0; i < v.size(); i++) {
            v[i].ChangeDir();
        }
        start = start2;
    }

    ret.push_back(v[start]);
    Seg cur = v[start];
    for (unsigned int j = 0; j < v.size(); j++) {
        for (unsigned int i = 0; i < v.size(); i++) {
            if ((v[i].s.x == cur.e.x) && (v[i].s.y == cur.e.y)) {
                if (!(v[i] == v[start]))
                    ret.push_back(v[i]);
                cur = v[i];
                break;
            }
        }
    }

    return ret;
}
