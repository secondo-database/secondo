/*
*/

#include "interpolate.h"

Pt::Pt() : valid(0) {
}

Pt::Pt(double x, double y) : x(x), y(y), valid(1) {
}

bool Pt::operator<(const Pt& a) const {
    return ((y < a.y) || ((y == a.y) && (x < a.x)));
}

bool Pt::operator==(const Pt& a) const {
    return (nearlyEqual(x, a.x) && nearlyEqual(y, a.y));
}

Pt Pt::operator-(const Pt& a) const {
    return Pt(x-a.x,y-a.y);
}

Pt Pt::operator+(const Pt& a) const {
    return Pt(x+a.x,y+a.y);
}

Pt Pt::operator/(const double a) const {
    return Pt(x/a,y/a);
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

string Pt::ToString() const {
    std::ostringstream ss;
    ss << "(" << x << "/" << y << ")";
    return ss.str();
}

double Pt::distance(Pt p) {
    return sqrt((p.x-x)*(p.x-x)+(p.y-y)*(p.y-y));
}