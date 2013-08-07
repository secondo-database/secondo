/*
*/

#include "interpolate.h"

MSeg::MSeg() {
}

MSeg::MSeg(int sx1,int sy1,int sx2,int sy2,int fx1,int fy1,int fx2,int fy2)
     : sx1(sx1),sy1(sy1),sx2(sx2),sy2(sy2),fx1(fx1),fy1(fy1),fx2(fx2),fy2(fy2) {
}

MSegmentData MSeg::ToMSegmentData(int face, int cycle, int segno) {
    MSegmentData m(face, cycle, segno, false, sx1,sy1,sx2,sy2,fx1,fy1,fx2,fy2);
    
    return m;
}

bool MSeg::operator== (const MSeg& a) const {
    return (
            sx1 == a.sx1 &&
            sy1 == a.sy1 &&
            sx2 == a.sx2 &&
            sy2 == a.sy2 &&
            fx1 == a.fx1 &&
            fy1 == a.fy1 &&
            fx2 == a.fx2 &&
            fy2 == a.fy2
            );
}

bool MSeg::intersects (const MSeg& a) const {
    unsigned int detailedResult;
    
    return specialTrapeziumIntersects(
            1,
            sx1, sy1,
            sx2, sy2,
            fx2, fy2,
            fx1, fy1,
            
            a.sx1, a.sy1,
            a.sx2, a.sy2,
            a.fy1, a.fy2,
            a.fx1, a.fx2,
            detailedResult
            );
}

bool MSeg::operator< (const MSeg& a) const {
    if (sx1 < a.sx1) {
        return true;
    } else if (sx1 > a.sx1) {
        return false;
    }
    if (sy1 < a.sy1) {
        return true;
    } else if (sy1 > a.sy1) {
        return false;
    }
    if (sx2 < a.sx2) {
        return true;
    } else if (sx2 > a.sx2) {
        return false;
    }
    if (sy2 < a.sy2) {
        return true;
    } else if (sy2 > a.sy2) {
        return false;
    }
    if (fx1 < a.fx1) {
        return true;
    } else if (fx1 > a.fx1) {
        return false;
    }
    if (fy1 < a.fy1) {
        return true;
    } else if (fy1 > a.fy1) {
        return false;
    }
    if (fx2 < a.fx2) {
        return true;
    } else if (fx2 > a.fx2) {
        return false;
    }
    if (fy2 < a.fy2) {
        return true;
    } else if (fy2 > a.fy2) {
        return false;
    }
    
    return false;
}

string MSeg::ToString() const {
    std::ostringstream ss;

    ss << "((" << sx1 << " " << sy1 << " " << sx2 << " " << sy2 << ")("
               << fx1 << " " << fy1 << " " << fx2 << " " << fy2 << "))";

    return ss.str();
}