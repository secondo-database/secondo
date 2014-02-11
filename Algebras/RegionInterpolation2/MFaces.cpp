/*
*/

#include "interpolate.h"

MFaces::MFaces() : sregs(NULL), dregs(NULL),
        needSEvap(false), needDEvap(false) {
}

MFaces::MFaces(MFace face) : sregs(NULL), dregs(NULL), 
        needSEvap(false), needDEvap(false) {
    AddFace(face);
}

void MFaces::AddFace(MFace face) {
    faces.push_back(face);
}

MFaces MFaces::CreateBorderMFaces(bool src) {
    MFaces ret;
    vector<Face> regs = CreateBorderRegions(src);

    for (unsigned int i = 0; i < regs.size(); i++) {
        ret.AddFace(regs[i].GetMSegs(false));
    }

    return ret;
}

vector<Face> MFaces::CreateBorderRegions(bool src) {
    vector<Face> ret;

    for (unsigned int i = 0; i < faces.size(); i++) {
        Face r = faces[i].CreateBorderRegion(src);
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
    
    if (faces.empty())
        return nl->Empty();

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

DateTime msec1(durationtype, 0);

MRegion MFaces::ToMRegion(Interval<Instant> _iv) {
    MRegion ret(1);
    bool needStartRegion = false, needEndRegion = false;
    Interval<Instant> iv;
    Instant cur;
    iv.CopyFrom(_iv);
    
    for (unsigned int i = 0; i < faces.size(); i++) {
        faces[i].MergeConcavities();
        needStartRegion = needStartRegion || faces[i].needStartRegion;
        needEndRegion = needEndRegion || faces[i].needEndRegion;
    }
    
//    needSEvap = needDEvap = false;

    Instant onethird = (_iv.end - _iv.start) / 3;
    // Compiling intervals
    Interval<Instant> startEvapIv, endEvapIv;
    Interval<Instant> startRegIv, endRegIv;
    Interval<Instant> mainIv;

    mainIv.CopyFrom(_iv);
    if (needSEvap) {
        startEvapIv.lc = mainIv.lc;

        startEvapIv.start = mainIv.start;
        mainIv.start = mainIv.start + onethird;
        startEvapIv.end = mainIv.start;

        startEvapIv.rc = true;
        mainIv.lc = false;
    }

    if (needDEvap) {
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


    if (needSEvap) {
        cerr << "\n==== Start-Evaporations ====\n";
        MFaces fs;
        cerr << "CreateBorderRegs start\n";
        vector<Face> bordersregs = CreateBorderRegions(true);
        cerr << "Interpolate start\n";
        fs = interpolate(sregs, &bordersregs, 0, true, "");
        cerr << "Interpolate end\n";
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

    if (needDEvap) {
        cerr << "\n==== End-Evaporations ====\n";
        MFaces fs;
        vector<Face> borderdregs = CreateBorderRegions(false);
        fs = interpolate(&borderdregs, dregs, 0, true, "");
        URegion u = fs.ToURegion(endEvapIv, 0, 1);
        ret.AddURegion(u);
        cerr << "==== /End-Evaporations ====\n";
    }

    return ret;
}

static void Append(ListExpr &head, ListExpr l) {
    if (l == nl->Empty())
        return;
    if (head == nl->Empty()) {
        head = nl->OneElemList(l);
    } else {
        nl->Append(nl->End(head), l);
    }
}

ListExpr MFaces::ToMListExpr(Interval<Instant> _iv) {
    bool needStartRegion = false, needEndRegion = false;
    Interval<Instant> iv;
    Instant cur;
    iv.CopyFrom(_iv);

    ListExpr mreg = nl->Empty();

    for (unsigned int i = 0; i < faces.size(); i++) {
        faces[i].MergeConcavities();
        needStartRegion = needStartRegion || faces[i].needStartRegion;
        needEndRegion = needEndRegion || faces[i].needEndRegion;
    }

    Instant onethird = (_iv.end - _iv.start) / 3;
    // Compiling intervals
    Interval<Instant> startEvapIv, endEvapIv;
    Interval<Instant> startRegIv, endRegIv;
    Interval<Instant> mainIv;

//    needSEvap = needDEvap = false;

    mainIv.CopyFrom(_iv);
    if (needSEvap) {
        startEvapIv.lc = mainIv.lc;

        startEvapIv.start = mainIv.start;
        mainIv.start = mainIv.start + onethird;
        startEvapIv.end = mainIv.start;

        startEvapIv.rc = false;
        mainIv.lc = true;
    }

    if (needDEvap) {
        endEvapIv.rc = mainIv.rc;

        endEvapIv.end = mainIv.end;
        mainIv.end = mainIv.end - onethird;
        endEvapIv.start = mainIv.end;

        endEvapIv.lc = false;
        mainIv.rc = true;
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


    if (needSEvap) {
        cerr << "\n==== Start-Evaporations ====\n";
        MFaces fs;
        vector<Face> bordersregs = CreateBorderRegions(true);
        cerr << "Interpolate start\n";
        cerr << (*sregs)[0].ToString() << "\n";
        fs = interpolate(sregs, &bordersregs, 0, true, "");
        cerr << "Interpolate end\n";
        Append(mreg, fs.ToListExpr(startEvapIv, 0, 1));
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

    if (needDEvap) {
        cerr << "\n==== End-Evaporations ====\n";
        MFaces fs;
        vector<Face> borderdregs = CreateBorderRegions(false);
        fs = interpolate(&borderdregs, dregs, 0, true, "");
        Append(mreg, fs.ToListExpr(endEvapIv, 0, 1));
        cerr << "==== /End-Evaporations ====\n";
    }
    
    cerr << "\n==== Result ====\n";
    nl->WriteListExpr(mreg);
    cerr << "\n";
    
    return mreg;
}

ListExpr MFaces::fallback(vector<Face> *sregs, vector<Face> *dregs,
        Interval<Instant> iv) {
    MFaces start;
    for (unsigned int i = 0; i < sregs->size(); i++) {
        Face r = (*sregs)[i];
        MFace f(r.GetMSegs(false));
        start.AddFace(f);
    }
    MFaces end;
    for (unsigned int i = 0; i < dregs->size(); i++) {
        Face r = (*dregs)[i];
        MFace f(r.GetMSegs(false));
        end.AddFace(f);
    }

    Interval<Instant> startiv(iv.start, iv.start + (iv.end - iv.start) / 2,
            iv.lc, true);
    Interval<Instant> endiv(startiv.end, iv.end, false, iv.rc);

    ListExpr mreg = nl->Empty();
    Append(mreg, start.ToListExpr(startiv, 0, 1));
    Append(mreg, end.ToListExpr(endiv, 0, 1));

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