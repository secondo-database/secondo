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
//        cerr << "Matching: \n";
//        for (unsigned int i = 0; i < match.size(); i++) {
//            cerr << match[i].ToString() << "\n";
//        }
        face.MergeConcavity(m);
//        cerr << "Now is: \n";
//        cerr << face.ToString() << "\n";
    } else {
        holes.push_back(m);
    }
}

URegion MFace::ToURegion(Interval<Instant> iv, int facenr) {
    vector<MSegmentData> ms = face.ToMSegmentData(facenr, 0);
    cerr << "Creating URegion from a face and " << holes.size() << " holes!\n";
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(0, i+1);
        ms.insert(ms.end(), h.begin(), h.end());
    }
    for (unsigned int i = 0; i < ms.size(); i++) {
        cerr << ms[i].ToString() << "\n";
    }
    cerr << "Create done a.\n";
    URegion ret(ms, iv);
    
    cerr << "Create done b.";
    
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