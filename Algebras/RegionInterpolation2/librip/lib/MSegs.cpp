/*
   1 Class MSegs represents a set of MSeg-objects, usually a whole cycle.
 
*/

#include "interpolate.h"

MSegs::MSegs() : iscollapsed(0), isevaporating(0) {
}

/*
  1.1 ~AddMSeg~ adds a new MSeg to this set. If the current segment is
  collinear with the previously added MSeg, then the two MSeg-objects are
  merged into one. After that, the bounding box is updated.

*/
void MSegs::AddMSeg(MSeg m) {
    // The MSeg is degenerated in both instants, so we ignore it.
    if ((m.is == m.ie) && (m.fs == m.fe))
        return;

    bool merged = false;
    if (!msegs.empty()) {
       // If the list of segments is not empty, try to merge the segment with
       // the previously added MSeg
        MSeg *prev = &msegs[msegs.size() - 1];
        merged = prev->Merge(m); // Returns true, if merge was successful
    }
    if (!merged)
        msegs.push_back(m);
    
    // The Bounding-Box may have changed, update it.
    calculateBBox();
}

/*
  1.2 ~MergeConcavity~ is responsible to either merge two cycles into one
  cycle, or, if that is not possible, add the cycle to the list of holes.
 
*/
bool MSegs::MergeConcavity(MSegs c) {
    bool fastPath = true;
    bool success = false;
    
    // Determine if a fast path can be used. This is possible if no
    // MSeg-object was merged, so the pointlists only include the endpoints of
    // the MSeg, which is degenerated in one point.
    for (unsigned int i = 0; i < msegs.size(); i++) {
        if ((msegs[i].ip.size() + msegs[i].fp.size()) > 3) {
            fastPath = false;
            break;
        }
    }
    
    // If the fast-path can be used, then we can find identical MSeg-objects
    // by sorting both lists and comparing them step-by-step.
    if (fastPath) {
        std::sort(msegs.begin(), msegs.end());
        std::sort(c.msegs.begin(), c.msegs.end());

        std::vector<MSeg>::iterator i = msegs.begin();
        std::vector<MSeg>::iterator j = c.msegs.begin();
        while (i != msegs.end() && j != c.msegs.end()) {
            if (*i == *j) {
                // We found an identical MSeg, erase it from both lists.
                i = msegs.erase(i);
                j = c.msegs.erase(j);
                // and tell the world, that we succeeded, but still search for
                // more matches.
                success = true;
            } else if (*i < *j) {
                i++;
            } else {
                j++;
            }
        }
    } else {
        // We cannot use the Fast path, so we have to try to integrate each
        // segment of the parent with each segment of the cycle to merge
        std::vector<MSeg>::iterator i = msegs.begin();
        while (i != msegs.end()) {
            std::vector<MSeg>::iterator j = c.msegs.begin();
            bool integrated = false;
            while (j != c.msegs.end()) {
                MSeg m1, m2;
                // Try to integrate the MSeg of the (possible) concavity into
                // the parents MSeg. This leads to a split of the parent into
                // m1 and m2, which must be added to the cycle, too.
                if ((integrated = i->Split(*j, m1, m2))) {
                    // Integration was successful, so erase both segments
                    i = msegs.erase(i);
                    j = c.msegs.erase(j);
                    // and add the rest of the parent segment. These may be
                    // degenerated, AddMSeg will ignore them in this case
                    AddMSeg(m1);
                    AddMSeg(m2);
                    // We had success merging a concavity
                    success = true;
                    break;
                }
                j++;
            }
            if (!integrated) // If we integrated a segment we do not need to
                i++;         // advance, since we erased this segment
            else
                i = msegs.begin(); // Since we modified the iterators, reset
        }
    }
    
    if (success) {
        // Both cycles were oriented counterclockwise, to integrate the cycle
        // we therefore have to change the orientation of the MSeg-objects
        for (unsigned int x = 0; x < c.msegs.size(); x++)
            c.msegs[x].ChangeDirection();

        msegs.insert(msegs.end(), c.msegs.begin(), c.msegs.end());
    }
    
    return success;
}

/*
  1.3 ~ToString~ creates a string-representation of this object for debugging
  purposes.
 
*/
string MSegs::ToString() const {
    std::ostringstream ss;

    ss << "MSegs (\n";
    for (unsigned int i = 0; i < msegs.size(); i++) {
        ss << " " << msegs[i].ToString() << "\n";
    }
    ss << ")\n";
    return ss.str();
}

/*
 1.4 ~Check~ if the MSeg-objects of two MSegs intersect
 
 If matchIdent is set, then also complain if two MSeg-objects are identical.
 If matchSegs is set, the initial and final segments are also checked for
 overlap.
 
*/
bool MSegs::intersects(const MSegs& a, bool matchIdent, bool matchSegs) const {

    // Fast path if the bounding-boxes are disjoint
    if ((bbox.first.x > a.bbox.second.x) ||
        (a.bbox.first.x > bbox.second.x) ||
        (bbox.first.y > a.bbox.second.y) ||
        (a.bbox.first.y > bbox.second.y))
            return false;


    // Otherwise check each segment-pair
    bool ret = false;
    for (unsigned int i = 0; i < a.msegs.size(); i++) {
        for (unsigned int j = 0; j < msegs.size(); j++) {
            if ((matchIdent || !(msegs[j] == a.msegs[i])) &&
                    msegs[j].intersects(a.msegs[i], matchSegs)) {
                ret = true;
                return ret;
            }
        }
    }

    return ret;
}

/* 
   1.5 ~CreateBorderFace~ returns a face which represents the initial
   or final face of this MSegs (depending on the parameter ~initial~)
 
   A moving segment can consist of several points if collinear MSeg-objects were
   merged into one. Reconstruct the original segments from the list of
   MSeg-objects here to be able to merge concavities properly.

*/
Face MSegs::CreateBorderFace(bool initial) {
    vector<Seg> ret;
    
    // Use the original parent-face
    Face f = initial ? sreg : dreg;
    
    // but construct the segments new, since this may have changed due to
    // intersection problems.
    for (unsigned int i = 0; i < msegs.size(); i++) {
        vector<Pt> points = initial ? msegs[i].ip : msegs[i].fp;
        for (unsigned int j = 0; j < points.size() - 1; j++) {
            ret.push_back(Seg(points[j], points[j+1]));
        }
    }
    f.holes.clear();
    f.v = ret;
    f.Sort();
    
    return f;
}


/*
 1.6 ~kill~ is used to break up the connection between two faces and create two
 MSegs, one for collapsing the source-face and one for expanding the
 destination-face. This is for example used in handleIntersections, when the
 original MSegs-object intersects with other objects, so this object is
 eliminated and replaced by the two objects created here.
 
*/
pair<MSegs, MSegs> MSegs::kill() {
    MSegs src = CreateBorderFace(true).collapse(true);
    MSegs dst = CreateBorderFace(false).collapse(false);
    
    return pair<MSegs, MSegs>(src, dst);
}

/*
 1.7 ~divide~ is used to restrict the time-interval of the MSeg-objects of
 this cycle to the given boundaries.
 Example: divide(0.0, 0.5) would restrict to the first half of the whole
 interval.
 
*/
MSegs MSegs::divide(double start, double end) {
    MSegs ret;

    ret.sreg = sreg;
    ret.dreg = dreg;

    for (unsigned int i = 0; i < msegs.size(); i++) {
        MSeg m = msegs[i].divide(start, end);
        // Ignore degenerated moving segments. This can happen if the interval
        // is restricted to the start- or endpoint.
        if (m.is == m.ie && m.fs == m.fe)
            continue;
        ret.AddMSeg(m);
    }

    return ret;
}

/*
 1.8 ~findNext~ tries to find the index of the next matching MSeg for the MSeg
 ~cur~. It also checks, if there are two or more successors (if check is true),
 which should not happen, since the cycle is ambiguous then.
 
*/
int MSegs::findNext(MSeg cur, int start, bool check) {
    unsigned int nrsegs = msegs.size();
    int ret = -1;

    for (unsigned int i = 0; i < nrsegs; i++) {
        int nindex = (i + start) % nrsegs;
        MSeg *next = &msegs[nindex];
        // We have found a successor, if the initial and final start-points
        // match the initial and final endpoints of the current MSeg.
        if (cur.ie == next->is && cur.fe == next->fs) {
            if (ret == -1) {
                ret = nindex;
            } else {
                // We had already found a successor, so this is bad.
                DEBUG(2, " Found 2 successors for " << cur.ToString() << endl <<
                        msegs[ret].ToString() << " AND\n" <<
                        msegs[nindex].ToString() << "\n");
                ret = -2;

                break;
            }
            
            if (!check)
                break;
            // We have found a candidate, but since "check" is true, we search
            // on to see, if a second object matches too.
        }
    }

    return ret;
}

int MSegs::findNext(int index, bool check) {
    return findNext(msegs[index], index, check);
}

// helper-function: Find the maximum of four double-values
static inline double max(double a, double b, double c, double d) {
    double m = a;

    if (b > m)
        m = b;
    if (c > m)
        m = c;
    if (d > m)
        m = d;

    return m;
}

// helper-function: Find the minimum of four double-values
static inline double min(double a, double b, double c, double d) {
    double m = a;

    if (b < m)
        m = b;
    if (c < m)
        m = c;
    if (d < m)
        m = d;

    return m;
}

/*
 1.9 ~calculateBBox~ determines the bounding-box of the current cycle of
 MSegs.
 
*/
pair<Pt, Pt> MSegs::calculateBBox() {
    if (msegs.empty()) {
        // This usually does not happen, but return something sane anyway.
        bbox = pair<Pt, Pt>(Pt(0, 0), Pt(0, 0));
    } else {
        bbox.first.valid = bbox.second.valid = 0;
        // Enlarge the current bounding-box MSeg by MSeg.
        for (unsigned int i = 0; i < msegs.size(); i++) {
            updateBBox(msegs[i]);
        }
    }

    return bbox;
}

/*
 1.10 ~updateBBox~ checks, if the current bounding-box changes when the
 segment ~seg~ is added and updates it accordingly.
 
*/
void MSegs::updateBBox(MSeg& mseg) {
    Pt p1(min(mseg.is.x, mseg.ie.x,
            mseg.fs.x, mseg.fe.x),
            min(mseg.is.y, mseg.ie.y,
            mseg.fs.y, mseg.fe.y)
            );
    Pt p2(max(mseg.is.x, mseg.ie.x,
            mseg.fs.x, mseg.fe.x),
            max(mseg.is.y, mseg.ie.y,
            mseg.fs.y, mseg.fe.y)
            );
    if (!bbox.first.valid || (bbox.first.x > p1.x))
        bbox.first.x = p1.x;
    if (!bbox.first.valid || (bbox.first.y > p1.y))
        bbox.first.y = p1.y;
    if (!bbox.second.valid || (bbox.second.x < p2.x))
        bbox.second.x = p2.x;
    if (!bbox.second.valid || (bbox.second.y < p2.y))
        bbox.second.y = p2.y;
    bbox.first.valid = 1;
    bbox.second.valid = 1;
}
