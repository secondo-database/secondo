/*
*/

#include "interpolate.h"

MSegs::MSegs() : ignore(0), iscollapsed(0), id(-1) {
}

void MSegs::AddMSeg(MSeg m) {
    if ((m.is == m.ie) && (m.fs == m.fe))
        return;

#if 0
    bool merged = false;
    if (!segs.empty()) {
        MSeg *prev = &segs[segs.size() - 1];
        if ((prev->fs == prev->fe) && (m.is == m.ie) && (prev->ie == m.is)) {
            Seg s1(prev->is, prev->ie);
            Seg s2(m.fs, m.fe);
            if (s1.angle() == s2.angle()) {
                prev->fs = m.fs;
                prev->fe = m.fe;
                merged = true;
            }
        } else if ((prev->is == prev->ie) && (m.fs == m.fe) &&
                (prev->fe == m.fs)) {
            Seg s1(prev->fs, prev->fe);
            Seg s2(m.is, m.ie);
            if (s1.angle() == s2.angle()) {
                prev->is = m.is;
                prev->ie = m.ie;
                merged = true;
            }
        }
    }
    if (!merged)
#endif
        segs.push_back(m);
    calculateBBox();
}

vector<MSeg> MSegs::GetMatchingMSegs(MSegs m) {
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

void MSegs::MergeConcavity(MSegs c) {
#if 1
    std::sort(segs.begin(), segs.end());
    std::sort(c.segs.begin(), c.segs.end());
    std::vector<MSeg>::iterator i = segs.begin();
    std::vector<MSeg>::iterator j = c.segs.begin();
    while (i != segs.end() && j != c.segs.end()) {
        if (*i == *j) {
            i = segs.erase(i);
            j = c.segs.erase(j);
        } else if (*i < *j) {
            i++;
        } else {
            j++;
        }
    }
#else
    // Delete cross-added segments
   std::vector<MSeg>::iterator i = segs.begin();
    while (i != segs.end()) {
        std::vector<MSeg>::iterator j = c.segs.begin();
        while (j != segs.end()) {
            MSeg s = *i, e = *j;
            if (((s.is == e.is) && (s.ie == e.ie)) ||
                ((s.fs == e.fs) && (s.fe == e.fe))) {
                i->valid = false;
                j->valid = false;
            }
        }
    }
    
    for (i = segs.begin(); i != segs.end(); i++) {
        if (!i->valid)
            i = segs.erase(i);
    }
    
    for (i = c.segs.begin(); i != c.segs.end(); i++) {
        if (!i->valid)
            i = c.segs.erase(i);
    }
#endif
    
    for (unsigned int x = 0; x < c.segs.size(); x++)
        c.segs[x].ChangeDirection();
    segs.insert(segs.end(), c.segs.begin(), c.segs.end());
    std::sort(segs.begin(), segs.end());
    //    cerr << "Result: " << ToString() << "\n\n";
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

vector<MSegmentData> MSegs::ToMSegmentData(int face, int cycle, int segno) {
    vector<MSegmentData> ret;
    for (unsigned int i = 0; i < segs.size(); i++) {
        MSegmentData ms(face, cycle, segno++, false,
                segs[i].is.x,
                segs[i].is.y,
                segs[i].ie.x,
                segs[i].ie.y,
                segs[i].fs.x,
                segs[i].fs.y,
                segs[i].fe.x,
                segs[i].fe.y
                );
        ret.push_back(ms);
    }

    return ret;
}

bool MSegs::intersects(const MSegs& a) const {
    
    // Fast path if the boundingboxes are disjoint
    
    if ((bbox.first.x > a.bbox.second.x) ||
        (a.bbox.first.x > bbox.second.x) ||
        (bbox.first.y > a.bbox.second.y) ||
        (a.bbox.first.y > bbox.second.y))
            return false;
    
    
    bool ret = false;
    for (unsigned int i = 0; i < a.segs.size(); i++) {
        for (unsigned int j = 0; j < segs.size(); j++) {
            if (segs[j].intersects(a.segs[i])) {
                ret = true;
                return ret;
            }
        }
    }

    return ret;
}

pair<MSegs, MSegs> MSegs::kill() {
    MSegs src = sreg.collapse(true);
    MSegs dst = dreg.collapse(false);

    return pair<MSegs, MSegs>(src, dst);
}

MSegs MSegs::divide(double start, double end) {
    MSegs ret;

    ret.sreg = sreg;
    ret.dreg = dreg;

    for (unsigned int i = 0; i < segs.size(); i++) {
        MSeg m = segs[i].divide(start, end);
        if (m.is == m.ie && m.fs == m.fe)
            continue;
        ret.AddMSeg(m);
    }

    return ret;
}

int MSegs::getLowerLeft() {
    int idx = 0, x, y;

    x = segs[0].ie.x;
    y = segs[0].ie.y;

    for (unsigned int i = 1; i < segs.size(); i++) {
        if (segs[i].ie.x < x) {
            idx = i;
            x = segs[i].ie.x;
            y = segs[i].ie.y;
        } else if (segs[i].ie.x == x) {
            if (segs[i].ie.y < y) {
                idx = i;
                x = segs[i].ie.x;
                y = segs[i].ie.y;
            }
        }
    }

    return idx;
}

int MSegs::findNext(int index) {
    unsigned int nrsegs = segs.size();
    MSeg *s1 = &segs[index];

    for (unsigned int i = 0; i < nrsegs; i++) {
        int nindex = (i + index) % nrsegs;
        MSeg *s2 = &segs[nindex];
        if (s1->ie == s2->is && s1->fe == s2->fs)
            return nindex;
    }

    return -1;
}

void MSegs::crossAdd(set<Seg> csegssrc, set<Seg> csegsdst) {
    std::set<Seg>::iterator si = csegssrc.begin();
    std::set<Seg>::iterator di;
    
    while (si != csegssrc.end()) {
        di = csegsdst.begin();
        while (di != csegsdst.end()) {
            Seg s = *si, d = *di;
            
            AddMSeg(MSeg(s.s, s.e, d.s, d.s));
            AddMSeg(MSeg(s.s, s.s, d.s, d.e));
            
            AddMSeg(MSeg(s.e, s.e, d.s, d.e));
            AddMSeg(MSeg(s.s, s.e, d.e, d.e));
            
            di++;
        }
        si++;
    }
    
}

static inline double max(double a, double b, double c, double d) {
    double m = a;

    if (b > m)
        m = b;
    if (c > m)
        m = c;
    if (d > m)
        m = d;

    return m;
}

static inline double min(double a, double b, double c, double d) {
    double m = a;

    if (b < m)
        m = b;
    if (c < m)
        m = c;
    if (d < m)
        m = d;

    return m;
}

pair<Pt, Pt> MSegs::calculateBBox() {
    if (segs.empty()) {
        bbox = pair<Pt, Pt>(Pt(0, 0), Pt(0, 0));
    } else {
        bbox.first.valid = bbox.second.valid = 0;
        for (unsigned int i = 0; i < segs.size(); i++) {
            updateBBox(segs[i]);
        }
    }

    return bbox;
}

void MSegs::updateBBox(MSeg& seg) {
    Pt p1(min(seg.is.x, seg.ie.x,
            seg.fs.x, seg.fe.x),
            min(seg.is.y, seg.ie.y,
            seg.fs.y, seg.fe.y)
            );
    Pt p2(max(seg.is.x, seg.ie.x,
            seg.fs.x, seg.fe.x),
            max(seg.is.y, seg.ie.y,
            seg.fs.y, seg.fe.y)
            );
    if ((bbox.first.x > p1.x) || !bbox.first.valid)
        bbox.first.x = p1.x;
    if ((bbox.first.y > p1.y) || !bbox.first.valid)
        bbox.first.y = p1.y;
    if ((bbox.second.x < p2.x) || !bbox.second.valid)
        bbox.second.x = p2.x;
    if ((bbox.second.y < p2.y) || !bbox.second.valid)
        bbox.second.y = p2.y;
    bbox.first.valid = 1;
    bbox.second.valid = 1;
}
