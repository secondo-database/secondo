/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] 

April 2008, initial version created by M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

*/


#ifndef POINTVECTOR_H_
#define POINTVECTOR_H_

#include <math.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "NumericUtil.h"
//#include "Segment.h"

using namespace std;

namespace mregionops {

#define VRML_SCALE_FACTOR 0.3
#define VRML_DOUBLE_PRECISION 16

class Point2D;
class Point3D;
class Vector2D;
class Vector3D;
class Segment2D;

class Vector3D {

public:

    inline Vector3D() :
        x(0.0), y(0.0), z(0.0) {

    }

    inline Vector3D(double _x, double _y, double _z) :
        x(_x), y(_y), z(_z) {

    }

    Vector3D(const Vector2D& v);

    ~Vector3D() {

    }

    inline double GetX() const {

        return x;
    }

    inline double GetY() const {

        return y;
    }

    inline double GetZ() const {

        return z;
    }

    inline double GetT() const {

        return z;
    }

    inline double Length() const {

        return sqrt(x*x + y*y + z*z);
    }

    inline double Length2() const {

        return x*x + y*y + z*z;
    }
    
    inline bool IsZero() const {
        
        return NumericUtil::NearlyEqual(x, 0.0) && 
               NumericUtil::NearlyEqual(y, 0.0) && 
               NumericUtil::NearlyEqual(z, 0.0);
    }

    inline Vector3D operator -() const {

        return Vector3D(-x, -y, -z);
    }

    inline friend Vector3D operator *(const double c, const Vector3D& w) {

        Vector3D v;
        v.x = c * w.x;
        v.y = c * w.y;
        v.z = c * w.z;
        return v;
    }

    inline friend Vector3D operator *(const Vector3D& w, const double c) {

        Vector3D v;
        v.x = c * w.x;
        v.y = c * w.y;
        v.z = c * w.z;
        return v;
    }

    inline friend Vector3D operator /(const Vector3D& w, const double c) {

        Vector3D v;
        v.x = w.x / c;
        v.y = w.y / c;
        v.z = w.z / c;
        return v;
    }

    inline Vector3D operator +(const Vector3D& w) const {

        Vector3D v;
        v.x = x + w.x;
        v.y = y + w.y;
        v.z = z + w.z;
        return v;
    }

    inline Vector3D operator -(const Vector3D& w)const  {

        Vector3D v;
        v.x = x - w.x;
        v.y = y - w.y;
        v.z = z - w.z;
        return v;
    }

    // Inner Dot Product
    inline double operator *(const Vector3D& w) const {

        return (x * w.x + y * w.y + z * w.z);
    }

    // 3D Exterior Cross Product
    inline Vector3D operator ^(const Vector3D& w) const {

        Vector3D v;
        v.x = y * w.z - z * w.y;
        v.y = z * w.x - x * w.z;
        v.z = x * w.y - y * w.x;
        return v;
    }

    inline void Normalize() {

        const double len = sqrt(x*x + y*y + z*z);

        if (len != 0.0) {

            x /= len;
            y /= len;
            z /= len;
        }
    }
    
    inline bool operator ==(const Vector3D& p) const {

        return NumericUtil::NearlyEqual(x, p.x) && NumericUtil::NearlyEqual(y,
                p.y) && NumericUtil::NearlyEqual(z, p.z);
    }

private:

    double x;
    double y;
    double z;
};

class Vector2D {

public:

    inline Vector2D() :
        x(0.0), y(0.0) {

    }

    inline Vector2D(double _x, double _y) :
        x(_x), y(_y) {

    }

    Vector2D(const Vector3D& v);

    ~Vector2D() {

    }

    inline double GetX() const {

        return x;
    }

    inline double GetW() const {

        return x;
    }

    inline double GetY() const {

        return y;
    }

    inline double GetT() const {

        return y;
    }

    inline double Length() const {

        return sqrt(x*x + y*y);
    }

    inline double Length2() const {

        return x*x + y*y;
    }
    
    inline bool IsZero() const {

        return NumericUtil::NearlyEqual(x, 0.0) && 
               NumericUtil::NearlyEqual(y, 0.0);
    }

    inline Vector2D operator -() const {

        return Vector2D(-x, -y);
    }

    // Scalar Multiplies
    inline friend Vector2D operator *(const double c, const Vector2D& w) {

        Vector2D v;
        v.x = c * w.x;
        v.y = c * w.y;
        return v;
    }

    inline friend Vector2D operator *(const Vector2D& w, const double c) {

        Vector2D v;
        v.x = c * w.x;
        v.y = c * w.y;
        return v;
    }

    // Scalar Divides
    inline friend Vector2D operator /(const Vector2D& w, const double c) {

        Vector2D v;
        v.x = w.x / c;
        v.y = w.y / c;
        return v;
    }

    inline Vector2D operator +(const Vector2D& w) const {

        Vector2D v;
        v.x = x + w.x;
        v.y = y + w.y;
        return v;
    }

    inline Vector2D operator -(const Vector2D& w) const {

        Vector2D v;
        v.x = x - w.x;
        v.y = y - w.y;
        return v;
    }

    // Inner Dot Product
    inline double operator *(const Vector2D& w) const {

        return (x * w.x + y * w.y);
    }

    // 2D Exterior Perp Product
    inline double operator |(const Vector2D& w) const {

        return (x * w.y - y * w.x);
    }

    // 3D Exterior Cross Product
    inline Vector3D operator ^(const Vector2D& w) const {

        return Vector3D(0.0, 0.0, x * w.y - y * w.x);
    }

    inline void Normalize() {

        const double len = sqrt(x*x + y*y);

        if (len != 0.0) {

            x /= len;
            y /= len;
        }
    }
    
    inline bool operator ==(const Vector2D& p) const {

        return NumericUtil::NearlyEqual(x, p.x) && NumericUtil::NearlyEqual(y,
                p.y);
    }

private:

    double x;
    double y;
};

class Point2D {

public:

    inline Point2D() :
        x(0.0), y(0.0) {

    }

    inline Point2D(double _x, double _y) :
        x(_x), y(_y) {

    }

    Point2D(const Point3D& p);

    ~Point2D() {

    }

    inline double GetX() const {

        return x;
    }

    inline double GetW() const {

        return x;
    }

    inline double GetY() const {

        return y;
    }

    inline double GetT() const {

        return y;
    }
    
    inline bool operator ==(const Point2D& p) const {
        
        return NumericUtil::NearlyEqual(x, p.x) &&
               NumericUtil::NearlyEqual(y, p.y);
    }
    
    inline bool operator !=(const Point2D& p) const {
        
        return !(*this == p);
    }
    
    inline bool operator <(const Point2D& p) const {

        if (NumericUtil::Lower(x, p.x))
            return true;

        if (NumericUtil::Greater(x, p.x))
            return false;
            
        return NumericUtil::Lower(y, p.y);
    }

    inline Vector2D operator -(const Point2D& p) const {

        return Vector2D(x - p.x, y - p.y);
    }

    inline Point2D operator +(const Vector2D& v) const // +ve translation
    {
        Point2D p;
        p.x = x + v.GetX();
        p.y = y + v.GetY();
        return p;
    }

    inline Point2D operator -(const Vector2D& v) const // -ve translation
    {
        Point2D p;
        p.x = x - v.GetX();
        p.y = y - v.GetY();
        return p;
    }

    inline Point2D operator +(const Point2D& p)const  // affine sum
    {
        Point2D sum;
        sum.x = x + p.x;
        sum.y = y + p.y;
        return sum;
    }

    inline Point2D operator *(const double& f) const {

        Point2D res;
        res.x = x * f;
        res.y = y * f;
        return res;
    }
    
    inline double Distance(const Point2D& p) const {

        return (p - *this).Length();
    }

    inline double Distance2(const Point2D& p) const {

        return (p - *this).Length2();
    }

    inline double WhichSide(const Point2D& start, const Point2D& end) const {

        //return (start.x - x) * (end.y - y) - (end.x - x) * (start.y - y);
        
        Vector2D v1 = end - start;
        Vector2D v2 = *this - start;
        
        v1.Normalize();
        v2.Normalize();
        
        return v1 | v2;
    }
    
    double WhichSide(const Segment2D& s) const;

    inline bool IsLeft(const Point2D& start, const Point2D& end) const {

        return NumericUtil::Greater(WhichSide(start, end), 0.0);
    }
    
    inline bool IsRight(const Point2D& start, const Point2D& end) const {

        return NumericUtil::Lower(WhichSide(start, end), 0.0);
    }
    
    inline bool IsColinear(const Point2D& start, const Point2D& end) const {

        return NumericUtil::NearlyEqual(WhichSide(start, end), 0.0);
    }
    
    bool IsLeft(const Segment2D& s) const;

    bool IsRight(const Segment2D& s) const;

    bool IsColinear(const Segment2D& s) const;

private:

    double x;
    double y;
};

class Point3D {

public:

    inline Point3D() :
        x(0.0), y(0.0), z(0.0) {

    }

    inline Point3D(double _x, double _y, double _z) :
        x(_x), y(_y), z(_z) {

    }

    Point3D(const Point2D& p);

    ~Point3D() {

    }

    inline double GetX() const {

        return x;
    }

    inline double GetY() const {

        return y;
    }

    inline double GetZ() const {

        return z;
    }

    inline double GetT() const {

        return z;
    }

    inline string GetVRMLDesc() const {

        std::ostringstream oss;

        oss << std::setprecision(VRML_DOUBLE_PRECISION) << std::fixed << x
                << " " << y << " " << z << ", ";

        return oss.str();
    }
    
    inline bool operator ==(const Point3D& p) const {

        return NumericUtil::NearlyEqual(x, p.x) && 
               NumericUtil::NearlyEqual(y, p.y) && 
               NumericUtil::NearlyEqual(z, p.z);
    }
    
    inline bool operator !=(const Point3D& p) const {

        return !(*this == p);
    }

    inline Vector3D operator -(const Point3D& p) const {

        return Vector3D(x - p.x, y - p.y, z - p.z);
    }

    inline Point3D operator +(const Vector3D& v) const // +ve translation
    {
        Point3D p;
        p.x = x + v.GetX();
        p.y = y + v.GetY();
        p.z = z + v.GetZ();
        return p;
    }

    inline Point3D operator -(const Vector3D& v) const // -ve translation
    {
        Point3D p;
        p.x = x - v.GetX();
        p.y = y - v.GetY();
        p.z = z - v.GetZ();
        return p;
    }

    inline Point3D operator +(const Point3D& p) const // affine sum
    {
        Point3D sum;
        sum.x = x + p.x;
        sum.y = y + p.y;
        sum.z = z + p.z;
        return sum;
    }

    inline Point3D operator *(const double& f) const {

        Point3D res;
        res.x = x * f;
        res.y = y * f;
        res.z = z * f;
        return res;
    }
    
    inline double Distance(const Point3D& p) const {
        
        return (p - *this).Length();
    }
    
    inline double Distance2(const Point3D& p) const {

        return (p - *this).Length2();
    }
    
    double DistanceToPlane(const Point3D& p0, const Vector3D& normal) const {
 
        const double sb = (- (normal * (*this - p0))) / normal.Length2();
        const Point3D base = *this + (sb * normal);
        
        return Distance(base);
    }
    
    double DistanceToPlane2(const Point3D& p0, const Vector3D& normal) const {

        const double sb = (- (normal * (*this - p0))) / normal.Length2();
        const Point3D base = *this + (sb * normal);

        return Distance2(base);
    }
    
    

private:

    double x;
    double y;
    double z;
};

ostream& operator <<(ostream& o, const Point2D& p);
ostream& operator <<(ostream& o, const Point3D& p);
ostream& operator <<(ostream& o, const Vector2D& p);
ostream& operator <<(ostream& o, const Vector3D& p);

} // end of namespace mregionops





#endif // POINTVECTOR_H_
