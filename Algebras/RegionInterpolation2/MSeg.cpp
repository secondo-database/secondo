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


MSegs::MSegs() {
}

string MSeg::ToString() const {
    std::ostringstream ss;

    ss << "((" << sx1 << " " << sy1 << " " << sx2 << " " << sy2 << ")("
               << fx1 << " " << fy1 << " " << fx2 << " " << fy2 << "))";

    return ss.str();
}

void MSegs::AddMSeg (int sx1, int sy1, int sx2, int sy2,
                     int fx1, int fy1, int fx2, int fy2) {
    MSeg m(sx1, sy1, sx2, sy2, fx1, fy1, fx2, fy2);
    segs.push_back(m);
}

void MSegs::AddMSegs (vector<MSeg> v) {
    for (unsigned int i = 0; i < v.size(); i++) {
        segs.push_back(v[i]);
    }
}

vector<MSeg> MSegs::GetMatchingMSegs (MSegs m) {
    vector<MSeg> ret;
    std::sort(segs.begin(), segs.end());
    std::sort(m.segs.begin(), m.segs.end());
    
    unsigned int i = 0, j = 0;
    while (i < segs.size() && j < m.segs.size()) {
        if (segs[i] == m.segs[j]) {
            ret.push_back(segs[i]);
            i++;
            j++;
        } else if (segs[i] < m.segs[j]) {
            i++;
        } else {
            j++;
        }
    }
    
    return ret;
}

void MSegs::MergeConcavity (MSegs c) {
    std::sort(segs.begin(), segs.end());
    std::sort(c.segs.begin(), c.segs.end());
    std::vector<MSeg>::iterator i = segs.begin();
    std::vector<MSeg>::iterator j = c.segs.begin();
    while (i != segs.end() && j != c.segs.end()) {
        cerr << "Comparing " << i->ToString() << " = " << j->ToString() << "\n";
        if (*i == *j) {
            cerr << "Erasing: " << i->ToString() << "\n";
            i = segs.erase(i);
            j = c.segs.erase(j);
        } else if (*i < *j) {
            i++;
        } else {
            j++;
        }
    }
    segs.insert(segs.end(),c.segs.begin(),c.segs.end());
}

string MSegs::ToString() const {
    std::ostringstream ss;

    ss << "MSegs (\n";
    for (unsigned int i = 0; i < segs.size(); i++) {
        ss << " " << segs[i].ToString() << "\n";
    }
    ss << ")\n";
    return ss.str();
}

vector<MSegmentData> MSegs::ToMSegmentData(int face, int cycle) {
    vector<MSegmentData> ret;
    int segno = 0;
    
    for (unsigned int i = 0; i < segs.size(); i++) {
        MSegmentData ms(face, cycle, segno++, false,
                segs[i].sx1,
                segs[i].sy1,
                segs[i].sx2,
                segs[i].sy2,
                segs[i].fx1,
                segs[i].fy1,
                segs[i].fx2,
                segs[i].fy2
                );
        ret.push_back(ms);
    }
    
    return ret;
}

// Inefficient!
bool MSegs::intersects (const MSegs& a) const {
    for (unsigned int i = 0; i < a.segs.size(); i++) {
        for (unsigned int j = 0; j < segs.size(); j++) {
            if (segs[j].intersects(a.segs[i])) {
                return true;
            }
        }
    }
    
    return false;
}


MFace::MFace() {
}

MFace::MFace(MSegs face) : face(face) {
}

void MFace::AddHole(MSegs m) {
    vector<MSeg> match = m.GetMatchingMSegs(face);
    if (match.size() > 0) {
        cerr << "Matching: \n";
        for (unsigned int i = 0; i < match.size(); i++) {
            cerr << match[i].ToString() << "\n";
        }
        face.MergeConcavity(m);
        cerr << "Now is: \n";
        cerr << face.ToString() << "\n";
    } else {
        holes.push_back(m);
    }
}

URegion MFace::ToURegion(Interval<Instant> iv) {
    vector<MSegmentData> ms = face.ToMSegmentData(0, 0);
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(0, i+1);
        ms.insert(ms.end(), h.begin(), h.end());
    }
    URegion ret(ms, iv);
    
    return ret;
}

MFaces::MFaces() {
}

MFaces::MFaces(MFace face) {
    AddFace(face);
}

void MFaces::AddFace(MFace face) {
    faces.push_back(face);
}

MRegion MFaces::ToMRegion(Interval<Instant> iv) {
    MRegion ret;
    
    if (faces.size() > 0) {
        URegion u = faces[0].ToURegion(iv);
        for (unsigned int i = 0; i < faces.size(); i++) {
            URegion u2 = faces[i].ToURegion(iv);
            u.AddURegion(&u2);
        }
        ret.AddURegion(u);
    }
    
    return ret;
}
