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

bool MSeg::intersects(const MSeg& a) const {
    unsigned int detailedResult;
    bool ret;
    
    ret = specialTrapeziumIntersects(
            1,
            is.x, is.y,
            ie.x, ie.y,
            fs.x, fs.y,
            fe.x, fe.y,

            a.is.x, a.is.y,
            a.ie.x, a.ie.y,
            a.fs.x, a.fs.y,
            a.fe.x, a.fe.y,
            detailedResult
            );
    
    if (ret) {
        cerr << "Intersection found, detail: " << detailedResult << "\n";
        cerr << ToString() << "\n" << a.ToString() << "\n";
        
        if (detailedResult != 8)
            return true;
    }
        
    return false;
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