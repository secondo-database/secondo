/*
  1 Class ~MFace~ represents a Moving Face with optional holes

*/

#include "interpolate.h"

using namespace std;

MFace::MFace() : needStartRegion(false), needEndRegion(false) {
}

/*
 1.1 Constructs a Moving Face from a set of Moving Segments
 
*/
MFace::MFace(MSegs face) : face(face),
        needStartRegion(false), needEndRegion(false) {
}

/*
 1.2 ~SortCycle~

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
 1.3 ~SplitCycle~

 Sort the Moving Segments according to their position in the cycle. Split
 the cycle, if two independent cycles can be built from this cycle.
 
*/
vector<MFace> MFace::SplitCycle() {
    MSegs ms = face;
    vector<MFace> ret;
        
    DEBUG(4, "Called SplitCycle on " << this->ToString());
    
    do {
        MSegs cycle;
        MSeg n = ms.msegs[0];
        MSeg cur = n;
        cycle.AddMSeg(ms.msegs[0]);
        ms.msegs.erase(ms.msegs.begin());
        bool complete = false;
        do {
            int next = ms.findNexta(cur, 1);
            if (next < 0) {
                DEBUG(4, "Error: no successor found!");
                break;
            }
            cur = ms.msegs[next];
            cycle.AddMSeg(cur);
            ms.msegs.erase(ms.msegs.begin() + next);
            if (n.is == cur.ie && n.fs == cur.fe) {
                complete = true;
                break;
            }
        } while (1);
        if (complete) {
            DEBUG(4, "Cycle complete, " << cycle.msegs.size() << " segs");
            MFace f = MFace(cycle);
            f.face.EliminateSpikes();
            f.Check();
            f.face.calculateBBox();
            if (f.face.msegs.size()>0)
                ret.push_back(f);
        }
    } while (ms.msegs.size() > 0);

    if (ret.size() == 1) {
        ret[0].face.sreg = face.sreg;
        ret[0].face.dreg = face.dreg;
        ret[0].holes = holes;
    }

    DEBUG(4, "SplitCycle done, now " << ret.size() << " cycles");
    return ret;
}

/*
   1.5 ~Check~
   Performs several sanity-checks on this object

*/
bool MFace::Check() {
    bool ret = true;

    if (isEmpty()) // an empty MFace is valid per definition
        return true;

    if (!SortCycle()) {
        DEBUG(3, "SortCycle failed!");
        ret = false;
    }


    for (unsigned int i = 0; i < holes.size(); i++) {
        MFace h1(holes[i]);
        for (unsigned int j = 0; j < holes.size(); j++) {
            if (holes[i].intersects(holes[j], false, false)) {
                DEBUG(3, "Hole intersection!");
                ret = false;
            }
        }
    }

    if (face.intersects(face, false, true)) {
        DEBUG(3, "Self Intersection!");
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
   1.6 ~AddConcavity~

   Add a new concavity or hole to this MFace-Object.
   The cycle will be integrated as a Concavity or hole when the function
   MergeConcavities is called afterwards.

*/
void MFace::AddConcavity(MFace c) {
    if (c.face.msegs.size() >= 3) // Ignore invalid or degenerated cycles
        cvs.push_back(c);
}

/*
   1.7 ~MergeConcavities~

 Merge the objects in the concavities-list into the current cycle.
 These will either be integrated into the cycle if possible, or otherwise be
 added as a hole.
 
*/
vector<MFace> MFace::MergeConcavities() {
    Check();
    for (unsigned int i = 0; i < cvs.size(); i++) {
        if (cvs[i].face.msegs.size() < 3) // Ignore invalid or degenerated faces
            continue;
        
        // Inherit the need of a start or endregion
        needStartRegion |= cvs[i].needStartRegion;
        needEndRegion |= cvs[i].needEndRegion;
        DEBUG(4, "Merging " << ToString() << " with " << cvs[i].ToString());
        if (face.MergeConcavity(cvs[i].face)) {
            DEBUG(4, "Success, result: " << ToString());
        } else {
            DEBUG(4, "Failed, adding as hole");
            // Merging the concavity into the cycle was not successful, add
            // this cycle to the list of holes.
            holes.push_back(cvs[i].face);

            // If the hole was not a real hole but a concavity in the source- or
            // destination region, we have to create a start and/or end region.
            if (!cvs[i].face.sreg.ishole)
                needStartRegion = true;
            if (!cvs[i].face.dreg.ishole)
                needEndRegion = true;
        }
    }

    // All concavities have been handled, clear the list.
    cvs.erase(cvs.begin(), cvs.end());

    // Check and see, if this cycle was split into several cycles due to the
    // merge. 
    vector<MFace> split = SplitCycle();
    if (split.size() > 1) {
        DEBUG(3, "Cycle is ambiguous, splitted into ");
        for (unsigned int i = 0; i < split.size(); i++) {
            DEBUG(3, split[i].ToString());
        }
    }

    return split;
}

/*
 1.8 ~CycleToListExpr~ takes a cycle of MSegs and constructs a RList
 suitable to be integrated into the RList-Representation of a
 moving region.
 
*/
static RList CycleToListExpr(MSegs face) {
    int first = 0, cur = 0;
        
    assert(face.msegs.size() > 0);
    
    RList cy, empty;
    do {
        if (cur < 0)
            return empty;
        assert(cur >= 0);
        RList mseg;
        mseg.append(face.msegs[cur].ie.x / SCALEOUT);
        mseg.append(face.msegs[cur].ie.y / SCALEOUT);
        mseg.append(face.msegs[cur].fe.x / SCALEOUT);
        mseg.append(face.msegs[cur].fe.y / SCALEOUT);
        cy.append(mseg);
        cur = face.findNext(face.msegs[cur], cur, true);
    } while (cur != first);
    
    return cy;
}

/*
 1.9 ~ToListExpr~ converts this face and its holes to a RList-Expression
 suitable to be embedded into the RList representation of a moving
 region.
 
*/
RList MFace::ToListExpr() {
    RList ret;
    
    RList f = CycleToListExpr(face);
    if (f.empty())
        return ret;
    ret.append(f);
    
    for (unsigned int i = 0; i < holes.size(); i++) {
        if (holes[i].msegs.size() < 3)
            continue;
        RList h = CycleToListExpr(holes[i]);
        if (!h.empty())
            ret.append(h);
    }

    return ret;
}

/*
 1.10 ~PrintMRegionListExpr~ is used for debugging purposes and prints an
 OBJECT-representation of this MFace suitable to be loaded with
 "restore" into the Secondo database system
 
*/
void MFace::PrintMRegionListExpr() {
    DEBUG(1, "(OBJECT mr () mregion ( ( "
            "(\"2013-01-01\" \"2013-01-02\" TRUE TRUE)("
            << ToListExpr().ToString() << ") ) ) )");
}

/*
 1.11 ~divide~ is used to create a MFace from this MFace over a part of the whole
 time-interval.
 For example: divide(0.0, 0.5) creates an MFace over the first half of the
 original time interval.
 
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
  1.11 ~ToString~ creates a textual representation of this face including its
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
 1.12 ~CreateBorderFace~ is used to reconstruct a Face from this MFace.
 
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
 1.13 ~isEmpty~ returns true, if this MFace does not contain any moving segments

*/
bool MFace::isEmpty() {
    return face.msegs.size() == 0;
}
