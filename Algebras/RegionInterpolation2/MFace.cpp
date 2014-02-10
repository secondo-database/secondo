/*
*/

#include "interpolate.h"

MFace::MFace() : needStartRegion(false), needEndRegion(false) {
}

MFace::MFace(MSegs face) : needStartRegion(false), needEndRegion(false),
face(face) {
    Check();
}

bool MFace::SortCycle() {
    vector<MSeg> s1 = face.segs;
    vector<MSeg> s2;

    int cur = 0, i = 0;
    bool ret = true;
    
    do {
        s2.push_back(s1[cur]);
        cur = face.findNext(cur);
        if (i++ > face.segs.size()) {
            ret = false;
            cerr << "Endless loop!\n";
            break;
        }
        
    } while (cur > 0);
    
    if (cur == -1) {
        ret = false;
        cerr << "Error: cycle incomplete!\n";
    } else if (cur == -2) {
        ret = false;
        cerr << "Error: cycle ambiguous!\n";
    } else if (cur < 0) {
        ret = false;
        cerr << "Error: unknown error in cycle!\n";
    }

    if (s2.size() != face.segs.size()) {
        ret = false;
        cerr << "Unused segments! " << "has " << face.segs.size() <<
                " used " << s2.size() << "\n";
    } else {
        face.segs = s2;
    }
    
    return ret;
}

bool MFace::Check() {
    bool ret = true;
    
    // Checking Border-Regions
    
    CreateBorderRegion(true);
    CreateBorderRegion(false);
    
    for (unsigned int i = 0; i < holes.size(); i++) {
        MFace h1(holes[i]);
        for (unsigned int j = 0; j < holes.size(); j++) {
            if (holes[i].intersects(holes[j], false, false))
                ret = false;
        }
    }
    
//    for (unsigned int i = 0; i < face.segs.size(); i++) {
//        for (unsigned int j = 0; j < face.segs.size(); j++) {
//            if (i == j)
//                continue;
//            MSeg a = face.segs[i];
//            MSeg b = face.segs[j];
//            if (a.intersects(b, true)) {
//                ret = false;
//                cerr << "Intersection!\n";
//            }
//            Seg ai(a.is, a.ie), af(a.fs, a.fe);
//            Seg bi(b.is, b.ie), bf(b.fs, b.fe);
//            if (ai.intersects(bi)) {
//                ret = false;
//                cerr << "Intersection initial!\n" << ai.ToString()
//                        << " " << bi.ToString() << "\n";
//            }
//            if (af.intersects(bf)) {
//                ret = false;
//                cerr << "Intersection final!\n" << af.ToString()
//                        << " " << bf.ToString() << "\n";
//            }
//        }
//    }

    
    if (!SortCycle()) {
        ret = false;
    }

    
    if (face.intersects(face, false, true)) {
        cerr << "Intersection!\n";
        ret = false;
    }
    
    if (!ret) {
        cerr << "Error with MFace " << this->ToString() << "\n";
    }
    assert(ret);

    return ret;
}

void MFace::AddMsegs(MSegs m) {
    AddConcavity(m);
}

void MFace::AddConcavity(MSegs c) {
    cvs.push_back(c);
}

static ListExpr CycleToListExpr(MSegs face);

void MFace::MergeConcavities() {
    Check();
    for (unsigned int i = 0; i < cvs.size(); i++) {
        MFace check(cvs[i]);
        if (face.intersects(cvs[i], false, false))
            assert(false);
        cerr << "Merging\n";
        cerr << face.ToString() << "\n"; 
        PrintMRegionListExpr();
        cerr << "\nwith\n";
        cerr << cvs[i].ToString() << "\n"; 
        check.PrintMRegionListExpr();
        cerr << "\n";
        if (face.MergeConcavity(cvs[i])) {
            cerr << "Merged\n";
            Check();
        } else {
            if (!cvs[i].sreg.ishole)
                needStartRegion = true;
            if (!cvs[i].dreg.ishole)
                needEndRegion = true;
            holes.push_back(cvs[i]);
        }
    }
    
    Check();

    cvs.erase(cvs.begin(), cvs.end());
}

URegion MFace::ToURegion(Interval<Instant> iv, int facenr) {
    MergeConcavities();
    vector<MSegmentData> ms = face.ToMSegmentData(facenr, 0, 0);
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(facenr, i + 1,
                ms.size());
        ms.insert(ms.end(), h.begin(), h.end());
    }
    URegion ret(ms, iv);

    return ret;
}

static ListExpr CycleToListExpr(MSegs face) {
    ListExpr le, le2;
    int first, cur;
    first = cur = 0;

    assert(face.segs.size() > 0);

    ListExpr c = nl->OneElemList(nl->RealAtom(face.segs[cur].ie.x));
    le = nl->Append(c, nl->RealAtom(face.segs[cur].ie.y));
    le = nl->Append(le, nl->RealAtom(face.segs[cur].fe.x));
    le = nl->Append(le, nl->RealAtom(face.segs[cur].fe.y));
    ListExpr cy = nl->OneElemList(c);
    le2 = cy;
    cur = face.findNext(cur);
    while (cur != first) {
        assert(cur >= 0);
        c = nl->OneElemList(nl->RealAtom(face.segs[cur].ie.x));
        le = nl->Append(c, nl->RealAtom(face.segs[cur].ie.y));
        le = nl->Append(le, nl->RealAtom(face.segs[cur].fe.x));
        le = nl->Append(le, nl->RealAtom(face.segs[cur].fe.y));
        le2 = nl->Append(le2, c);
        cur = face.findNext(cur);
    }

    return cy;
}

ListExpr MFace::ToListExpr() {
//    MergeConcavities();

    ListExpr ret = nl->OneElemList(CycleToListExpr(face));
    ListExpr le = ret;
    for (unsigned int i = 0; i < holes.size(); i++) {
        le = nl->Append(le, CycleToListExpr(holes[i]));
    }

    return ret;
}

void MFace::PrintMRegionListExpr() {
    cerr << "(OBJECT mr () mregion ( ( "
            "(\"2013-01-01\" \"2013-01-02\" TRUE TRUE)"
            << nl->ToString(ToListExpr()) << ") ) )";
}

MFace MFace::divide(double start, double end) {
    MFace ret(face.divide(start, end));

    for (unsigned int i = 0; i < holes.size(); i++) {
        MSegs m = holes[i].divide(start, end);
        ret.AddMsegs(m);
    }
    ret.MergeConcavities();

    return ret;
}

string MFace::ToString() {
    std::ostringstream ss;

    ss << "Face:\n";
    ss << face.ToString();
    for (unsigned int i = 0; i < holes.size(); i++) {
        ss << "Hole " << i << ":\n"
                << holes[i].ToString();
    }
    ss << "\n";

    return ss.str();
}

Face MFace::CreateBorderRegion(bool src) {
    vector<Seg> segs;

    assert(SortCycle());
    cerr << "CBR " << face.ToString() << "\n";

    for (unsigned int i = 0; i < face.segs.size(); i++) {
        MSeg ms = face.segs[i];
        if ((src && (ms.is == ms.ie)) ||
                (!src && (ms.fs == ms.fe)))
            continue;
        segs.push_back(src ? Seg(ms.is, ms.ie) : Seg(ms.fs, ms.fe));
    }

    Face ret(segs);

    for (unsigned int h = 0; h < holes.size(); h++) {
        vector<Seg> hole;
        MSegs mss = holes[h];
        for (unsigned int i = 0; i < mss.segs.size(); i++) {
            MSeg ms = mss.segs[i];
            if ((src && (ms.is == ms.ie)) ||
                    (!src && (ms.fs == ms.fe)))
                continue;
            hole.push_back(src ? Seg(ms.is, ms.ie) : Seg(ms.fs, ms.fe));
        }
        ret.AddHole(hole);
    }

    return ret;
}

