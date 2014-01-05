/*
*/

#include "interpolate.h"

MSegs::MSegs() : ignore(0), iscollapsed(0) {
}

void MSegs::AddMSeg (MSeg m) {
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
    cerr << "Merging " << ToString() << " with " << c.ToString() << "\n";
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
    for (unsigned int x = 0; x < c.segs.size(); x++)
        c.segs[x].ChangeDirection();
    segs.insert(segs.end(),c.segs.begin(),c.segs.end());
    std::sort(segs.begin(), segs.end());
    cerr << "Result: " << ToString() << "\n\n";
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

// Inefficient!
bool MSegs::intersects (const MSegs& a) const {
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

MSegs MSegs::divide (double start, double end) {
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

int MSegs::findNext (int index) {
    int nrsegs = segs.size();
    MSeg *s1 = &segs[index];
    
    for (unsigned int i = 0; i < nrsegs; i++) {
        int nindex = (i+index)%nrsegs;
        MSeg *s2 = &segs[nindex];
        if (s1->ie == s2->is && s1->fe == s2->fs)
            return nindex;
    }
    
    return -1;
}
