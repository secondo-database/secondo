/*
   1 MFaces represents a set of Moving Faces (MFace-objects) in one
   (unspecified) time-interval.

*/

#include "interpolate.h"

MFaces::MFaces() : sregs(NULL), dregs(NULL),
        needSEvap(false), needDEvap(false) {
}

/*
 1.1 Constructs an MFaces-object with one MFace
 
*/
MFaces::MFaces(MFace face) : sregs(NULL), dregs(NULL), 
        needSEvap(false), needDEvap(false) {
    AddMFace(face);
}

/*
 1.2 AddFace adds an additional MFace to this MFaces-object
 
*/
void MFaces::AddMFace(MFace face) {
    faces.push_back(face);
}

/*
 1.3 CreateBorderMFaces constructs a new MFaces-Object which 
 statically represents the situation at the start or end of the
 time-interval (depending on the parameter ~src~).
 
*/
MFaces MFaces::CreateBorderMFaces(bool src) {
    MFaces ret;
    
    vector<Face> fcs = CreateBorderFaces(src);

    for (unsigned int i = 0; i < fcs.size(); i++) {
        MFace f(fcs[i].GetMSegs(true));
        if (!f.isEmpty())
            ret.AddMFace(f);
    }

    return ret;
}

/*
 1.4 CreateBorderFaces creates a list of Faces representing the region at the
 start (or the end, depending on the parameter ~src~) of the time-interval.
 
*/
vector<Face> MFaces::CreateBorderFaces(bool src) {
    vector<Face> ret;

    for (unsigned int i = 0; i < faces.size(); i++) {
        Face f = faces[i].CreateBorderFace(src);
        if (!f.isEmpty())
            ret.push_back(faces[i].CreateBorderFace(src));
    }

    return ret;
}

// Helper function, which appends a list-item to the end of a list.
static void Append(ListExpr &head, ListExpr l) {
    if (l == nl->Empty())
        return;
    if (head == nl->Empty()) {
        head = nl->OneElemList(l);
    } else {
        nl->Append(nl->End(head), l);
    }
}

/*
 1.6 ToListExpr converts this MFaces-object to a NestedList-representation of
 a Secondo-mregion over the given interval with the given restrictions.
 
*/
ListExpr MFaces::ToListExpr(Interval<Instant> iv, double start, double end) {
    
    if (faces.empty())
        return nl->Empty();

    if (start == 0 && end == 1) {
        // No restrictions, so we can start the real work
        
        // Build the ListExpr for the time-interval
        ListExpr interval = nl->Empty();
        Append(interval, nl->StringAtom(iv.start.ToString(false), true));
        Append(interval, nl->StringAtom(iv.end.ToString(false), true));
        Append(interval, nl->BoolAtom(iv.lc));
        Append(interval, nl->BoolAtom(iv.rc));

        ListExpr uregion = nl->OneElemList(interval);
        
        // Now convert the mface-objects one by one and put them into one list
        ListExpr uregions = nl->Empty();
        for (unsigned int i = 0; i < faces.size(); i++) {
            Append(uregions, faces[i].ToListExpr());
        }
        Append(uregion, uregions);

        return uregion;
    } else {
        // We need to restrict the interval first
        MFaces f = divide(start, end);
        // and then recall this function with the new mfaces w/o restrictions.
        return f.ToListExpr(iv, 0, 1);
    }
}

// This is the duration of one moment, used for the borderregions
DateTime moment(durationtype, 0); // One moment has duration 0 here

/*
 1.7 ToMListExpr converts this MFaces-object to the NestedList-representation
 of an mregion suitable for import with InMRegion().
 It takes care of creating the correct intervals (evaporation, main and/or
 condensation), and the borderregions.
 
*/
ListExpr MFaces::ToMListExpr(Interval<Instant> iv) {
    ListExpr mreg = nl->Empty();
    
    // Determine if we need a start- or endregion. This is exactly the case when
    // one of the contained MFace-objects of this MFaces-object needs it.
    bool needStartRegion = false, needEndRegion = false;
    for (unsigned int i = 0; i < faces.size(); i++) {
        needStartRegion = needStartRegion || faces[i].needStartRegion;
        needEndRegion = needEndRegion || faces[i].needEndRegion;
    }

    // one-third of the whole interval is used for the evaporisation and/or
    // condensation-phase
    Instant onethird = (iv.end - iv.start) / 3;
    // Calculate the intervals
    Interval<Instant> evaporIv, condensIv;
    Interval<Instant> startRegIv, endRegIv;
    Interval<Instant> mainIv;

    mainIv.CopyFrom(iv);
    mainIv.lc = mainIv.rc = true;
    if (needSEvap) { // Create the evaporation interval
        // the left side of the evaporation interval is always open, since we
        // need a borderregion there
        evaporIv.lc = false;

        evaporIv.start = mainIv.start;
        mainIv.start = mainIv.start + onethird;
        evaporIv.end = mainIv.start;

        evaporIv.rc = false;
    }

    if (needDEvap) { // Create the condensation interval
        // the right side of the condensation interval is always open, since we
        // need a borderregion there
        condensIv.rc = false;

        condensIv.end = mainIv.end;
        mainIv.end = mainIv.end - onethird;
        condensIv.start = mainIv.end;

        condensIv.lc = false;
    }

    if (needStartRegion) { // Create the start-region interval
        startRegIv.lc = true;

        startRegIv.start = mainIv.start;
        mainIv.start = mainIv.start + moment;
        startRegIv.end = mainIv.start;

        startRegIv.rc = true;
        mainIv.lc = false;
    }

    if (needEndRegion) { // Create the end-region interval
        endRegIv.rc = true;

        endRegIv.end = mainIv.end;
        mainIv.end = mainIv.end - moment;
        endRegIv.start = mainIv.end;

        endRegIv.lc = true;
        mainIv.rc = false;
    }


    if (needSEvap) { // We need to perform an evaporations-phase
        cerr << "\n==== Evaporations ====\n";
        Interval<Instant> siv(evaporIv.start, evaporIv.start, true, true);
        MFaces s = Face::CreateMFaces(sregs);
        Append(mreg, s.ToListExpr(siv, 0, 1));
        MFaces fs;
        // Reconstruct the borderfaces at the start of the main interpolation ..
        vector<Face> borderfaces = CreateBorderFaces(true);
        // and interpolate them with the original sourceregion with
        // the evaporation-mode-flag set.
        fs = interpolate(sregs, &borderfaces, 0, true, "");
        Append(mreg, fs.ToListExpr(evaporIv, 0, 1));
        cerr << "==== /Evaporations ====\n";
    }

    if (needStartRegion) {
        cerr << "\n==== Start-Region ====\n";
        // Create the borderregion at the begin of the main interpolation
        // interval here and add it to the list.
        Append(mreg, CreateBorderMFaces(true).ToListExpr(startRegIv, 0, 1));
        cerr << "==== /Start-Region ====\n";
    }

    cerr << "\n==== Main-Interpolation to List ====\n";
    // Add the main-interpolation
    Append(mreg, ToListExpr(mainIv, 0, 1));
    cerr << "==== /Main-Interpolation to List ====\n";

    if (needEndRegion) {
        cerr << "\n==== End-Region ====\n";
        // Create the borderregion at the end of the main interpolation
        // interval here and add it to the list.
        Append(mreg, CreateBorderMFaces(false).ToListExpr(endRegIv, 0, 1));
        cerr << "==== /End-Region ====\n";
    }

    if (needDEvap) { // A condensation-phase is needed
        cerr << "\n==== Condensations ====\n";
        MFaces fs;
        // Reconstruct the borderfaces at the end of the main interpolation ..
        vector<Face> borderdregs = CreateBorderFaces(false);
        // and interpolate them with the original destinationregion with
        // the evaporation-mode-flag set.
        fs = interpolate(&borderdregs, dregs, 0, true, "");
        Append(mreg, fs.ToListExpr(condensIv, 0, 1));
        
        Interval<Instant> eiv(condensIv.end, condensIv.end, true, true);
        MFaces s = Face::CreateMFaces(dregs);
        Append(mreg, s.ToListExpr(eiv, 0, 1));
        cerr << "==== /Condensations ====\n";
    }
    
    cerr << "\n==== Result ====\n";
    nl->WriteListExpr(mreg);
    cerr << "\n";
    
    return mreg;
}



/*
 1.8 ToURegion converts this MFaces-object into a uregion using the given
 time-interval (honoring the restrictions ~start~ and ~end~). Only used by
 ToMRegion below.
 
*/
URegion MFaces::ToURegion(Interval<Instant> iv, double start, double end) {
    vector<URegion> uregs;

    if (start == 0 && end == 1) {
        // No restrictions, so just convert each face to a URegion and store it
        // in a list.
        for (unsigned int i = 0; i < faces.size(); i++) {
            URegion u = faces[i].ToURegion(iv, i);
            uregs.push_back(u);
        }
    } else {
        // Restrictions are in effect, so divide the MFaces accordingly ...
        MFaces f = divide(start, end);
        // .. and recall this function with the new MFaces w/o restrictions
        URegion ret = f.ToURegion(iv, 0, 1);

        return ret;
    }
    
    // Now merge the uregions to one uregion.
    URegion ret(uregs[0]);
    for (unsigned int i = 1; i < uregs.size(); i++) {
        ret.AddURegion(&uregs[i]);
    }

    return ret;
}

/*
 1.9 ToMRegion does the same as ToMListExpr, but constructs a native Secondo-
 mregion-object. Since the URegion-constructor is not fully sane at the time
 of this writing this method is not used at the moment.
 For the commented version (which only minimally differs), see 1.7 ToMListExpr

*/
MRegion MFaces::ToMRegion(Interval<Instant> iv) {
    MRegion ret(1);
    
    bool needStartRegion = false, needEndRegion = false;
    for (unsigned int i = 0; i < faces.size(); i++) {
        needStartRegion = needStartRegion || faces[i].needStartRegion;
        needEndRegion = needEndRegion || faces[i].needEndRegion;
    }

    Instant onethird = (iv.end - iv.start) / 3;
    Interval<Instant> startEvapIv, endEvapIv;
    Interval<Instant> startRegIv, endRegIv;
    Interval<Instant> mainIv;

    mainIv.CopyFrom(iv);
    mainIv.lc = mainIv.rc = true;
    if (needSEvap) {
        startEvapIv.lc = false;

        startEvapIv.start = mainIv.start;
        mainIv.start = mainIv.start + onethird;
        startEvapIv.end = mainIv.start;

        startEvapIv.rc = false;
    }

    if (needDEvap) {
        endEvapIv.rc = false;

        endEvapIv.end = mainIv.end;
        mainIv.end = mainIv.end - onethird;
        endEvapIv.start = mainIv.end;

        endEvapIv.lc = false;
    }

    if (needStartRegion) {
        startRegIv.lc = true;

        startRegIv.start = mainIv.start;
        mainIv.start = mainIv.start + moment;
        startRegIv.end = mainIv.start;

        startRegIv.rc = true;
        mainIv.lc = false;
    }

    if (needEndRegion) {
        endRegIv.rc = true;

        endRegIv.end = mainIv.end;
        mainIv.end = mainIv.end - moment;
        endRegIv.start = mainIv.end;

        endRegIv.lc = true;
        mainIv.rc = false;
    }


    if (needSEvap) {
        cerr << "\n==== Start-Evaporations ====\n";
        MFaces fs;
        cerr << "CreateBorderRegs start\n";
        vector<Face> bordersregs = CreateBorderFaces(true);
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
        vector<Face> borderdregs = CreateBorderFaces(false);
        fs = interpolate(&borderdregs, dregs, 0, true, "");
        URegion u = fs.ToURegion(endEvapIv, 0, 1);
        ret.AddURegion(u);
        cerr << "==== /End-Evaporations ====\n";
    }

    return ret;
}

/*
 1.10 fallback is used, when the construction of a valid mregion failed in the
 steps before. This can for example happen because of numerical instability and
 resulting errors in the intersection checks or because of bugs in the
 interpolation code.
 
 This function tries to create a mregion-ListExpression is such a simple manner,
 that no errors are to be expected in any case:
 
 * Divide the time-interval in two halves
 * The first half shows statically the source region
 * The second half shows statically the destination region
 
 If the source- and destination-faces are valid, this can only fail if the
 time-interval is too short. One of the intervals must be half-open and so has
 to have a minimum duration.

*/
ListExpr MFaces::fallback(vector<Face> *sfaces, vector<Face> *dfaces,
        Interval<Instant> iv) {
    ListExpr mreg = nl->Empty();
    
    Interval<Instant> startiv(iv.start, iv.start + (iv.end - iv.start) / 2,
            true, true);
    // Build the static MSegs of the source-faces
    MFaces start = Face::CreateMFaces(sfaces);
    Append(mreg, start.ToListExpr(startiv, 0, 1));
    
    // This interval is half-open on the left end, since the preceding interval
    // above is closed there
    Interval<Instant> endiv(startiv.end, iv.end, false, true);
    // Build the static MSegs of the destination-faces
    MFaces end = Face::CreateMFaces(dfaces);
    Append(mreg, end.ToListExpr(endiv, 0, 1));


    return mreg;
}

/*
 1.11 divide restricts the interval of the MFaces to the part specified by
 ~start~ and ~end~, which must be fractions between 0 and 1.
 divide(0.0, 0.5) would represent the first half of the whole interval.
 
*/
MFaces MFaces::divide(double start, double end) {
    MFaces ret;

    for (unsigned int i = 0; i < faces.size(); i++) {
        ret.AddMFace(faces[i].divide(start, end));
    }

    return ret;
}

/*
 1.12 ToString creates a textual representation of this MFaces-object for
 debugging purposes.
 
*/
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