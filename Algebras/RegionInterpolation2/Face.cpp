/*
  1 Class Face
  
  This class represents a face with an optional set of holes.
  
*/

/* scale-factor for ~region~-import and export */
#define SCALE 1

#include "interpolate.h"

Face::Face() : cur(0), ishole(false) {
}

/*
  1.1 Constructs a face from a region-listexpression

*/
Face::Face(ListExpr tle) : cur(0), parent(NULL), ishole(false) {
    ListExpr le = nl->First(tle);
    while (nl->ListLength(le) > 1) {
        ListExpr pa = nl->First(le);
        ListExpr pb = nl->First(nl->Rest(le));
        double p1 = nl->RealValue(nl->First(pa)) * SCALE;
        pa = nl->Rest(pa);
        double p2 = nl->RealValue(nl->First(pa)) * SCALE;
        pa = nl->Rest(pa);
        double p3 = nl->RealValue(nl->First(pb)) * SCALE;
        pb = nl->Rest(pb);
        double p4 = nl->RealValue(nl->First(pb)) * SCALE;
        pb = nl->Rest(pb);
        le = nl->Rest(le);
        Seg s = Seg(Pt(p1, p2), Pt(p3, p4));
        AddSeg(s);
    }
    while (nl->ListLength(tle) > 1) {
        tle = nl->Rest(tle);
        Face hole(tle);
        hole.ishole = true;
        holes.push_back(hole);
    }
    Close();
    Check();
}

/*
  1.2 Constructs a face from a set of segments.

*/
Face::Face(vector<Seg> v) : cur(0), parent(NULL), v(v), ishole(false) {
    Sort();
    Check();
}

// Helper-function to append an item to a nested-list

static void Append(ListExpr &head, ListExpr l) {
    if (l == nl->Empty())
        return;
    if (head == nl->Empty()) {
        head = nl->OneElemList(l);
    } else {
        nl->Append(nl->End(head), l);
    }
}

// Helper-Function to get the NestedList-representation of a list of segments
static ListExpr Segs2NL(vector<Seg> segs, double offx, double offy,
        double scalex, double scaley) {
    ListExpr cycle = nl->Empty();

    for (unsigned int i = 0; i < segs.size(); i++) {
        ListExpr seg = nl->Empty();
        Append(seg, nl->RealAtom((segs[i].s.x - offx) * scalex / SCALE));
        Append(seg, nl->RealAtom((segs[i].s.y - offy) * scaley / SCALE));
        Append(cycle, seg);
    }

    return cycle;
}

/*
  1.3 Convert this face to a region with the given offsets and scale-factors.
  Also convert holes, if the parameter ~withholes~ is true

*/
Region Face::MakeRegion(double offx, double offy, double scalex, double scaley,
        bool withholes)
{
    ListExpr cycle = Segs2NL(v, offx, offy, scalex, scaley);
    ListExpr cycles = nl->OneElemList(cycle);

    if (withholes) {
        for (unsigned int h = 0; h < holes.size(); h++) {
            ListExpr hcycle = Segs2NL(holes[h].v, offx, offy, scalex, scaley);
            Append(cycles, hcycle);
        }
    }
    
    ListExpr face = nl->OneElemList(cycles);

    ListExpr err = nl->Empty();
    bool correct = false;
    Word w = InRegion(nl->Empty(), face, 0, err, correct);
    if (correct) {
        Region ret(*((Region*) w.addr));
        return ret;
    } else {
        return Region();
    }
}

/*
  1.4 Convert this face to a region

*/
Region Face::MakeRegion(bool withholes) {
    return MakeRegion(0, 0, 1, 1, withholes);
}

/*
  1.5 Sort the segments of this Face.

*/
void Face::Sort() {
    v = sortSegs(v);
}

/* 
  1.6 sortSegs sorts a list of segments to be in the correct order of a cycle.
  A cycle should begin with the lowest-(leftest)-point and then go counter-
  clockwise.

*/
vector<Seg> Face::sortSegs(vector<Seg> v) {
    vector<Seg> ret;

    if (v.size() == 0)
        return ret;

    int start = -1, start2 = -1;
    double minx = 0, miny = 0;
    Seg minseg1, minseg2;


    // Find the segment with the lowest start-point
    for (unsigned int i = 0; i < v.size(); i++) {
        if ((v[i].s.y < miny) || ((v[i].s.y == miny) &&
                (v[i].s.x < minx)) || (start < 0)) {
            start = i;
            miny = v[i].s.y;
            minx = v[i].s.x;
            minseg1 = v[i];
        }
    }

    /* Find the corresponding segment with the lowest
     * end-point and change its direction to compare the angle*/
    for (unsigned int i = 0; i < v.size(); i++) {
        if ((v[i].e.x == minx) && (v[i].e.y == miny)) {
            start2 = i;
            minseg2 = v[i];
        }
    }
    minseg2.ChangeDir();

    /* If the angle of the segment with the lowest end-point is less than
     * the segment with the lowest start-point, then the segments are oriented
     * clockwise at the moment, so we have to change their direction */
    if (minseg2.angle() < minseg1.angle()) {
        for (unsigned int i = 0; i < v.size(); i++) {
            v[i].ChangeDir();
        }
        start = start2;
    }

    /* Now go and seek the next segment for each segment and put it ordered
     * into a list */
    Seg cur = v[start];
    Seg startseg = cur;
    ret.push_back(cur);
    while (1) {
        bool found = false;
        for (unsigned int i = 0; i < v.size(); i++) {
            if (v[i].s == cur.e) {
                cur = v[i];
                ret.push_back(cur);
                found = true;
                if (cur.e == startseg.s)
                    break;
            }
        }
        assert(found); // This should never happen on a complete cycle.
        /* If the endpoint of this segment is the startpoint of the first, we
         * are done.                                                         */
        if (cur.e == startseg.s)
            break;
    }

    return ret;
}

/*
 1.7 AddSeg adds a segment to this face and updates the bounding-box
 accordingly.
 
*/
void Face::AddSeg(Seg a) {
    double minx = (a.s.x > a.e.x) ? a.e.x : a.s.x;
    double maxx = (a.s.x > a.e.x) ? a.s.x : a.e.x;
    double miny = (a.s.y > a.e.y) ? a.e.y : a.s.y;
    double maxy = (a.s.y > a.e.y) ? a.s.y : a.e.y;
    
    if (v.size() == 0) {
        bbox.first.x = minx;
        bbox.second.x = maxx;
        bbox.first.y = miny;
        bbox.second.y = maxy;
    } else {
        if (minx < bbox.first.x)
            bbox.first.x = minx;
        if (maxx > bbox.second.x)
            bbox.second.x = maxx;
        if (miny < bbox.first.y)
            bbox.first.y = miny;
        if (maxy > bbox.second.y)
            bbox.second.y = maxy;
    }
    v.push_back(a);
}

/*
 1.8 Close completes the cycle if this is not already the case and precalculates
 the convex hull.
 
*/
void Face::Close() {
    if (!v.size())
        return;
    int i = v.size() - 1;
    if (!(v[i].e == v[0].s)) {
        // The cycle is not closed yet, we do that now.
        Seg s(v[i].e, v[0].s);
        AddSeg(s);
    }

    ConvexHull();
    Check();
}

/*
 1.8 getPoints returns a vector with all points of the current cycle.
 
*/
vector<Pt> Face::getPoints() {
    vector<Pt> points;

    for (unsigned int i = 0; i < v.size(); i++) {
        points.push_back(v[i].s);
    }

    return points;
}

/*
 1.9 leftOf determines if the point ~next~ is left of the segment (pt1 pt2).
 It also returns true if it is on the line.
 
*/
static bool leftOf(Pt pt1, Pt pt2, Pt next) {
    long double sign = ((pt2.x - pt1.x)*(next.y - pt1.y)
            -(next.x - pt1.x)*(pt2.y - pt1.y));
    return sign >= 0;
}

// helper-function to sort the points by angle. If the angle is identical, the
// lower distance wins.
static bool sortAngle(const Pt& a, const Pt& b) {
    if (a.angle == b.angle) {
        if (a.dist > b.dist)
            return true;
    }
    return (a.angle < b.angle);
}

/*
 1.10 ConvexHull calculates the convex hull of this face and returns it as a
 new face.
 Cornerpoints on the convex hull are treated as members of the hull.
 
*/
Face Face::ConvexHull() {
    // erase the previously calculated hull
    convexhull.erase(convexhull.begin(), convexhull.end());
    vector<Pt> lt = getPoints();

    // Sort the points by the y-axis, begin with the lowest-(leftest) point
    std::sort(lt.begin(), lt.end());

    // Now calculate the polar coordinates (angle and distance of each point
    // with the lowest-(leftest) point as origin.
    for (unsigned int a = 1; a < lt.size(); a++) {
        lt[a].calcAngle(lt[0]);
    }
    // Sort the other points by their angle (startpoint lt[0] remains in place)
    std::sort(lt.begin()+1, lt.end(), sortAngle);

    // since we want the cornerpoints on the hull, too, we need to list the
    // points with the lowest angle in counter-clockwise order. We sorted with
    // descending distance as second criterion, so we need to reverse these.
    // For that reason the points with the highest angle are already in
    // ccw-order.
    
    std::vector<Pt>::iterator s = lt.begin() + 1, e = s; // Start with the
    // second point and find the last point with the same angle.
    while (s->angle == e->angle)
        e++;
    std::reverse(s, e); // Now reverse these to get the correct order.

    // This is the main part of the Graham scan. Initially, take the first two
    // points and put them on a stack
    vector<Pt> uh = vector<Pt> ();
    uh.push_back(lt[0]);
    uh.push_back(lt[1]);
    for (int a = 2; a < (int) lt.size();) {
        assert(uh.size() >= 2); // If the stack shrinks to 1, something went
                                // ugly wrong.
        // Examine the two points on top of the stack ...
        Pt point1 = uh[uh.size() - 1];
        Pt point2 = uh[uh.size() - 2];
        Pt next = lt[a]; // ... and the next candidate for the hull
        if (leftOf(point2, point1, next)) {
            // If the candidate is left of (or on) the segment made up of the
            // twp topmost points on the stack, we assume it is part of the hull
            uh.push_back(next); // and push it on the stack
            a++;
        } else {
            uh.pop_back(); // Otherwise it is right of the hull and the topmost
                           // point is definitively _not_ part of the hull, so
                           // we kick it.
        }
    }

    // Now make a face from the points on the convex hull, which the vector uh
    // now contains in counter-clockwise order.
    for (unsigned int i = 0; i < uh.size(); i++) {
        Pt p1 = uh[i];
        Pt p2 = uh[(i + 1) % uh.size()];
        Seg s = Seg(p1, p2);
        convexhull.push_back(s);
    }

    return Face(convexhull);
}
/*
  1.11 Translate moves all points of a face by the given offsets
 
*/
void Face::Translate(int offx, int offy) {
    for (unsigned int i = 0; i < v.size(); i++) {
        v[i].s.x += offx;
        v[i].e.x += offx;
        v[i].s.y += offy;
        v[i].e.y += offy;
    }
}

/* 
  1.12 The following four functions are used to iterate over the segments of
  a face. Begin() resets the position to the first segment, Next() and Prev()
  change the position to the segment after/before the current segment (with
  wraparound). Cur() returns the current segment and End() determines, if the
  position is already past the end of the cycle.

*/
Seg Face::Begin() {
    cur = 0;
    
    return v[cur];
}

Seg Face::Prev() {
    cur--;
    if (cur < 0)
        cur = v.size() - 1;

    return v[cur];
}

Seg Face::Next() {
    cur++;

    return Cur();
}

Seg Face::Cur() {
    return v[cur % v.size()];
}

bool Face::End() {
    return cur >= (int) v.size();
}

/*
   1.13 collapse is used to generate the MSegs which collapse this Face to the
   point ~dst~. If ~close~ is false, then the initial and final segments are
   swapped, effectively making this the function "expand"
  
*/
MSegs Face::collapse(bool close, Pt dst) {
    MSegs ret;

    for (unsigned int i = 0; i < v.size(); i++) {
        if (close) {
            // The final segmens are degenerated to the collapse-point
            ret.AddMSeg(MSeg(v[i].s, v[i].e, dst, dst));
        } else {
            // The initial segments are degenerated to the expand-point
            ret.AddMSeg(MSeg(dst, dst, v[i].s, v[i].e));
        }
    }

    if (close)
        ret.sreg = *this;
    else
        ret.dreg = *this;

    // Mark the MSegs as collapsed (1) or expanded (2)
    ret.iscollapsed = 1 + (close ? 0 : 1);

    return ret;
}

/*
 1.14 collapseWithHoles collapses (or expands) the Face with all of its holes.
 
*/
MFace Face::collapseWithHoles(bool close) {
    MFace ret(collapse(close));

    for (unsigned int i = 0; i < holes.size(); i++) {
        ret.AddConcavity(holes[i].collapse(close, collapsePoint()));
    }

    ret.MergeConcavities();

    return ret;
}

/*
  1.15 Wrapper around 1.13 which selects the collapse-point automatically
 
*/
MSegs Face::collapse(bool close) {
    MSegs ret;

    Pt dst = collapsePoint();

    return collapse(close, dst);
}

/*
  1.16 Select a suitable collapse-point for this face
 
*/
Pt Face::collapsePoint() {
    Pt dst;

    // The peerpoint (usually set by RotatingPlane) is the best choice
    if (peerPoint.valid)
        dst = peerPoint;
    else // Otherwise just use the first point
        dst = v[0].s;
    
    return dst;
}

/*
 1.17 getFaces extracts all Faces from the NestedList-Representation of a
 Secondo-region (including their holes)
 
*/
vector<Face> Face::getFaces(ListExpr le) {
    vector<Face> ret;

    while (!nl->IsEmpty(le)) {
        // Iterate over all faces
        ListExpr l = nl->First(le);
        // Create a Face with optional holes from the current list-position
        Face r(l);
        ret.push_back(r);
        // Advance in the list
        le = nl->Rest(le);
    }

    return ret;
}

/*
  1.18 GetMiddle returns the middle-point of the bounding-box of this face.
  This is much easier to calculate than for example the centroid.
 
*/
Pt Face::GetMiddle() {
    Pt middle = (GetBoundingBox().second + GetBoundingBox().first) / 2;

    return middle;
}

/*
  1.19 GetArea returns the area of this polygon (ignoring holes) 

*/
double Face::GetArea() {
    return this->MakeRegion(false).Area(NULL);
}

/*
 1.20 GetCentroid returns the centroid of this face disregarding holes.
 
*/
Pt Face::GetCentroid() {
    double area = GetArea();
    double x = 0, y = 0;

    // Just return some point of the face if the area is zero
    if (area == 0)
        return v[0].s;

    // Otherwise calculate the centroid by the common method
    unsigned int n = v.size();
    for (unsigned int i = 0; i < n; i++) {
        double tmp = (v[i].s.x * v[i].e.y - v[i].e.x * v[i].s.y);
        x += (v[i].s.x + v[i].e.x) * tmp;
        y += (v[i].s.y + v[i].e.y) * tmp;
    }
    x = x * (1 / (6 * area));
    y = y * (1 / (6 * area));

    return Pt(x, y);
}

/*
  1.21 distance calculates the distance of the middlepoints of the bounding-
  boxes of this face to the given face.
 
*/
double Face::distance(Face r) {
    return r.GetMiddle().distance(GetMiddle());
}

/*
 1.22 ToString gives a string-representation of this face (mainly for debugging
 purposes)

*/
string Face::ToString() const {
    std::ostringstream ss;

    for (unsigned int i = 0; i < v.size(); i++)
        ss << v[i].ToString() << "\n";
    for (unsigned int i = 0; i < holes.size(); i++) {
        ss << "Hole " << (i + 1) << "\n";
        ss << holes[i].ToString();
    }

    return ss.str();
}

/*
  1.23 GetBoundingBox of a list of Faces returns the Bounding-Box which
  encloses all faces together.
 
*/
pair<Pt, Pt> Face::GetBoundingBox(vector<Face> regs) {
    if (regs.empty())
        return pair<Pt, Pt>(Pt(0, 0), Pt(0, 0));
    pair<Pt, Pt> ret = regs[0].bbox;
    for (unsigned int i = 1; i < regs.size(); i++) {
        pair<Pt, Pt> bbox = regs[i].bbox;
        if (bbox.first.x < ret.first.x)
            ret.first.x = bbox.first.x;
        if (bbox.second.x > ret.second.x)
            ret.second.x = bbox.second.x;
        if (bbox.first.y < ret.first.y)
            ret.first.y = bbox.first.y;
        if (bbox.second.y > ret.second.y)
            ret.second.y = bbox.second.y;
    }

    return ret;
}

pair<Pt, Pt> Face::GetBoundingBox(set<Face*> regs) {
    if (regs.empty())
        return pair<Pt, Pt>(Pt(0, 0), Pt(0, 0));
    //    assert(*(*(regs.begin()))->v.size() > 0);

    //    Reg *r = *(regs.begin());
    pair<Pt, Pt> ret = (*(regs.begin()))->bbox;

    for (std::set<Face*>::iterator it = regs.begin(); it != regs.end(); ++it) {
        pair<Pt, Pt> bbox = (*it)->bbox;
        if (bbox.first.x < ret.first.x)
            ret.first.x = bbox.first.x;
        if (bbox.second.x > ret.second.x)
            ret.second.x = bbox.second.x;
        if (bbox.first.y < ret.first.y)
            ret.first.y = bbox.first.y;
        if (bbox.second.y > ret.second.y)
            ret.second.y = bbox.second.y;
    }

    return ret;
}

/*
 1.24 GetBoundingBox returns the bounding-box of this face. It is not
 calculated here since this is always tracked when segments are added to this
 face.
 
*/
pair<Pt, Pt> Face::GetBoundingBox() {
    return bbox;
}

/*
 1.25 GetMSegs returns an MSegs-object containing MSeg-objects representing
 this face statically (initial and final segments equal the segments of this
 face). If ~triangles~ is set, then for each segment two MSeg-objects are
 created, one with degenerated initial and one with degenerated final segment.
 
*/
MSegs Face::GetMSegs(bool triangles) {
    MSegs ret;

    for (unsigned int i = 0; i < v.size(); i++) {
        Seg s = v[i];
        if (triangles) {
            ret.AddMSeg(MSeg(s.s, s.e, s.s, s.s));
            ret.AddMSeg(MSeg(s.e, s.e, s.s, s.e));
        } else {
            ret.AddMSeg(MSeg(s.s, s.e, s.s, s.e));
        }
    }

    return ret;
}

Face Face::ClipEar() {
    Face ret;

    if (v.size() <= 3) {
        return Face(v);
    } else {
        Pt a, b, c;
        unsigned int n = v.size();

        for (unsigned int i = 0; i < n; i++) {
            a = v[(i + 0) % n].s;
            b = v[(i + 1) % n].s;
            c = v[(i + 2) % n].s;
            if (Pt::sign(a, b, c) < 0) {
                continue;
            }

            bool inside = false;
            for (unsigned int j = 0; j < (n - 3); j++) {
                Pt x = v[(i + j + 3) % n].s;
                inside = Pt::insideTriangle(a, b, c, x);
                if (inside) {
                    break;
                }
            }

            if (!inside) {
                ret.AddSeg(v[i + 0]);
                ret.AddSeg(v[i + 1]);
                Seg nw(v[i + 1].e, v[i + 0].s);
                ret.AddSeg(nw);
                v.erase(v.begin() + i);
                v[i].s = nw.e;
                v[i].e = nw.s;
                hullSeg.valid = 0;

                return ret;
            }
        }
    }
    assert(false);
    // Shouldn't ever happen
    return ret;
}

vector<MSegs> Face::Evaporate(bool close) {
    vector<MSegs> ret;
    Face reg(v);
    reg.IntegrateHoles();
    //    cerr << "Clipping ear of " << ToString() << "\n";

    while (reg.v.size() > 3) {
        Face r = reg.ClipEar();
        ret.push_back(r.collapse(close, r.GetCentroid()));
    }
    ret.push_back(reg.collapse(close, reg.GetCentroid()));

    return ret;
}

void Face::IntegrateHoles() {
    vector<Seg> allsegs = v;

    for (unsigned int i = 0; i < holes.size(); i++) {
        allsegs.insert(allsegs.end(), holes[i].v.begin(), holes[i].v.end());
    }

    for (unsigned int h = 0; h < holes.size(); h++) {
        Face hole = holes[h];
        unsigned int i = 0, j = 0;
        bool found = false;

        Pt s, e;
        Seg se;
        while (!found) {
            s = v[i].s;
            while (!found && j < hole.v.size()) {
                e = hole.v[j].e;
                se = Seg(s, e);
                bool intersects = false;
                for (unsigned int k = 0; k < allsegs.size(); k++) {
                    if (se.intersects(allsegs[k])) {
                        intersects = true;
                        break;
                    }
                }
                if (!intersects) {
                    found = true;
                    break;
                } else {
                    j = j + 1;
                }
            }
        }

        vector<Seg> newsegs;
        newsegs.insert(newsegs.end(), v.begin(), v.begin()+(i - 1));
        newsegs.push_back(Seg(s, e));
        unsigned int n = hole.v.size();
        for (unsigned int k = 0; k < n; k++) {
            Seg ns = hole.v[n - (k + j) % n - 1];
            ns.ChangeDir();
            newsegs.push_back(ns);
        }
        newsegs.push_back(Seg(e, s));
        newsegs.insert(newsegs.end(), v.begin() + i, v.end());
        v = newsegs;
        allsegs.push_back(Seg(s, e));
    }

    holes.clear();
}

bool Face::Check() {
    bool ret = true;

    if (v.size() == 0) {
        return false;
    }

    assert(v.size() >= 3);

    for (unsigned int i = 0; i < v.size(); i++) {
        if (v[i].s == v[i].e) {
            cerr << "degenerated segment\n";
            ret = false;
        }
        for (unsigned int j = 0; j < v.size(); j++) {
            if (i == j)
                continue;
            if (v[i].intersects(v[j])) {
                cerr << "Region intersects\n";
                ret = false;
            }
            if (v[i].s == v[j].s) {
                cerr << "Same startpoint\n";
                ret = false;
            }
            if (v[i].e == v[j].e) {
                cerr << "Same endpoint\n";
                ret = false;
            }
        }
    }

    unsigned int nr = (int) v.size();
#ifdef DO_ANGLECHECK
    for (unsigned int i = 0; i < nr; i++) {
        Seg a = v[i];
        Seg b = v[(i + 1) % nr];

        long double aa = a.angle();
        long double ab = b.angle();
        long double ab2 = b.angle() + 180;
        if (ab2 > 360)
            ab2 -= 180;
        if ((aa == ab) || (aa == ab2)) {
            cerr << "Angle-Check failed!\n";
            ret = false;
        }
    }
#endif

    for (unsigned int i = 0; i < (nr - 1); i++) {
        if (!(v[i].e == v[i + 1].s)) {
            cerr << "Region not contiguous\n";
            ret = false;
        }
    }
    
    if (!(v[nr - 1].e == v[0].s)) {
        cerr << "Region not closed\n";
        ret = false;
    }
    
    if (v[0].angle() > v[nr - 1].angle()) {
        cerr << "Region not clockwise\n";
        ret = false;
    }

    if (!ret) {
        cerr << "Invalid Region:\n" << this->ToString() << "\n";
    }

    assert(ret);

    return ret;
}

void Face::AddHole(vector<Seg> hole) {
    vector<Seg> nsegs;
    bool ishole = true;

    if (hole.size() < 3)
        return;

    hole = sortSegs(hole);

    for (unsigned int i = 0; i < hole.size(); i++) {
        bool found = false;
        for (unsigned int j = 0; j < v.size(); j++) {
            if (hole[i] == v[j]) {
                found = true;
                ishole = false;
                v.erase(v.begin() + j);
                break;
            }
        }
        if (!found) {
            nsegs.push_back(hole[i]);
        }
    }
    if (ishole) {
        Face r(hole);
        r.ishole = true;
        r.Close();
        holes.push_back(r);
    } else {
        for (unsigned int i = 0; i < nsegs.size(); i++) {
            nsegs[i].ChangeDir();
        }
        nsegs.insert(nsegs.end(), v.begin(), v.end());
        v = sortSegs(nsegs);
    }
}

