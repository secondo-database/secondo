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


class Segment2D {

public:

    inline Segment2D() {
        
    }

    inline Segment2D(const Point2D& p1, 
                     const Point2D& p2,
                     const bool sort) {

        if (sort && p2 < p1) {
            
            start = p2;
            end = p1;
            
        } else { // !sort || p1 <= p2
            
            start = p1;
            end = p2;
        }
    }

    inline Point2D GetStart() const {

        return start;
    }

    inline Point2D GetEnd() const {

        return end;
    }

    //bool IsAbove(const Point2D& p) const;
    
    inline bool IsVertical() const {

        return NumericUtil::NearlyEqual(start.GetX(), end.GetX());
    }
    
    inline bool IsHorizontal() const {

        return NumericUtil::NearlyEqual(start.GetY(), end.GetY());
    }

private:

    Point2D start, end;
};



class Segment3D {

public:

    inline Segment3D() {
        
    }

    inline Segment3D(Point3D _start, Point3D _end) :
        start(_start), end(_end) {

        // TODO: Sort Points by t, x, y.
    }

    inline Point3D GetStart() const {

        return start;
    }

    inline Point3D GetEnd() const {

        return end;
    }

private:

    Point3D start, end;
};

} // end of namespace mregionops

#endif /*SEGMENT_H_*/
