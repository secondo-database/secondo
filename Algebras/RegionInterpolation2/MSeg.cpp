/* 
   1 MSeg represents a Moving Segment determined by the endpoints of the
   initial and the final segment. It also maintains two lists of merge-points on
   the initial and final segment in the case that two collinear moving segments
   were merged to one.

*/

#include "interpolate.h"

MSeg::MSeg()  {
}

/*
  1.1 Constructs a MSeg from the endpoints of the initial and final segment.
 
 
*/
MSeg::MSeg(Pt is, Pt ie, Pt fs, Pt fe) : is(is), ie(ie), fs(fs), fe(fe)
 {
    // The endpoints are members of the initial-pointlist (ip) and the
    // final-pointlist (fp)
    ip.push_back(is);
    ip.push_back(ie);
    fp.push_back(fs);
    fp.push_back(fe);
}

/*
  1.2 Converts this MSeg to a Secondo MSegmentData-object with the given
  face-, cycle- and segment-number.
 
*/
MSegmentData MSeg::ToMSegmentData(int face, int cycle, int segno) {
    MSegmentData m(face, cycle, segno, false,
            is.x, is.y, ie.x, ie.y, fs.x, fs.y, fe.x, fe.y);
    return m;
}

/*
 1.3 ==
 Two MSeg-objects equal if the initial and final segments endpoints match.

*/
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

#define USE_SPECIALTRAPEZIUMINTERSECTS

/*
 1.4 
 
*/
bool MSeg::intersects(const MSeg& a, bool checkSegs) const {
    int ret;
#ifndef USE_SPECIALTRAPEZIUMINTERSECTS

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
        V0[0] = is.x;
        V0[1] = is.y;
        V0[2] = 0;
        V1[0] = fs.x;
        V1[1] = fs.y;
        V1[2] = 1;
        V2[0] = fe.x;
        V2[1] = fe.y;
        V2[2] = 1;
    } else if (fs.x == fe.x && fs.y == fe.y) {
        V0[0] = is.x;
        V0[1] = is.y;
        V0[2] = 0;
        V1[0] = ie.x;
        V1[1] = ie.y;
        V1[2] = 0;
        V2[0] = fs.x;
        V2[1] = fs.y;
        V2[2] = 1;
    } else {
        cerr << "ERROR: src-triangle is a trapezium!\n";
    }

    if (a.is.x == a.ie.x && a.is.y == a.ie.y) {
        U0[0] = a.is.x;
        U0[1] = a.is.y;
        U0[2] = 0;
        U1[0] = a.fs.x;
        U1[1] = a.fs.y;
        U1[2] = 1;
        U2[0] = a.fe.x;
        U2[1] = a.fe.y;
        U2[2] = 1;
    } else if (a.fs.x == a.fe.x && a.fs.y == a.fe.y) {
        U0[0] = a.is.x;
        U0[1] = a.is.y;
        U0[2] = 0;
        U1[0] = a.ie.x;
        U1[1] = a.ie.y;
        U1[2] = 0;
        U2[0] = a.fs.x;
        U2[1] = a.fs.y;
        U2[2] = 1;
    } else {
        cerr << "ERROR: dst-triangle is a trapezium!\n";
    }

    ret = TriangleIntersection(V0, V1, V2, U0, U1, U2);
#else
    
    unsigned int detailedResult;

    ret = specialTrapeziumIntersects(
            100,
            is.x, is.y,
            ie.x, ie.y,
            fe.x, fe.y,
            fs.x, fs.y,

            a.is.x, a.is.y,
            a.ie.x, a.ie.y,
            a.fe.x, a.fe.y,
            a.fs.x, a.fs.y,
            detailedResult
            );
    if (ret) {
        cerr << "Intersection between " << ToString()
                << " and " << a.ToString() << "\n";
    } else {
        //        ret = specialTrapeziumIntersects(
        //                1,
        //                a.is.x, a.is.y,
        //                a.ie.x, a.ie.y,
        //                a.fe.x, a.fe.y,
        //                a.fs.x, a.fs.y,
        //
        //                is.x, is.y,
        //                ie.x, ie.y,
        //                fe.x, fe.y,
        //                fs.x, fs.y,
        //
        //                detailedResult
        //                );
        if (ret) {
            cerr << "Asymmetric Intersection between " << a.ToString()
                    << " and " << ToString() << "\n";

        }
    }

    if (checkSegs) {
        Seg ai(a.is, a.ie), af(a.fs, a.fe);
        Seg bi(is, ie), bf(fs, fe);
        if (ai.intersects(bi)) {
            ret = true;
            cerr << "Intersection initial!\n" << ai.ToString() << " "
                    << bi.ToString() << "\n";
        }
        if (af.intersects(bf)) {
            ret = true;
            cerr << "Intersection final!\n" << af.ToString() << " "
                    << bf.ToString() << "\n";
        }
    }


#endif
    return ret;

}

bool MSeg::operator<(const MSeg & a) const {
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

    std::reverse(ip.begin(), ip.end());
    std::reverse(fp.begin(), fp.end());
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

bool MSeg::Merge(const MSeg& m) {
    Seg s1 = (is == ie) ? Seg(fs, fe) : Seg(is, ie);
    Seg s2 = (m.is == m.ie) ? Seg(m.fs, m.fe) : Seg(m.is, m.ie);

    if (s1.angle() != s2.angle())
        return false;

    if ((ie == m.is) && (fe == m.fs)) {
        ie = m.ie;
        fe = m.fe;
        ip.push_back(ie);
        fp.push_back(fe);
    } else if ((m.ie == is) && (m.fe == fs)) {
        is = m.is;
        fs = m.fs;
        ip.insert(ip.begin(), is);
        fp.insert(fp.begin(), fs);
    } else {
        return false;
    }

    return true;
}

bool MSeg::Integrate(MSeg& n, MSeg& m1, MSeg& m2) {
    std::vector<Pt>::iterator i1, i2;
    i1 = ip.begin();

    while (i1 != ip.end()) {
        if (*i1 == n.is)
            break;
        i1++;
    }
    i2 = i1;

    while (i2 != ip.end()) {
        if (*i2 == n.ie)
            break;
        i2++;
    }

    std::vector<Pt>::iterator f1, f2;
    f1 = fp.begin();
    
    while (f1 != fp.end()) {
        if (*f1 == n.fs)
            break;
        f1++;
    }
    f2 = f1;

    while (f2 != fp.end()) {
        if (*f2 == n.fe)
            break;
        f2++;
    }

    if ((i2 == ip.end()) || (f2 == fp.end()))
        return false;

    m1 = MSeg(is, *i1, fs, *f1);
    m1.ip = vector<Pt>(ip.begin(), i1);
    m1.fp = vector<Pt>(fp.begin(), f1);
    m2 = MSeg(*i2, ie, *f2, fe);
    m2.ip = vector<Pt>(i2, ip.end());
    m2.fp = vector<Pt>(f2, fp.end());

    return true;
}
