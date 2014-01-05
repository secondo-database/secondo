/*
*/

#include "interpolate.h"

MFace::MFace() : needStartRegion(false), needEndRegion(false) {
}

MFace::MFace(MSegs face) : needStartRegion(false), needEndRegion(false),
        face(face) {
}

void MFace::AddMsegs(MSegs m) {
    vector<MSeg> match = m.GetMatchingMSegs(face);
    if (1 || match.size() > 0) {
        AddConcavity(m);
    } else {
        holes.push_back(m);
    }
}

void MFace::AddConcavity (MSegs c) {
    cvs.push_back(c);
}

void MFace::MergeConcavities () {
    for (unsigned int i = 0; i < cvs.size(); i++) {
        if (cvs[i].GetMatchingMSegs(face).size() > 0)
            face.MergeConcavity(cvs[i]);
        else {
            if (!cvs[i].sreg.ishole)
                needStartRegion = true;
            if (!cvs[i].dreg.ishole)
                needEndRegion = true;
            holes.push_back(cvs[i]);
        }
    }
    cvs.erase(cvs.begin(), cvs.end());
}

URegion MFace::ToURegion(Interval<Instant> iv, int facenr) {
    MergeConcavities();
    vector<MSegmentData> ms = face.ToMSegmentData(facenr, 0, 0);
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(facenr,i+1,ms.size());
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
        c = nl->OneElemList(nl->RealAtom(face.segs[cur].ie.x));
        le = nl->Append(c, nl->RealAtom(face.segs[cur].ie.y));
        le = nl->Append(le, nl->RealAtom(face.segs[cur].fe.x));
        le = nl->Append(le, nl->RealAtom(face.segs[cur].fe.y));
        le2 = nl->Append(le2, c);
        cur = face.findNext(cur);
        assert (cur >= 0);
    }
    
    return cy;
}

ListExpr MFace::ToListExpr() {
    MergeConcavities();
    
    ListExpr ret = nl->OneElemList(CycleToListExpr(face));
    ListExpr le = ret;
    for (unsigned int i = 0; i < holes.size(); i++) {
        le = nl->Append(le, CycleToListExpr(holes[i]));
    }
    
    return ret;
}



MFace MFace::divide (double start, double end) {
    MFace ret(face.divide(start, end));
    
    for (unsigned int i = 0; i < holes.size(); i++) {
        MSegs m = holes[i].divide(start, end);
        ret.AddMsegs(m);
//        ret.holes.push_back(holes[i].divide(start, end));
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