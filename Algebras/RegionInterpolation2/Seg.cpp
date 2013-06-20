/*
*/

#include "MovingRegionAlgebra.h"
#include "interpolate.h"
#include <string>

Seg::Seg() {
}

Seg::Seg(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {
}

bool Seg::operator<(const Seg& a) const {
    return ((y1 < a.y1) || ((y1 == a.y1) && (x1 < a.x1)));
}

void Seg::ChangeDir() {
    int tx = x1;
    int ty = y1;
    x1 = x2;
    y1 = y2;
    x2 = tx;
    y2 = ty;
}

bool Seg::operator==(const Seg& a) const {
    return ((x1 == a.x1) && (y1 == a.y1) && (x2 == a.x2) && (y2 == a.y2));
}

double Seg::angle() const {
    double ret;
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (x2 == x1) {
        ret = (y2 < y1) ? -M_PI / 2 : M_PI / 2;
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

string Seg::ToString() {
    std::ostringstream ss;

    ss << "Seg: " << x1 << " " << y1 << " " << x2 
            << " " << y2 << " Angle: " << angle();

    return ss.str();
}

vector<Seg> sortSegs(vector<Seg> v) {
    vector<Seg> ret;

    int start = -1, start2 = -1, miny = 0, minx = 0;
    Seg minseg1, minseg2;

    // Find the lowest point
    for (unsigned int i = 0; i < v.size(); i++) {
        if ((v[i].y1 < miny) || ((v[i].y1 == miny) &&
                (v[i].x1 < minx)) || (start < 0)) {
            start = i;
            miny = v[i].y1;
            minx = v[i].x1;
            minseg1 = v[i];
        }
    }

    for (unsigned int i = 0; i < v.size(); i++) {
        if ((v[i].x2 == minx) && (v[i].y2 == miny)) {
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
            if ((v[i].x1 == cur.x2) && (v[i].y1 == cur.y2)) {
                if (!(v[i] == v[start]))
                    ret.push_back(v[i]);
                cur = v[i];
                break;
            }
        }
    }

    return ret;
}
