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
    if (!(is == ie))
        ip.push_back(ie);
    fp.push_back(fs);
    if (!(fs == fe))
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
 1.3 operator equals
 Two MSeg-objects are equal exactly if the initial and final segments'
 endpoints match.

*/
bool MSeg::operator==(const MSeg& a) const {
    return (
            is == a.is &&
            ie == a.ie &&
            fs == a.fs &&
            fe == a.fe
            );
}

/*
 1.4 less-operator is mainly used to deterministically sort a list of MSeg-
 objects to compare with another list or to find duplicates.
 
*/
bool MSeg::operator<(const MSeg & a) const {
    if (is.x < a.is.x) {
        return true;
    } else if (is.x > a.is.x) {
        return false;
    } else if (is.y < a.is.y) {
        return true;
    } else if (is.y > a.is.y) {
        return false;
    } else if (ie.x < a.ie.x) {
        return true;
    } else if (ie.x > a.ie.x) {
        return false;
    } else if (ie.y < a.ie.y) {
        return true;
    } else if (ie.y > a.ie.y) {
        return false;
    } else if (fs.x < a.fs.x) {
        return true;
    } else if (fs.x > a.fs.x) {
        return false;
    } else if (fs.y < a.fs.y) {
        return true;
    } else if (fs.y > a.fs.y) {
        return false;
    } else if (fe.x < a.fe.x) {
        return true;
    } else if (fe.x > a.fe.x) {
        return false;
    } else if (fe.y < a.fe.y) {
        return true;
    } else if (fe.y > a.fe.y) {
        return false;
    } else { // The two objects are identical, so return false
        return false;
    }
}


/*
 1.5 intersects is called to test if two MSeg-objects intersect in 3D. In the
 backend the function specialTrapeziumIntersects from the MovingRegion-Algebra
 is used. If ~checkSegs~ is set, then additional checks are performed if the
 initial or final segments overlap.
 
*/
#define TRAPEZIUMINTERSECTS trapeziumIntersects2
bool MSeg::intersects(const MSeg& a, bool checkSegs) const {
    bool ret;
    unsigned int detailedResult;
    
    bool TRAPEZIUMINTERSECTS(MSeg m, MSeg a, unsigned int &detailedResult);
    ret = TRAPEZIUMINTERSECTS(*this, a, detailedResult);
    
    if (ret)
        DEBUG(3, "Segments intersect: " << this->ToString() <<
                " with " << a.ToString());

    if (checkSegs) {
        Seg ai(a.is, a.ie), af(a.fs, a.fe);
        Seg bi(is, ie), bf(fs, fe);
        if (ai.intersects(bi)) {
            DEBUG(3, "Initial segment intersects: " << ai.ToString());
            ret = true;
        }
        if (af.intersects(bf)) {
            DEBUG(3, "Final segment intersects: " << af.ToString());
            ret = true;
        }
    }
    
    return ret;
}

static int trapeziumIntersects (MSeg m, MSeg a, unsigned int& detailedResult) {
    int ret = specialTrapeziumIntersects(
            100,
            m.is.x, m.is.y,
            m.ie.x, m.ie.y,
            m.fe.x, m.fe.y,
            m.fs.x, m.fs.y,

            a.is.x, a.is.y,
            a.ie.x, a.ie.y,
            a.fe.x, a.fe.y,
            a.fs.x, a.fs.y,
            detailedResult
            );
    
    return ret;
}

static bool trapeziumIntersects3 (MSeg m, MSeg a) {
    double s1 = Pt::sign(m.is, a.is, a.ie);
    double s2 = Pt::sign(m.fs, a.fs, a.fe);
    
    if ((s1 <= 0) && (s2 <= 0) ||
        (s1 >= 0) && (s2 >= 0))
        return false;
        
}

/*
 1.6 ChangeDirection is used to change the orientation of the initial and final
 segments. This is mainly used to integrate one cycle into another cycle.
 
*/
void MSeg::ChangeDirection() {
    Pt tmp;

    tmp = is;
    is = ie;
    ie = tmp;

    tmp = fs;
    fs = fe;
    fe = tmp;

    // Reverse the list of intermediary points on the segments, too.
    std::reverse(ip.begin(), ip.end());
    std::reverse(fp.begin(), fp.end());
}

/*
 1.7 ToString creates a string-representation of this object. This can be used
 for debugging purposes.
 
*/
string MSeg::ToString() const {
    std::ostringstream ss;

    ss << "((" << is.ToString() << " " << ie.ToString() << ")("
            << fs.ToString() << " " << fe.ToString() << "))";

    return ss.str();
}

/*
 1.8 divide restricts the interval of this MSeg to the given limits.
 ~start~ and ~end~ must be a fraction between 0 and 1.
 For example, divide(0, 0.5) would create an MSeg representing the first half
 of the interval.

*/
MSeg MSeg::divide(double start, double end) {
    MSeg ret;

    ret.is = Pt(is.x + (fs.x - is.x) * start, is.y + (fs.y - is.y) * start);
    ret.ie = Pt(ie.x + (fe.x - ie.x) * start, ie.y + (fe.y - ie.y) * start);
    ret.fs = Pt(is.x + (fs.x - is.x) * end, is.y + (fs.y - is.y) * end);
    ret.fe = Pt(ie.x + (fe.x - ie.x) * end, ie.y + (fe.y - ie.y) * end);

    return ret;
}

/*
 1.9 Merge tries to merge another MSeg-object into this one.
 This is only possible if the initial and final segments are both collinear
 (collinearity with a degenerated segment is trivially given) and the segments
 are adjacent. We only check, if it can be merged with the last segment
 inserted since RotatingPlane pushes them in the correct order.
 The object also remembers the original endpoints to be able to split this
 object again, for example to integrate another cycle
 (MergeConcavity uses that)
 
*/
bool MSeg::Merge(const MSeg& m) {
    // The initial and final segments of an MSeg are always collinear or
    // (at most) one side is degenerated. Compare the angles of a
    // not-degenerated segment of each MSeg
    Seg s1 = (is == ie) ? Seg(fs, fe) : Seg(is, ie);
    Seg s2 = (m.is == m.ie) ? Seg(m.fs, m.fe) : Seg(m.is, m.ie);
    if (s1.angle() != s2.angle()) // The angles differ, so we cannot merge
        return false;

    if ((ie == m.is) && (fe == m.fs)) {
        // The MSeg to merge is adjacent to the end of this MSeg, so correct the
        // endpoints of this MSeg
        ie = m.ie;
        fe = m.fe;
        
        // Remember the original points, but don't insert duplicates in case
        // of degenerated segments
        if (!(ip[ip.size()-1] == ie))
            ip.push_back(ie);
        if (!(fp[fp.size()-1] == fe))
            fp.push_back(fe);
    } else if ((m.ie == is) && (m.fe == fs)) {
        // The MSeg to merge is adjacent to the begin of this MSeg, so correct
        // the initial-pointlist of this MSeg
        is = m.is;
        fs = m.fs;
        
        // Remember the original points, but don't insert duplicates in case
        // of degenerated segments
        if (!(ip[0] == is))
            ip.insert(ip.begin(), is);
        if (!(fp[0] == fs))
            fp.insert(fp.begin(), fs);
    } else {
        // The segment is collinear, but not adjacent, so we cannot merge.
        return false;
    }

    return true;
}

/*
 1.10 Split tries to split this MSeg by the given MSeg ~n~ and defines the
 remaining MSeg-objects ~m1~ and ~m2~.
 
 As seen in 1.9 an MSeg may have been merged from several objects, but the
 original endpoints are recorded. This function checks, if the segment ~n~
 can be constructed from the original endpoints and, if this is the case,
 defines the two remaining MSeg-objects ~m1~ and ~m2~, if ~n~ is "cut" out
 of this object (so m1 and m2 are effectively return-values).
 The function returns true if a split was possible.
 
*/
bool MSeg::Split(MSeg& n, MSeg& m1, MSeg& m2) {
    std::vector<Pt>::iterator i1, i2;
    i1 = ip.begin();

    // Search the initial segment's startpoint of n in the list
    while (i1 != ip.end()) {
        if (*i1 == n.is)
            break;
        i1++;
    }
    i2 = i1;

    // Search the initial segment's endpoint of n in the list
    while (i2 != ip.end()) {
        if (*i2 == n.ie)
            break;
        i2++;
    }

    std::vector<Pt>::iterator f1, f2;
    f1 = fp.begin();
    
    // Search the final segment's startpoint of n in the list
    while (f1 != fp.end()) {
        if (*f1 == n.fs)
            break;
        f1++;
    }
    f2 = f1;

    // Search the final segment's endpoint of n in the list
    while (f2 != fp.end()) {
        if (*f2 == n.fe)
            break;
        f2++;
    }

    // Some point was not found, so we cannot split by n
    if ((i2 == ip.end()) || (f2 == fp.end()))
        return false;

    // Otherwise, define the remainders (which may even be degenerated in both
    // instants, the caller has to handle that case)
    m1 = MSeg(is, *i1, fs, *f1);
    m1.ip = vector<Pt>(ip.begin(), i1+1);
    m1.fp = vector<Pt>(fp.begin(), f1+1);
    m2 = MSeg(*i2, ie, *f2, fe);
    m2.ip = vector<Pt>(i2, ip.end());
    m2.fp = vector<Pt>(f2, fp.end());
    
    // Now the following is true: this = m1 + n + m2

    return true;
}
