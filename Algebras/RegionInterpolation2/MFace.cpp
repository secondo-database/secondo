/*
*/

#include "interpolate.h"

MFace::MFace() {
}

MFace::MFace(MSegs face) : face(face) {
}

void MFace::AddMsegs(MSegs m) {
    vector<MSeg> match = m.GetMatchingMSegs(face);
    if (match.size() > 0) {
        face.MergeConcavity(m);
    } else {
        holes.push_back(m);
    }
}

URegion MFace::ToURegion(Interval<Instant> iv, int facenr) {
    vector<MSegmentData> ms = face.ToMSegmentData(facenr, 0);
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(0, i+1);
        ms.insert(ms.end(), h.begin(), h.end());
    }
    URegion ret(ms, iv);
    
    
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