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

#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "PointVector.h"

namespace mregionops {


class Segment3D {

public:

    inline Segment3D() {
        
    }

    inline Segment3D(const Point3D& _start, const Point3D& _end) :
        start(_start), end(_end) {

    }

    inline const Point3D& GetStart() const {

        return start;
    }

    inline const Point3D& GetEnd() const {

        return end;
    }
    
    inline const bool IsOrthogonalToTAxis() const {
        
        return NumericUtil::NearlyEqual(start.GetT(), end.GetT());
    }
    
    inline double Length() const {

        return (end - start).Length();
    }

    inline double Length2() const {

        return (end - start).Length2();
    }

private:

    Point3D start, end;
};



class Segment2D {

public:

    inline Segment2D() {
        
    }

    inline Segment2D(const Point2D& p1, 
                     const Point2D& p2,
                     const bool sort = false) {

        if (sort && p2 < p1) {
            
            start = p2;
            end = p1;
            
        } else { // !sort || p1 <= p2
            
            start = p1;
            end = p2;
        }
    }
    
    inline Segment2D(const Segment3D& s) {
        
        start = Point2D(s.GetStart());
        end = Point2D(s.GetEnd());
    }
                         
    inline const Point2D& GetStart() const {

        return start;
    }

    inline const Point2D& GetEnd() const {

        return end;
    }

    inline bool IsParallel(const Segment2D& s) const {

        Vector2D v1 = end - start;
        Vector2D v2 = s.end - s.start;

        v1.Normalize();
        v2.Normalize();
        const double d = v1 | v2;

        return NumericUtil::NearlyEqual(d, 0.0);
    }
    
    inline bool IsColinear(const Segment2D& s) const {
        
        return start.IsColinear(s) && end.IsColinear(s);
    }
    
    inline bool IsVertical() const {

        return NumericUtil::NearlyEqual(start.GetX(), end.GetX());
    }
    
    inline bool IsHorizontal() const {

        return NumericUtil::NearlyEqual(start.GetY(), end.GetY());
    }
    
    inline double Length() const {

        return (end - start).Length();
    }

    inline double Length2() const {

        return (end - start).Length2();
    }
    
    inline void Flip() {
        
        Point2D temp = start;
        
        start = end;
        end = temp;
    }

private:

    Point2D start, end;
};

ostream& operator <<(ostream& o, const Segment2D& s);
ostream& operator <<(ostream& o, const Segment3D& s);

} // end of namespace mregionops

#endif /*SEGMENT_H_*/
