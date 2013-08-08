/*
*/

#include "interpolate.h"

MFaces::MFaces() {
}

MFaces::MFaces(MFace face) {
    AddFace(face);
}

void MFaces::AddFace(MFace face) {
    faces.push_back(face);
}

MRegion MFaces::ToMRegion(Interval<Instant> iv) {
    MRegion ret(1);

    if (faces.size() > 0) {
        URegion u = faces[0].ToURegion(iv, 0);
        for (unsigned int i = 1; i < faces.size(); i++) {
            URegion u2 = faces[i].ToURegion(iv, i);
            u.AddURegion(&u2);
        }
        ret.AddURegion(u);
    }

    return ret;
}

string MFaces::ToString() {
    std::ostringstream ss;

    ss << "\n"
       << "=========================  MFaces  ========================\n";
    
    for (unsigned int i = 0; i < faces.size(); i++) {
        ss << " === Face " << i << " ===\n";
        ss << faces[i].ToString();
    }
            
    ss << "=========================  /MFaces  ========================\n\n";
    
    return ss.str();
}