/*
   Merge.cpp   Join two Faces

*/

#include "interpolate.h"

static Face JoinFaces (Face f1, Face f2, Pt jp) {
    Face ret;
    int if1 = 0;
    int if2 = 0;
    
    DEBUG(2, "F1: " << f1.ToString());
    DEBUG(2, "F2: " << f2.ToString());
    
    for (unsigned int i = 0; i < f1.v.size(); i++) {
        if (f1.v[i].s == jp)
            if1 = i;
    }
    
    for (unsigned int i = 0; i < f2.v.size(); i++) {
        if (f2.v[i].s == jp)
            if2 = i;
    }
    
    vector<Seg> v;
    for (unsigned int i = 0; i < f1.v.size(); i++) {
        v.push_back(f1.v[(i+if1)%f1.v.size()]);
    }
    for (unsigned int i = 0; i < f2.v.size(); i++) {
        v.push_back(f2.v[(i+if2)%f2.v.size()]);
    }
    
    double minx = 0, miny = 0;
    int start = -1;

    // Find the segment with the lowest-left start-point
    for (unsigned int i = 0; i < v.size(); i++) {
        cerr << "X: " << v[i].ToString() << "\n";
        if ((v[i].s.y < miny) || ((v[i].s.y == miny) &&
                (v[i].s.x < minx)) || (start < 0)) {
            start = i;
            miny = v[i].s.y;
            minx = v[i].s.x;
        }
    }
    cerr << "Found startpoint " << start << "\n";
    ret.v.insert(ret.v.end(), v.begin()+start, v.end());
    ret.v.insert(ret.v.end(), v.begin(), v.begin()+start);
    
    for (unsigned int i = 0; i < f1.holes.size(); i++) {
        ret.holes.push_back(f1.holes[i]);
    }
    for (unsigned int i = 0; i < f2.holes.size(); i++) {
        ret.holes.push_back(f2.holes[i]);
    }
    
    DEBUG(2, "RET: " << ret.ToString());
    
    return ret;    
}

vector<MFace> MergeFaces (Matches& m) {
    vector<MFace> fcs;
    
    assert ((m.src.size() == 1 && m.dst.size() > 1) ||
            (m.dst.size() == 1 && m.src.size() > 1));
    
    Face *a1, *a2, *b;
    
    if (m.dst.size() > 1) {
        a1 = m.dst[0];
        a2 = m.dst[1];
        b  = m.srcface();
    } else {
        a1 = m.src[0];
        a2 = m.src[1];
        b  = m.dstface();
    }
    Pt p1, p2;
    
    if (a1->bbox.first.x > a2->bbox.second.x) {
        p1 = a1->findX(a1->bbox.first.x);
        p2 = a2->findX(a2->bbox.second.x);
    } else if (a2->bbox.first.x > a1->bbox.second.x) {
        p1 = a2->findX(a2->bbox.first.x);
        p2 = a1->findX(a1->bbox.second.x);
    } else if (a1->bbox.first.y > a2->bbox.second.y) {
        p1 = a1->findY(a1->bbox.first.y);
        p2 = a2->findY(a2->bbox.second.y);
    } else if (a2->bbox.first.y > a1->bbox.second.y) {
        p1 = a2->findY(a2->bbox.first.y);
        p2 = a1->findY(a1->bbox.second.y);
    } else
        assert(false);
    
    Pt v = (p2 - p1) / 2;
    Pt jp = (p2 + p1) / 2;
    
    MFace mf1 = a1->GetMFace(false, -v);
    MFace mf2 = a2->GetMFace(false, v);
    if (m.dst.size() > 1) {
        mf1.reverse();
        mf2.reverse();
    }
    fcs.push_back(mf1);
    fcs.push_back(mf2);
    
    Face f1 = *a1;
    f1.Transform(-v, Pt(1,1));    
    Face f2 = *a2;
    f2.Transform(v, Pt(1,1));
    Face *f = new Face();
    *f = JoinFaces(f1, f2, jp);
    
    DEBUG(3, "Joined Face: " << f->ToString());
    
    if (m.src.size() > 1) {
        m.src.clear();
        m.src.push_back(f);
    } else {
        m.dst.clear();
        m.dst.push_back(f);        
    }
    
    return fcs;
}
