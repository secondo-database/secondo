/*
*/

#include "interpolate.h"

MSeg::MSeg() {
}

MSeg::MSeg(Pt is, Pt ie, Pt fs, Pt fe) : is(is), ie(ie), fs(fs), fe(fe) {
}

MSegmentData MSeg::ToMSegmentData(int face, int cycle, int segno) {
    MSegmentData m(face, cycle, segno, false,
            is.x, is.y, ie.x, ie.y, fs.x, fs.y, fe.x, fe.y);
    return m;
}

bool MSeg::operator==(const MSeg& a) const {
    return (
            is == a.is &&
            ie == a.ie &&
            fs == a.fs &&
            fe == a.fe
            );
}

int TriangleIntersection(float V0[3], float V1[3], float V2[3],
        float U0[3], float U1[3], float U2[3]);

bool MSeg::intersects(const MSeg& a) const {
    int ret;
    
    if (!(is == ie) && !(fs == fe)) {
        MSeg ms1(is, ie, fs, fs);
        MSeg ms2(is, is, fs, fe);
        return ms1.intersects(a) || ms2.intersects(a);
    }
    
    if (!(a.is == a.ie) && !(a.fs == a.fe)) {
        MSeg ms1(a.is, a.ie, a.fs, a.fs);
        MSeg ms2(a.is, a.is, a.fs, a.fe);
        return intersects(ms1) || intersects(ms2);
    }
    
    float V0[3], V1[3], V2[3];
    float U0[3], U1[3], U2[3];
    
    if (is.x == ie.x && is.y == ie.y) {
        V0[0] = is.x; V0[1] = is.y; V0[2] = 0;
        V1[0] = fs.x; V1[1] = fs.y; V1[2] = 1;
        V2[0] = fe.x; V2[1] = fe.y; V2[2] = 1;
    } else if (fs.x == fe.x && fs.y == fe.y) {
        V0[0] = is.x; V0[1] = is.y; V0[2] = 0;
        V1[0] = ie.x; V1[1] = ie.y; V1[2] = 0;
        V2[0] = fs.x; V2[1] = fs.y; V2[2] = 1;
    } else {
        cerr << "ERROR: src-triangle is a trapezium!\n";
    }
    
    if (a.is.x == a.ie.x && a.is.y == a.ie.y) {
        U0[0] = a.is.x; U0[1] = a.is.y; U0[2] = 0;
        U1[0] = a.fs.x; U1[1] = a.fs.y; U1[2] = 1;
        U2[0] = a.fe.x; U2[1] = a.fe.y; U2[2] = 1;
    } else if (a.fs.x == a.fe.x && a.fs.y == a.fe.y) {
        U0[0] = a.is.x; U0[1] = a.is.y; U0[2] = 0;
        U1[0] = a.ie.x; U1[1] = a.ie.y; U1[2] = 0;
        U2[0] = a.fs.x; U2[1] = a.fs.y; U2[2] = 1;
    } else {
        cerr << "ERROR: dst-triangle is a trapezium!\n";
    }
    
    ret = TriangleIntersection(V0, V1, V2, U0, U1, U2);

    if (ret) {
        cerr << "Intersection between " << ToString()
                << " and " << a.ToString() << "\n";
    }
//    else {
//        cerr << "No Intersection between " << ToString()
//                << " and " << a.ToString() << "\n";
//    }
    
    return ret;

    //    unsigned int detailedResult;
    //    return specialTrapeziumIntersects(
    //            1,
    //            sx1, sy1,
    //            sx2, sy2,
    //            fx2, fy2,
    //            fx1, fy1,
    //            
    //            a.sx1, a.sy1,
    //            a.sx2, a.sy2,
    //            a.fy1, a.fy2,
    //            a.fx1, a.fx2,
    //            detailedResult
    //            );
}

bool MSeg::operator<(const MSeg& a) const {
    if (is.x < a.is.x) {
        return true;
    } else if (is.x > a.is.x) {
        return false;
    }
    if (is.y < a.is.y) {
        return true;
    } else if (is.y > a.is.y) {
        return false;
    }
    if (ie.x < a.ie.x) {
        return true;
    } else if (ie.x > a.ie.x) {
        return false;
    }
    if (ie.y < a.ie.y) {
        return true;
    } else if (ie.y > a.ie.y) {
        return false;
    }
    if (fs.x < a.fs.x) {
        return true;
    } else if (fs.x > a.fs.x) {
        return false;
    }
    if (fs.y < a.fs.y) {
        return true;
    } else if (fs.y > a.fs.y) {
        return false;
    }
    if (fe.x < a.fe.x) {
        return true;
    } else if (fe.x > a.fe.x) {
        return false;
    }
    if (fe.y < a.fe.y) {
        return true;
    } else if (fe.y > a.fe.y) {
        return false;
    }

    return false;
}

void MSeg::ChangeDirection() {
    Pt tmp;

    tmp = is;
    is = ie;
    ie = tmp;

    tmp = fs;
    fs = fe;
    fe = tmp;
}

string MSeg::ToString() const {
    std::ostringstream ss;

    ss << "((" << is.ToString() << " " << ie.ToString() << ")("
            << fs.ToString() << " " << fe.ToString() << "))";

    return ss.str();
}

MSeg MSeg::divide(double start, double end) {
    MSeg ret;

    ret.is = Pt(is.x + (fs.x - is.x) * start, is.y + (fs.y - is.y) * start);
    ret.ie = Pt(ie.x + (fe.x - ie.x) * start, ie.y + (fe.y - ie.y) * start);
    ret.fs = Pt(is.x + (fs.x - is.x) * end, is.y + (fs.y - is.y) * end);
    ret.fe = Pt(ie.x + (fe.x - ie.x) * end, ie.y + (fe.y - ie.y) * end);

    return ret;
}
