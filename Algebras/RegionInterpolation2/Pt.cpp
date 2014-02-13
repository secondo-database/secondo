/*
 1 Class Pt represents a single point.

*/

#include "interpolate.h"

Pt::Pt() : valid(0) {
}

Pt::Pt(double x, double y) : x(x), y(y), valid(1) {
}

/*
 1.1 Less-operator sorts by y-value as first criterion, then by x-value
 
*/
bool Pt::operator<(const Pt& a) const {
    return ((y < a.y) || ((y == a.y) && (x < a.x)));
}

/*
 1.1 Equality-operator: Two points are equal if their x- and y-coordinates match
 
*/
bool Pt::operator==(const Pt& a) const {
    return (x == a.x && y == a.y);
}

/*
 1.2 Minus-operator calculates the x- and y-offset of two points.
 
*/
Pt Pt::operator-(const Pt& a) const {
    return Pt(x-a.x,y-a.y);
}

/*
 1.3 Plus-operator returns the point which is composed from the sum of the
 coordinates of the two points to be added.
 
*/
Pt Pt::operator+(const Pt& a) const {
    return Pt(x+a.x,y+a.y);
}

/*
 1.4 Division divides both coordinates by a scale factor
 
*/
Pt Pt::operator/(const double a) const {
    return Pt(x/a,y/a);
}

/*
 1.4 Multiplication multiplies both coordinates by a scale factor
 
*/
Pt Pt::operator*(const Pt& a) const {
    return Pt(x*a.x,y*a.y);
}

// Helper-function to sort the points by their (pre-calculated) polar
// coordinates. If the angle is equal, then sort by descending (!) distance
bool Pt::lessPolar(const Pt& a) const {
    return ((angle < a.angle) || ((angle == a.angle) && (dist > a.dist))) ;
}

/*
  1.5 calcPolar calculates the polar coordinates of this point relative to the
  origin ~origin~. This sets the attributes ~angle~ and ~distance~ of this
  object.
 
*/
void Pt::calcPolar(const Pt& origin) {
    long double tmpx = x - origin.x;
    long double tmpy = y - origin.y;
    long double hyp = sqrt(tmpx * tmpx + tmpy * tmpy);
    angle = acos(tmpx / hyp);
    dist = hyp;
}

/*
  1.6 ToString returns a string-representation of this object
 
*/
string Pt::ToString() const {
    std::ostringstream ss;
    ss << "(" << x << "/" << y << ")";
    return ss.str();
}

/*
 1.7 sign determines the relationship of the three points a, b and c.
 If the result is
 * positive, then the point a is left of (b c)
 * zero, then the points are collinear
 * negative, then the point a is right of (b c)
 
*/
double Pt::sign(const Pt& a, const Pt& b, const Pt& c) {

         return ((a.x-c.x)*(b.y-c.y)-(a.y-c.y)*(b.x-c.x));
}

/*
 1.8 insideTriangle checks, if the point x is inside the triangle (a b c).
 This is exactly the case, if the point x is on the same side of all three
 triangle-segments (a b), (b c) and (c a).
 * left,  when the triangle (a b c) is counterclockwise oriented)
 * right, when the triangle (a b c) is clockwise oriented)
 
*/
bool Pt::insideTriangle(const Pt& a, const Pt& b, const Pt& c, const Pt& x) {
    bool b1, b2, b3;
    
    b1 = Pt::sign(x, a, b) < 0;
    b2 = Pt::sign(x, b, c) < 0;
    b3 = Pt::sign(x, c, a) < 0;
    
    return ((b1 == b2) && (b2 == b3));
}

/*
 1.9 distance calculates the distance between this point and ~p~ using
 pythagoras' formula.
 
*/
double Pt::distance(Pt p) {
    return sqrt((p.x-x)*(p.x-x)+(p.y-y)*(p.y-y));
}