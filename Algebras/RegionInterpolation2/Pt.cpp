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
    return (x == a.x && y == a.y);
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

Pt Pt::operator*(const Pt& a) const {
    return Pt(x*a.x,y*a.y);
}

bool Pt::sortAngle(const Pt& a) const {
    return (angle < a.angle);
}

void Pt::calcAngle(const Pt& pt) {
    long double tmpx = x - pt.x;
    long double tmpy = y - pt.y;
    long double hyp = sqrt(tmpx * tmpx + tmpy * tmpy);
    angle = acos(tmpx / hyp);
    dist = hyp;
}

string Pt::ToString() const {
    std::ostringstream ss;
    ss << "(" << x << "/" << y << ")";
    return ss.str();
}

double Pt::sign(const Pt& a, const Pt& b, const Pt& c) {
    return ((a.x-c.x)*(b.y-c.y)-(a.y-c.y)*(b.x-c.x));
}

//static double sign (const Pt& p1, const Pt& p2, const Pt& p3) {
//    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
//}

bool Pt::insideTriangle(const Pt& a, const Pt& b, const Pt& c, const Pt& x) {
    bool b1, b2, b3;
    
    b1 = Pt::sign(x, a, b) < 0;
    b2 = Pt::sign(x, b, c) < 0;
    b3 = Pt::sign(x, c, a) < 0;
    
    return ((b1 == b2) && (b2 == b3));
}

double Pt::distance(Pt p) {
    return sqrt((p.x-x)*(p.x-x)+(p.y-y)*(p.y-y));
}