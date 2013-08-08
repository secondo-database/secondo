/*
*/

#include "interpolate.h"

Pt::Pt() {
}

Pt::Pt(int x, int y) : x(x), y(y) {
}

bool Pt::operator<(const Pt& a) const {
    return ((y < a.y) || ((y == a.y) && (x < a.x)));
}

bool Pt::operator==(const Pt& a) const {
    return ((x == a.x) && (y == a.y));
}

bool Pt::sortAngle(const Pt& a) const {
    return (angle < a.angle);
}

void Pt::calcAngle(const Pt& pt) {
    double tmpx = x - pt.x;
    double tmpy = y - pt.y;
    double hyp = sqrt(tmpx * tmpx + tmpy * tmpy);
    angle = acos(tmpx / hyp);
}

Pt Reg::GetMinXY() {
    int minx = INT_MAX;
    int miny = INT_MAX;
    
    for (unsigned int i = 0; i < v.size(); i++) {
        if (v[i].x1 < minx)
            minx = v[i].x1;
        if (v[i].y1 < miny)
            miny = v[i].y1;
    }
    
    return Pt(minx, miny);
}

string Pt::ToString() {
    std::ostringstream ss;
    ss << "(" << x << "/" << y << ")@" << angle;
    return ss.str();
}

int Pt::distance(Pt p) {
    return sqrt((p.x-x)*(p.x-x)+(p.y-y)*(p.y-y));
}