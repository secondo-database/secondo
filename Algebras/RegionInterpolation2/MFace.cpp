/*
*/

#include "interpolate.h"

MFace::MFace() {
}

MFace::MFace(MSegs face) : face(face) {
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
        else
            holes.push_back(cvs[i]);
    }
    cvs.erase(cvs.begin(), cvs.end());
}

URegion MFace::ToURegion(Interval<Instant> iv, int facenr) {
    MergeConcavities();
    vector<MSegmentData> ms = face.ToMSegmentData(facenr, 0);
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(facenr, i+1);
        ms.insert(ms.end(), h.begin(), h.end());
    }
    URegion ret(ms, iv);
    
    
    return ret;
}

MFace MFace::divide (double start, double end) {
    MFace ret(face.divide(start, end));
    
    for (unsigned int i = 0; i < holes.size(); i++) {
        ret.holes.push_back(holes[i].divide(start, end));
    }
    
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