/*
*/

#include "interpolate.h"

MFaces::MFaces() : sregs(NULL), dregs(NULL) {
}

MFaces::MFaces(MFace face) : sregs(NULL), dregs(NULL) {
    AddFace(face);
}

void MFaces::AddFace(MFace face) {
    faces.push_back(face);
}

//MFaces MFaces::GetBorderRegions(vector<Reg> *regs) {
//    MFaces ret;
//
//    for (unsigned int i = 0; i < regs->size(); i++) {
//        ret.AddFace(MFace((*regs)[i].GetMSegs()));
//    }
//
//    return ret;
//}

MFaces MFaces::CreateBorderMFaces(bool src) {
    MFaces ret;
    vector<Reg> regs = CreateBorderRegions(src);
    
    for (unsigned int i = 0; i < regs.size(); i++) {
        ret.AddFace(regs[i].GetMSegs());
    }
    
    return ret;
}

vector<Reg> MFaces::CreateBorderRegions(bool src) {
    vector<Reg> ret;
    
    for (unsigned int i = 0; i < faces.size(); i++) {
        Reg r = faces[i].CreateBorderRegion(src);
        if (r.v.size() >= 3)
            ret.push_back(r);
    }
    
    return ret;
}

URegion MFaces::ToURegion(Interval<Instant> iv, double start, double end) {
    vector<URegion> uregs;

    if (start == 0 && end == 1) {
        for (unsigned int i = 0; i < faces.size(); i++) {
            URegion u = faces[i].ToURegion(iv, i);
            uregs.push_back(u);
        }
    } else {
        MFaces f = divide(start, end);
        URegion ret = f.ToURegion(iv, 0, 1);

        return ret;
    }

    URegion ret(uregs[0]);
    for (unsigned int i = 1; i < uregs.size(); i++) {
        ret.AddURegion(&uregs[i]);
    }

    return ret;
}

ListExpr MFaces::ToListExpr(Interval<Instant> iv, double start, double end) {
    ListExpr le;
    
    assert(faces.size() > 0);
    
    if (start == 0 && end == 1) {
        ListExpr inst =
                nl->OneElemList(nl->StringAtom(iv.start.ToString(false), true));
        le = nl->Append(inst, nl->StringAtom(iv.end.ToString(false), true));
        le = nl->Append(le, nl->BoolAtom(iv.lc));
        le = nl->Append(le, nl->BoolAtom(iv.rc));
        ListExpr ur = nl->OneElemList(inst);

        ListExpr urs = nl->OneElemList(faces[0].ToListExpr());
        le = urs;
        for (unsigned int i = 1; i < faces.size(); i++) {
            le = nl->Append(le, faces[i].ToListExpr());
        }
        nl->Append(ur, urs);

        return ur;
    } else {
        MFaces f = divide(start, end);
        return f.ToListExpr(iv, 0, 1);
    }
}

MRegion MFaces::ToMRegion(Interval<Instant> _iv) {
    MRegion ret(1);
    bool needStartRegion = false, needEndRegion = false,
            needStartEvap = true, needEndEvap = true;
    Interval<Instant> iv;
    Instant cur;
    iv.CopyFrom(_iv);
    DateTime msec1(durationtype, 10000);

    for (unsigned int i = 0; i < faces.size(); i++) {
        faces[i].MergeConcavities();
        needStartRegion = needStartRegion || faces[i].needStartRegion;
        needEndRegion = needEndRegion || faces[i].needEndRegion;
    }
    
    Instant onethird = (_iv.end - _iv.start)/3;
    // Compiling intervals
    Interval<Instant> startEvapIv, endEvapIv;
    Interval<Instant> startRegIv, endRegIv;
    Interval<Instant> mainIv;
    
    mainIv.CopyFrom(_iv);
    if (needStartEvap) {
        startEvapIv.lc = mainIv.lc;
        
        startEvapIv.start = mainIv.start;
        mainIv.start = mainIv.start + onethird;
        startEvapIv.end = mainIv.start;
        
        startEvapIv.rc = true;
        mainIv.lc = false;
    }
    
    if (needEndEvap) {
        endEvapIv.rc = mainIv.rc;
        
        endEvapIv.end = mainIv.end;
        mainIv.end = mainIv.end - onethird;
        endEvapIv.start = mainIv.end;
        
        endEvapIv.lc = true;
        mainIv.rc = false;
    }
    
    if (needStartRegion) {
        startRegIv.lc = mainIv.lc;
        
        startRegIv.start = mainIv.start;
        mainIv.start = mainIv.start + msec1;
        startRegIv.end = mainIv.start;
        
        startRegIv.rc = true;
        mainIv.lc = false;
    }
    
    if (needEndRegion) {
        endRegIv.rc = mainIv.rc;
        
        endRegIv.end = mainIv.end;
        mainIv.end = mainIv.end - msec1;
        endRegIv.start = mainIv.end;
        
        endRegIv.lc = true;
        mainIv.rc = false;
    }
    

    if (needStartEvap) {
        cerr << "\n==== Start-Evaporations ====\n";
        MFaces fs;
        vector<Reg> bordersregs = CreateBorderRegions(true);
        fs = interpolate(sregs, &bordersregs, 0, true);
        URegion u = fs.ToURegion(startEvapIv, 0, 1);
        ret.AddURegion(u);
        cerr << "==== /Start-Evaporations ====\n";
    }
    
    if (needStartRegion) {
        cerr << "\n==== Start-Region ====\n";
        URegion u = CreateBorderMFaces(true).ToURegion(startRegIv, 0, 1);
        ret.AddURegion(u);
        cerr << "==== /Start-Region ====\n";
    }

    if (1) {
        cerr << "\n==== Main-Interpolation to List ====\n";
        URegion u = ToURegion(mainIv, 0, 1);
        ret.AddURegion(u);
        cerr << "==== /Main-Interpolation to List ====\n";
    }
    
    if (needEndRegion) {
        cerr << "\n==== End-Region ====\n";
        URegion u = CreateBorderMFaces(false).ToURegion(endRegIv, 0, 1);
        ret.AddURegion(u);
        cerr << "==== /End-Region ====\n";
    }
    
    if (needEndEvap) {
        cerr << "\n==== End-Evaporations ====\n";
        MFaces fs;
        vector<Reg> borderdregs = CreateBorderRegions(false);
        fs = interpolate(&borderdregs, dregs, 0, true);
        URegion u = fs.ToURegion(endEvapIv, 0, 1);
        ret.AddURegion(u);
        cerr << "==== /End-Evaporations ====\n";
    }

    return ret;
}

//MRegion MFaces::ToMRegion(Interval<Instant> _iv) {
//    MRegion ret(1);
//    bool needStartRegion = false, needEndRegion = false;
//    Interval<Instant> iv;
//    iv.CopyFrom(_iv);
//    DateTime msec1(durationtype, 1);
//
//    for (unsigned int i = 0; i < faces.size(); i++) {
//        faces[i].MergeConcavities();
//        needStartRegion = needStartRegion || faces[i].needStartRegion;
//        needEndRegion = needEndRegion || faces[i].needEndRegion;
//    }
//
//    if (needStartRegion) {
//        iv.lc = false;
//        Interval<Instant> startiv(iv.start, iv.start + msec1, true, true);
//        iv.start = iv.start + msec1;
//        URegion start = CreateBorderMFaces(true).ToURegion(startiv, 0, 1);
//        ret.AddURegion(start);
//    }
//
//    if (needEndRegion) {
//        iv.rc = false;
//        Interval<Instant> endiv(iv.end - msec1, iv.end, true, true);
//        iv.end = iv.end - msec1;
//        URegion end = CreateBorderMFaces(false).ToURegion(endiv, 0, 1);
//        ret.AddURegion(end);
//    }
//
//    URegion ureg = ToURegion(iv, 0, 1);
//    ret.AddURegion(ureg);
//
//    return ret;
//}

static void Append(ListExpr &head, ListExpr l) {
    if (head == nl->Empty()) {
        head = nl->OneElemList(l);
    } else {
        nl->Append(nl->End(head), l);
    }
}

ListExpr MFaces::ToMListExpr(Interval<Instant> _iv) {
    bool needStartRegion = false, needEndRegion = false,
            needStartEvap = true, needEndEvap = true;
    Interval<Instant> iv;
    Instant cur;
    iv.CopyFrom(_iv);
    DateTime msec1(durationtype, 10000);

    ListExpr mreg = nl->Empty();
    
    for (unsigned int i = 0; i < faces.size(); i++) {
        faces[i].MergeConcavities();
        needStartRegion = needStartRegion || faces[i].needStartRegion;
        needEndRegion = needEndRegion || faces[i].needEndRegion;
    }
    
    Instant onethird = (_iv.end - _iv.start)/3;
    // Compiling intervals
    Interval<Instant> startEvapIv, endEvapIv;
    Interval<Instant> startRegIv, endRegIv;
    Interval<Instant> mainIv;
    
    mainIv.CopyFrom(_iv);
    if (needStartEvap) {
        startEvapIv.lc = mainIv.lc;
        
        startEvapIv.start = mainIv.start;
        mainIv.start = mainIv.start + onethird;
        startEvapIv.end = mainIv.start;
        
        startEvapIv.rc = true;
        mainIv.lc = false;
    }
    
    if (needEndEvap) {
        endEvapIv.rc = mainIv.rc;
        
        endEvapIv.end = mainIv.end;
        mainIv.end = mainIv.end - onethird;
        endEvapIv.start = mainIv.end;
        
        endEvapIv.lc = true;
        mainIv.rc = false;
    }
    
    if (needStartRegion) {
        startRegIv.lc = mainIv.lc;
        
        startRegIv.start = mainIv.start;
        mainIv.start = mainIv.start + msec1;
        startRegIv.end = mainIv.start;
        
        startRegIv.rc = true;
        mainIv.lc = false;
    }
    
    if (needEndRegion) {
        endRegIv.rc = mainIv.rc;
        
        endRegIv.end = mainIv.end;
        mainIv.end = mainIv.end - msec1;
        endRegIv.start = mainIv.end;
        
        endRegIv.lc = true;
        mainIv.rc = false;
    }
    

    if (needStartEvap) {
        cerr << "\n==== Start-Evaporations ====\n";
        MFaces fs;
        vector<Reg> bordersregs = CreateBorderRegions(true);
        fs = interpolate(sregs, &bordersregs, 0, true);
        Append(mreg,fs.ToListExpr(startEvapIv, 0, 1));
        cerr << "==== /Start-Evaporations ====\n";
    }
    
    if (needStartRegion) {
        cerr << "\n==== Start-Region ====\n";
        Append(mreg, CreateBorderMFaces(true).ToListExpr(startRegIv, 0, 1));
        cerr << "==== /Start-Region ====\n";
    }

    cerr << "\n==== Main-Interpolation to List ====\n";
    Append(mreg, ToListExpr(mainIv, 0, 1));
    cerr << "==== /Main-Interpolation to List ====\n";

    if (needEndRegion) {
        cerr << "\n==== End-Region ====\n";
        Append(mreg, CreateBorderMFaces(false).ToListExpr(endRegIv, 0, 1));
        cerr << "==== /End-Region ====\n";
    }
    
    if (needEndEvap) {
        cerr << "\n==== End-Evaporations ====\n";
        MFaces fs;
        vector<Reg> borderdregs = CreateBorderRegions(false);
        fs = interpolate(&borderdregs, dregs, 0, true);
        Append(mreg,fs.ToListExpr(endEvapIv, 0, 1));
        cerr << "==== /End-Evaporations ====\n";
    }

    return mreg;
}

MFaces MFaces::divide(double start, double end) {
    MFaces ret;

    for (unsigned int i = 0; i < faces.size(); i++) {
        ret.AddFace(faces[i].divide(start, end));
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