/*
  1 Class MFace represents a Moving Face with optional holes

*/

#include "interpolate.h"

MFace::MFace() : needStartRegion(false), needEndRegion(false) {
}

/*
 1.1 Constructs a Moving Face from a set of Moving Segments
 
*/
MFace::MFace(MSegs face) : face(face),
        needStartRegion(false), needEndRegion(false)
{
    SortCycle(); // Sort the cycle
    EliminateSpikes(); // Eliminate empty spikes
    Check();
}

/*
 1.2 SortCycle

 Sort the Moving Segments according to their position in the cycle and perform
 several sanity-checks.
 
*/
bool MFace::SortCycle() {
    vector<MSeg> s1 = face.msegs;
    vector<MSeg> s2;
        
    if (face.msegs.size() < 3)
        return true;

    int cur = 0;
    unsigned int i = 0;
    bool ret = true;
    
    //Start with the first segment
    do {
        s2.push_back(s1[cur]);
        // Find the following segment
        cur = face.findNext(face.msegs[cur], cur, true);
        if (i++ > face.msegs.size()) {
            // If we have more iterations than the total number of msegments,
            // then we are stuck in an endless loop.
            ret = false;
            DEBUG(3, "Endless loop!");
            break;
        }
    } while (cur > 0); // We started with msegment 0, so stop here.
    
    if (cur == -1) { // We didn't find a following msegment, this is an error
        ret = false;
        DEBUG(3, "Error: cycle incomplete!");
    } else if (cur == -2) {
         // findNext returns -2 if two matching msegments were found
        ret = false;
        DEBUG(3, "Error: cycle ambiguous!");
    } else if (cur < 0) {
        // Should never happen
        ret = false;
        DEBUG(3, "Error: unknown error in cycle!");
    }

    if (s2.size() != face.msegs.size()) {
        // If the sorted cycle has less msegments than the original list,
        // then we didn't use all of them. This is an error.
        ret = false;
        DEBUG(3, "Unused segments! " << "has " << face.msegs.size() <<
                " used " << s2.size());
    } else {
        face.msegs = s2;
    }
    
    return ret;
}

/*
 1.2b SortAndSplitCycle

 Sort the Moving Segments according to their position in the cycle and perform
 several sanity-checks.
 
*/
vector<MFace> MFace::SortAndSplitCycle() {
    MSegs ms = face;
    vector<MFace> ret;
        
    DEBUG(4, "Called SortAndSplitCycle on " << this->ToString());
    
    do {
        bool found = false;
        for (unsigned int i = 0; i < ms.msegs.size(); i++) {
            int next = ms.findNext(ms.msegs[i], 0, true);
            if (next == -2) {
                // We have found the startpoint of an ambiguous cycle!
                MSeg cur = ms.msegs[i];
                MSegs cycle;

                DEBUG(4, "Start " << cur.ie.ToString() <<
                        " / " << cur.fe.ToString());
                next = ms.findNext(cur, 0, false);
                int c = 0;
                do {
                    if (next < 0 || c++ > ms.msegs.size())
                        return ret;
                    DEBUG(4, "Cycle: " << ms.msegs[next].ToString());
                    MSeg n = ms.msegs[next];
                    cycle.AddMSeg(n);
                    ms.msegs.erase(ms.msegs.begin() + next);
                    if (n.ie == cur.ie && n.fe == cur.fe)
                        break;
                    next = ms.findNext(n, 0, false);
                } while (1);
                MFace f = MFace(cycle);
                ret.push_back(f);
                found = true;
                break;
            }
        }
        if (found == false)
            break;
    } while (ms.msegs.size() > 0);
    
    if (ret.size() > 0) {
       face = ms;
       ms.calculateBBox();
    }
        
    return ret;
}

/*
 1.4 EliminateSpikes

 Try to find and eliminate empty spikes in the initial and/or final instant.
 
*/
void MFace::EliminateSpikes() {
    // Search spikes in the initial segments
    unsigned int i = 0, sz = face.msegs.size(), prev = 0;
    // Iterate twice over the segments to handle spikes at the wraparound, too
    while (i < 2*sz) {
        unsigned int cur = i%sz; // Access the arrays modulo arraysize
        if (face.msegs[cur].ie == face.msegs[prev].is) {
            // We have found an empty spike
            while (prev != cur) {
                // Degenerate initial segment to the startpoint of the spike
                face.msegs[prev].is = face.msegs[cur].ie;
                face.msegs[prev].ie = face.msegs[cur].ie;
                prev = (prev + 1)%sz;
            }
            face.msegs[cur].is = face.msegs[cur].ie; // also for current segment
        } else if (!(face.msegs[cur].is == face.msegs[cur].ie)) {
            // This was no spike, continue search from here
            prev = cur;
        }
        i++;
    }
    
    // Repeat the same procedure for the final segments
    i = 0; prev = 0;
    while (i < 2*sz) {
        unsigned int cur = i%sz;
        if (face.msegs[cur].fe == face.msegs[prev].fs) {
            while (prev != cur) {
                face.msegs[prev].fs = face.msegs[cur].fe;
                face.msegs[prev].fe = face.msegs[cur].fe;
                prev = (prev + 1)%sz;
            }
            face.msegs[cur].fs = face.msegs[cur].fe;
        } else if (!(face.msegs[cur].fs == face.msegs[cur].fe)) {
            prev = cur;
        }
        i++;
    }

    // Now eliminate MSeg-Objects with degenerated initial and final segments
    std::vector<MSeg>::iterator c = face.msegs.begin();
    while (c != face.msegs.end()) {
        if (c->is == c->ie && c->fs == c->fe)
            c = face.msegs.erase(c);
        else
            c++;
    }
}

/*
   1.4 Check
   Performs several sanity-checks on this object

*/

bool MFace::Check() {
    bool ret = true;
    
    if (isEmpty()) // an empty MFace is valid per definition
        return true;
    
    if (!SortCycle()) {
        ret = false;
    }
    
    
    for (unsigned int i = 0; i < holes.size(); i++) {
        MFace h1(holes[i]);
        for (unsigned int j = 0; j < holes.size(); j++) {
            if (holes[i].intersects(holes[j], false, false))
                ret = false;
        }
    }
    
    if (face.intersects(face, false, true)) {
        DEBUG(3, "Intersection!");
        ret = false;
    }

    if (!ret) {
        DEBUG(2, "Error with MFace " << ToString());
    }
    
    if (STRICT)
       assert(ret);
    else if (!ret)
        *this = MFace();

    return ret;
}

/*
   1.5 AddConcavity
 
   Add a new concavity or hole to this MFace-Object.
   The cycle will be integrated as a Concavity or hole when the function
   MergeConcavities is called afterwards.
 
*/
void MFace::AddConcavity(MSegs c) {
    if (c.msegs.size() >= 3) // Ignore invalid or degenerated cycles
        cvs.push_back(c);
}

static ListExpr CycleToListExpr(MSegs face);

/*
 1.5 MergeConcavities
 
 Merge the objects in the concavities-list into the current cycle.
 These will either be integrated into the cycle if possible, or otherwise be
 added as a hole.
 
*/
vector<MFace> MFace::MergeConcavities() {
    Check();
    for (unsigned int i = 0; i < cvs.size(); i++) {
       if (cvs[i].msegs.size() < 3) // Ignore invalid or degenerated faces
            continue;
        MFace f(cvs[i]);
        DEBUG(4, "Merging " << ToString() << " with " << f.ToString());
        if (face.MergeConcavity(cvs[i])) {
            DEBUG(4, "Success, result: " << ToString());
        } else {
            DEBUG(4, "Failed, adding as hole");
            // Merging the concavity into the cycle was not successful, add
            // this cycle to the list of holes.
            holes.push_back(cvs[i]);
            
            // If the hole was not a real hole but a concavity in the source- or
            // destination region, we have to create a start and/or end region.
            if (!cvs[i].sreg.ishole)
                needStartRegion = true;
            if (!cvs[i].dreg.ishole)
                needEndRegion = true;
        }
    }

    vector<MFace> split = SortAndSplitCycle();
    if (split.size() > 0) {
        DEBUG(3, "Cycle is ambiguous, splitted into ");
        DEBUG(3, this->ToString());
        for (unsigned int i = 0; i < split.size(); i++) {
            DEBUG(3, split[i].ToString());
        }
    }

    // Merging the concavity into the cycle was successful.
    SortCycle(); // Sort the cycle 
    EliminateSpikes(); // Eliminate empty spikes
    Check();
    
    // All concavities have been handled, clear the list.
    cvs.erase(cvs.begin(), cvs.end());
    
    return split;
}

/*
 1.6 Create a URegion-Object from this MFace
 
 As the time of this writing, the URegion-Constructor didn't work properly, so
 the resulting URegion would be broken.
 
*/
URegion MFace::ToURegion(Interval<Instant> iv, int facenr) {
    // Create the MSegmentData-objects from this cycle
    vector<MSegmentData> ms = face.ToMSegmentData(facenr, 0, 0);
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(facenr, i + 1,
                ms.size());
        ms.insert(ms.end(), h.begin(), h.end());
    }
    // Finally, construct the URegion
    URegion ret(ms, iv);

    return ret;
}

/*
 1.7 CycleToListExpr takes a cycle of MSegs and constructs a NestedList
 suitable to be integrated into the NestedList-Representation of a URegion
 or MRegion.
 
*/
static ListExpr CycleToListExpr(MSegs face) {
    ListExpr le, le2;
    int first, cur;
    first = cur = 0;
    
    assert(face.msegs.size() > 0);

    ListExpr c = nl->OneElemList(nl->RealAtom(face.msegs[cur].ie.x / SCALEOUT));
    le = nl->Append(c, nl->RealAtom(face.msegs[cur].ie.y / SCALEOUT));
    le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.x / SCALEOUT));
    le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.y / SCALEOUT));
    ListExpr cy = nl->OneElemList(c);
    le2 = cy;
    cur = face.findNext(face.msegs[cur], cur, true);
    while (cur != first) {
        assert(cur >= 0);
        c = nl->OneElemList(nl->RealAtom(face.msegs[cur].ie.x / SCALEOUT));
        le = nl->Append(c, nl->RealAtom(face.msegs[cur].ie.y / SCALEOUT));
        le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.x / SCALEOUT));
        le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.y / SCALEOUT));
        le2 = nl->Append(le2, c);
        cur = face.findNext(face.msegs[cur], cur, true);
    }

    return cy;
}

/*
 1.8 ToListExpr converts this face and its holes to a NestedList-Expression
 suitable to be embedded in a uregion- or mregion-NestedList
 
 
*/
ListExpr MFace::ToListExpr() {
    ListExpr ret = nl->OneElemList(CycleToListExpr(face));
    ListExpr le = ret;
    for (unsigned int i = 0; i < holes.size(); i++) {
        if (holes[i].msegs.size() < 0)
            continue;
        le = nl->Append(le, CycleToListExpr(holes[i]));
    }

    return ret;
}

/*
 1.9 PrintMRegionListExpr is used for debugging purposes and prints an
 OBJECT-representation of this MFace suitable to be loaded with
 "restore" into the database
 
*/
void MFace::PrintMRegionListExpr() {
    cerr << "(OBJECT mr () mregion ( ( "
            "(\"2013-01-01\" \"2013-01-02\" TRUE TRUE)("
            << nl->ToString(ToListExpr()) << ") ) ) )";
}

/*
 1.10 divide is used to create a MFace from this MFace over a part of the whole
 time-interval.
 For example: divide(0.0, 0.5) creates an MFace over the first half of the
 original time-interval.
 
*/
MFace MFace::divide(double start, double end) {
    MFace ret(face.divide(start, end));

    for (unsigned int i = 0; i < holes.size(); i++) {
        MSegs m = holes[i].divide(start, end);
        ret.AddConcavity(m);
    }
    ret.MergeConcavities();

    return ret;
}

/*
  1.11 ToString creates a textual representation of this face including its
  holes.

*/
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


/*
 1.12 CreateBorderFace is used to reconstruct a Face from this MFace.
 
 If the parameter ~src~ is TRUE, then the face is created from the initial
 segments of the MSegs (thus representing the state at the start of the
 time-interval), otherwise from the final segments.
 
*/
Face MFace::CreateBorderFace(bool src) {
    Check();

    Face ret = face.CreateBorderFace(src);

    for (unsigned int h = 0; h < holes.size(); h++) {
        Face hole = holes[h].CreateBorderFace(src);
        
        // AddHole also merges concavities if segments overlap
        ret.AddHole(hole);
    }

    return ret;
}

/*
 1.13 isEmpty returns true, if this MFace does not contain any moving segments

*/
bool MFace::isEmpty() {
    return face.msegs.size() == 0;
}