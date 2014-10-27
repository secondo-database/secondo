/*
   1 ~MFaces~ represents a set of Moving Faces (MFace-objects) in one
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
 1.2 ~AddFace~ adds an additional MFace to this MFaces-object
 
*/
void MFaces::AddMFace(MFace face) {
    if (!face.isEmpty())
        faces.push_back(face);
}

/*
 1.3 ~CreateBorderMFaces~ constructs a new MFaces-Object which 
 statically represents the situation at the start or end of the
 time-interval (depending on the parameter ~src~).
 
*/
MFaces MFaces::CreateBorderMFaces(bool src) {
    MFaces ret;
    
    vector<Face> fcs = CreateBorderFaces(src);

    for (unsigned int i = 0; i < fcs.size(); i++) {
        MFace f = fcs[i].GetMFace(true);
        if (!f.isEmpty())
            ret.AddMFace(f);
    }

    return ret;
}

/*
 1.4 ~CreateBorderFaces~ creates a list of Faces representing the region at
 the start (or the end, depending on the parameter ~src~) of the time
 interval.
 
*/
vector<Face> MFaces::CreateBorderFaces(bool src) {
    vector<Face> ret;

    for (unsigned int i = 0; i < faces.size(); i++) {
        if ((faces[i].half == 1 && !src) || 
            (faces[i].half == 2 && src))
            continue;
        Face f = faces[i].CreateBorderFace(src);
        if (!f.isEmpty()) {
            bool merged = false;
            for (unsigned int j = 0; j < ret.size(); j++) {
                merged = ret[j].Merge(f);
                if (merged)
                    break;
            }
            if (!merged)
                ret.push_back(f);
        }
    }

    return ret;
}

/*
 1.5 ~ToListExpr~ converts this MFaces-object to a RList-representation
 of a moving region over the given interval with the given restrictions.
 
*/
RList MFaces::ToListExpr(Interval iv, double start, double end, int half) {
    RList ret;
    
    if (faces.empty())
        return ret;

    if (start == 0 && end == 1) {
        // No restrictions, so we can start the real work
        
        // Build the ListExpr for the time-interval
        RList uregion, uregions;
        
        RList interval;
        interval.append(iv.startstr());
        interval.append(iv.endstr());
        interval.append(iv.lc);
        interval.append(iv.rc);
        
        uregion.append(interval);

        // Now convert the mface-objects one by one and put them into one list
        for (unsigned int i = 0; i < faces.size(); i++) {
            if ((half == 0) || (half == faces[i].half))
                uregions.append(faces[i].ToListExpr());
            if ((half != 0) && faces[i].half == 0) {
                assert(faces[i].face.msegs.size() >= 3);
                if (half == 1)
                    uregions.append(faces[i].divide(0, 0.5).ToListExpr());
                else if (half == 2) {
                    cerr << "Before: " << faces[i].ToString() << "\n";
                    cerr << "Now:    " << faces[i].divide(0.5,1).ToString()
                         << "\n";
                    uregions.append(faces[i].divide(0.5, 1).ToListExpr());
                }
            }
        }
        uregion.append(uregions);

        return uregion;
    } else {
        // We need to restrict the interval first
        MFaces f = divide(start, end);
        // and then recall this function with the new mfaces w/o restrictions.
        return f.ToListExpr(iv, 0, 1, 0);
    }
}

/*
 1.6 ~ToMListExpr~ converts this MFaces-object to the RList
 representation of an moving region suitable for import into a dbms.
 It takes care of creating the correct intervals (evaporation, main and/or
 condensation), and the borderregions.
 
*/
RList MFaces::ToMListExpr(Interval iv) {
    RList mreg;
    
    // Determine if we need a start- or endregion. This is exactly the case when
    // one of the contained MFace-objects of this MFaces-object needs it.
    bool needStartRegion = false, needEndRegion = false;
    for (unsigned int i = 0; i < faces.size(); i++) {
        cerr << "FC: " << faces[i].ToString() << "\n";
        needStartRegion = needStartRegion || faces[i].needStartRegion;
        needEndRegion = needEndRegion || faces[i].needEndRegion;
    }

    // one-third of the whole interval is used for the evaporisation and/or
    // condensation-phase
    double onethird = (iv.end - iv.start) / 3;
    // Calculate the intervals
    Interval evaporIv, condensIv;
    Interval startRegIv, endRegIv;
    Interval mainIv;

    mainIv = iv;
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
        mainIv.start = mainIv.start + MOMENTMS;
        startRegIv.end = mainIv.start;

        startRegIv.rc = true;
        mainIv.lc = false;
    }

    if (needEndRegion) { // Create the end-region interval
        endRegIv.rc = true;

        endRegIv.end = mainIv.end;
        mainIv.end = mainIv.end - MOMENTMS;
        endRegIv.start = mainIv.end;

        endRegIv.lc = true;
        mainIv.rc = false;
    }


    if (needSEvap) { // We need to perform an evaporations-phase
        Interval siv(evaporIv.start, evaporIv.start, true, true);
        MFaces s = Face::CreateMFaces(sregs);
        mreg.append(s.ToListExpr(siv, 0, 1, 0));
        MFaces fs;
        // Reconstruct the borderfaces at the start of the main interpolation ..
        vector<Face> borderfaces = CreateBorderFaces(true);
        // and interpolate them with the original sourceregion with
        // the evaporation-mode-flag set.
        fs = interpolate(sregs, &borderfaces, 0, true, "");
        mreg.append(fs.ToListExpr(evaporIv, 0, 1, 0));
    }

    if (needStartRegion) {
        // Create the borderregion at the begin of the main interpolation
        // interval here and add it to the list.
        mreg.append(CreateBorderMFaces(true).ToListExpr(startRegIv, 0, 1, 0));
    }

    // Add the main-interpolation
    bool needHalves = false;
    for (unsigned int i = 0; i < faces.size(); i++) {
        if (faces[i].half != 0)
            needHalves = true;
    }
    if (!needHalves) {
        mreg.append(ToListExpr(mainIv, 0, 1, 0));
    } else {
        Interval iv1 = mainIv;
        Interval iv2 = mainIv;
        iv1.end = (iv1.start + iv1.end)/2;
        iv2.start = iv1.end;
        iv1.rc = true;
        iv2.lc = false;
        mreg.append(ToListExpr(iv1, 0, 1, 1));
        mreg.append(ToListExpr(iv2, 0, 1, 2));
    }

    if (needEndRegion) {
        // Create the borderregion at the end of the main interpolation
        // interval here and add it to the list.
        mreg.append(CreateBorderMFaces(false).ToListExpr(endRegIv, 0, 1, 0));
    }

    if (needDEvap) { // A condensation-phase is needed
        MFaces fs;
        // Reconstruct the borderfaces at the end of the main interpolation ..
        vector<Face> borderdregs = CreateBorderFaces(false);
        // and interpolate them with the original destinationregion with
        // the evaporation-mode-flag set.
        fs = interpolate(&borderdregs, dregs, 0, true, "");
        mreg.append(fs.ToListExpr(condensIv, 0, 1, 0));
        
        Interval eiv(condensIv.end, condensIv.end, true, true);
        MFaces s = Face::CreateMFaces(dregs);
        mreg.append(s.ToListExpr(eiv, 0, 1, 0));
    }
        
    return mreg;
}

/*
 1.7 ~fallback~ can used, when the construction of a valid moving region failed
 in the steps before. This can for example happen because of numerical
 instability and resulting errors in the intersection checks or because of bugs
 in the interpolation code.
 
 This function tries to create a moving region RList representation in
 such a simple manner, that no errors are to be expected in any case:
 
 * Divide the time-interval in two halves
 * The first half shows statically the source region
 * The second half shows statically the destination region
 
 If the source- and destination-faces are valid, this can only fail if the
 time interval is too short. One of the intervals must be half-open and so has
 to have a certain minimum duration.

*/
RList MFaces::fallback(vector<Face> *sfaces, vector<Face> *dfaces,
        Interval iv) {
    RList mreg;
    
    Interval startiv(iv.start, iv.start + (iv.end - iv.start) / 2, true, true);
    // Build the static MSegs of the source-faces
    MFaces start = Face::CreateMFaces(sfaces);
    mreg.append(start.ToListExpr(startiv, 0, 1, 0));
    
    // This interval is half-open on the left end, since the preceding interval
    // above is closed there
    Interval endiv(startiv.end, iv.end, false, true);
    // Build the static MSegs of the destination-faces
    MFaces end = Face::CreateMFaces(dfaces);
    mreg.append(end.ToListExpr(endiv, 0, 1, 0));

    return mreg;
}

/*
 1.8 ~divide~ restricts the interval of the MFaces to the part specified by
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
 1.9 ~ToString~ creates a textual representation of this MFaces-object for
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
