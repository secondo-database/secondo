/*
*/

#include "interpolate.h"

MSegs::MSegs() : ignore(0), iscollapsed(0), id(-1) {
}

void MSegs::AddMSeg(MSeg m) {
    if ((m.is == m.ie) && (m.fs == m.fe))
        return;

#if 1
    bool merged = false;
    if (!segs.empty()) {
        MSeg *prev = &segs[segs.size() - 1];
        merged = prev->Merge(m);
    }
    if (!merged)
#endif
        segs.push_back(m);
    calculateBBox();
}

//vector<MSeg> MSegs::GetMatchingMSegs(MSegs m) {
//    vector<MSeg> ret;
//    std::sort(segs.begin(), segs.end());
//    std::sort(m.segs.begin(), m.segs.end());
//
//    unsigned int i = 0, j = 0;
//    while (i < segs.size() && j < m.segs.size()) {
//        if (segs[i] == m.segs[j]) {
//            ret.push_back(segs[i]);
//            i++;
//            j++;
//        } else if (segs[i] < m.segs[j]) {
//            i++;
//        } else {
//            j++;
//        }
//    }
//
//    return ret;
//}

bool MSegs::MergeConcavity(MSegs c) {
    bool fastPath = true;
    bool success = false;

    for (unsigned int i = 0; i < segs.size(); i++) {
        if (segs[i].ip.size() > 2 || segs[i].fp.size() > 2) {
            fastPath = false;
            break;
        }
    }
    
    if (fastPath) {
        std::sort(segs.begin(), segs.end());
        std::sort(c.segs.begin(), c.segs.end());

        std::vector<MSeg>::iterator i = segs.begin();
        std::vector<MSeg>::iterator j = c.segs.begin();
        while (i != segs.end() && j != c.segs.end()) {
            if (*i == *j) {
                i = segs.erase(i);
                j = c.segs.erase(j);
                success = true;
            } else if (*i < *j) {
                i++;
            } else {
                j++;
            }
        }
    } else {

        std::vector<MSeg>::iterator i = segs.begin();

        while (i != segs.end()) {
            std::vector<MSeg>::iterator j = c.segs.begin();
            bool integrated = false;
            while (j != c.segs.end()) {
                MSeg m1, m2;
                if ((integrated = i->Integrate(*j, m1, m2))) {
                    cerr << "Integrated " << i->ToString() <<
                            " with " << j->ToString() << "\n";
                    i = segs.erase(i);
                    j = c.segs.erase(j);
                    AddMSeg(m1);
                    AddMSeg(m2);
                    success = true;
                    break;
                }
                j++;
            }
            if (!integrated)
                i++;
        }
    }
    
    if (success) {
        for (unsigned int x = 0; x < c.segs.size(); x++)
            c.segs[x].ChangeDirection();

        segs.insert(segs.end(), c.segs.begin(), c.segs.end());
        std::sort(segs.begin(), segs.end());
    }

    return success;
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

bool MSegs::intersects(const MSegs& a, bool matchIdent, bool matchSegs) const {

    // Fast path if the boundingboxes are disjoint

    //    if ((bbox.first.x > a.bbox.second.x) ||
    //        (a.bbox.first.x > bbox.second.x) ||
    //        (bbox.first.y > a.bbox.second.y) ||
    //        (a.bbox.first.y > bbox.second.y))
    //            return false;


    bool ret = false;
    for (unsigned int i = 0; i < a.segs.size(); i++) {
        for (unsigned int j = 0; j < segs.size(); j++) {
            if ((matchIdent || !(segs[j] == a.segs[i])) &&
                    segs[j].intersects(a.segs[i], matchSegs)) {
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
    int ret = -1;

    for (unsigned int i = 0; i < nrsegs; i++) {
        int nindex = (i + index) % nrsegs;
        MSeg *s2 = &segs[nindex];
        if (s1->ie == s2->is && s1->fe == s2->fs) {
            if (ret == -1)
                ret = nindex;
            else {
                cerr << segs[index].ToString() << " has 2 sucessors:\n" <<
                        segs[ret].ToString() << " AND\n" <<
                        segs[nindex].ToString() << "\n";

                ret = -2;
                break;
            }
        }
    }

    return ret;
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
