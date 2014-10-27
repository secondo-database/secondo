/*
  1 Class Face
  This class represents a face with an optional set of holes.
  
*/

#include "interpolate.h"


Face::Face() : cur(0), ishole(false) {
}

/*
  1.1 Constructs a face from a region-listexpression.

*/
Face::Face(RList tle, bool withHoles) : cur(0), parent(NULL), ishole(false) {
   RList f;
   if (withHoles) {
      f = tle.items[0];
   } else {
      f = tle;
   }
   
    // Construct segments from the points
    for (unsigned int i = 0; i < f.items.size()-1; i++) {
        RList pa = f.items[i];
        RList pb = f.items[i+1];
        double p1 = pa.items[0].getNr() * SCALEIN;
        double p2 = pa.items[1].getNr() * SCALEIN;
        double p3 = pb.items[0].getNr() * SCALEIN;
        double p4 = pb.items[1].getNr() * SCALEIN;
        Seg s = Seg(Pt(p1, p2), Pt(p3, p4));
        AddSeg(s);
    }
    if (v.size() < 2) {
        // Only one segment yet, this cannot be valid. Return an empty region.
        v.clear();
    }
    Close(); // Close and sort the cycle
    
    // Construct the holes
    if (withHoles) {
        for (unsigned int i = 1; i < tle.items.size(); i++) {
            Face hole (tle.items[i], false);
            if (hole.v.size() < 3)
                continue;
            hole.ishole = true;
            holes.push_back(hole);
        }
    }
    
    Check();
}

/*
  1.2 Constructs a face from a set of segments.

*/
Face::Face(vector<Seg> v) : cur(0), v(v), parent(NULL), ishole(false) {
    Sort();
    Check();
}

/*
  1.3 Convert this face to a Poly object with the given offsets and
      scale-factors. Also convert holes, if the parameter ~withholes~ is true

*/
Poly Face::MakePoly(double offx, double offy, double scalex, double scaley,
        bool withholes)
{
    Poly ret(*this, offx, offy, scalex, scaley, withholes);

    return ret;
}

/*
  1.4 Convert this face to a Poly object

*/
Poly Face::MakePoly(bool withholes) {
    return MakePoly(0, 0, 1, 1, withholes);
}

/*
  1.5 Sort the segments of this Face.

*/
void Face::Sort() {
    v = sortSegs(v);
}

/* 
  1.6 ~sortSegs~ sorts a list of segments to be in the correct order of a cycle.
  A cycle should begin with the lowest-(leftmost)-point and then go counter-
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

    // Find the corresponding segment with the lowest
    // end-point and change its direction to compare the angle
    for (unsigned int i = 0; i < v.size(); i++) {
        if ((v[i].e.x == minx) && (v[i].e.y == miny)) {
            start2 = i;
            minseg2 = v[i];
        }
    }
    minseg2.ChangeDir();

    // If the angle of the segment with the lowest end-point is less than
    // the segment with the lowest start-point, then the segments are oriented
    // clockwise at the moment, so we have to change their direction
    if (minseg2.angle() < minseg1.angle()) {
        for (unsigned int i = 0; i < v.size(); i++) {
            v[i].ChangeDir();
        }
        start = start2;
    }

    // Now go and seek the next segment for each segment and put it ordered
    // into a list
    Seg cur = v[start];
    Seg startseg = cur;
    ret.push_back(cur);
    while (1) {
        bool found = false;
        std::vector<Seg>::iterator i = v.begin();
        while (i != v.end()) {
            if (i->s == cur.e) {
                cur = *i;
                ret.push_back(cur);
                i = v.erase(i);
                found = true;
                if (cur.e == startseg.s)
                    break;
            } else {
                i++;
            }
        }
        if (STRICT)
           assert(found); // This should never happen on a complete cycle.
        else if (!found) {
            ret.clear(); 
            break;
        }
        // If the endpoint of this segment is the startpoint of the first, we
        // are done.
        if (cur.e == startseg.s)
            break;
    }

    return ret;
}

/*
 1.7 ~AddSeg~ adds a segment to this face and updates the bounding-box
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
 1.8 ~Close~ completes the cycle if this is not already the case and
 pre-calculates the convex hull.
 
*/
void Face::Close() {
    if (v.size() < 2) // Cannot close a face with only one (or even no) segment
        return;
    int i = v.size() - 1;
    if (!(v[i].e == v[0].s)) {
        // The cycle is not closed yet, we do that now.
        Seg s(v[i].e, v[0].s);
        AddSeg(s);
    }
    Sort();
    ConvexHull();
    Check();
}

/*
 1.9 ~getPoints~ returns a vector with all points of the current cycle.
 
*/
vector<Pt> Face::getPoints() {
    vector<Pt> points;

    for (unsigned int i = 0; i < v.size(); i++) {
        points.push_back(v[i].s);
    }

    return points;
}

/*
 1.10 ~leftOf~ determines if the point ~next~ is left of the segment
      (~pt1~ ~pt2~). It also returns true if it is on the line.
 
*/
static bool leftOf(Pt pt1, Pt pt2, Pt next) {
    long double tmp = ((pt2.x - pt1.x)*(next.y - pt1.y)
            -(next.x - pt1.x)*(pt2.y - pt1.y));
    return tmp >= 0;
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
 1.11 ~ConvexHull~ calculates the convex hull of this face and returns it as a
 new face.
 Corner-points on the convex hull are treated as members of the hull.
 
*/
Face Face::ConvexHull() {
    if (isEmpty())
        return Face();
    // erase the previously calculated hull
    convexhull.erase(convexhull.begin(), convexhull.end());
    vector<Pt> lt = getPoints();

    // Sort the points by the y-axis, begin with the lowest-(leftmost) point
    std::sort(lt.begin(), lt.end());

    // Now calculate the polar coordinates (angle and distance of each point
    // with the lowest-(leftmost) point as origin.
    for (unsigned int a = 1; a < lt.size(); a++) {
        lt[a].calcPolar(lt[0]);
    }
    // Sort the other points by their angle (startpoint lt[0] remains in place)
    std::sort(lt.begin()+1, lt.end(), sortAngle);

    // since we want the corner-points on the hull, too, we need to list the
    // points with the lowest angle in counter-clockwise order. We sorted with
    // descending distance as second criterion, so we need to reverse these.
    // For that reason the points with the highest angle are already in
    // counter-clockwise order.
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
            // two topmost points on the stack, we assume it is part of the hull
            if (!(point1 == next))   // if it is not identical to the previous
                uh.push_back(next);  // point, then push it on the stack
            a++;
        } else {
            uh.pop_back(); // Otherwise it is right of the hull and the topmost
                           // point is definitively not part of the hull, so
                           // we kick it.
        }
    }

    // Now make a face from the points on the convex hull, which the vector ~uh~
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
  1.12 ~Transform~ modifies all points of this face by the given offsets and
  scale-factors

*/
void Face::Transform(Pt off, Pt scale) {
    for (unsigned int i = 0; i < v.size(); i++) {
        v[i].s = (v[i].s+off)*scale;
        v[i].e = (v[i].e+off)*scale;
    }
    for (unsigned int h = 0; h < holes.size(); h++) {
        holes[h].Transform(off, scale);
    }
}
/* 
  1.13 The following four functions are used to iterate over the segments of
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
   1.14 ~collapse~ is used to generate the MSegs which collapse this Face to the
   point ~dst~. If ~close~ is false, then the initial and final segments are
   swapped, effectively making this the function "expand"
  
*/
MSegs Face::collapse(bool close, Pt dst) {
    MSegs ret;

    for (unsigned int i = 0; i < v.size(); i++) {
        if (close) {
            // The final segments are degenerated to the collapse-point
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
 1.15 ~collapseWithHoles~ collapses (or expands) the Face with all of its holes.
 
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
  1.16 Wrapper around 1.14 which selects the collapse-point automatically
 
*/
MSegs Face::collapse(bool close) {
    MSegs ret;

    Pt dst = collapsePoint();

    return collapse(close, dst);
}

/*
  1.17 ~collapsePoint~ selects a suitable collapse point for this face
 
*/
Pt Face::collapsePoint() {
    Pt dst;

    // The peerpoint (usually set by RotatingPlane) is the best choice
    if (peerPoint.valid)
        dst = peerPoint;
    else if (!isEmpty()) // Otherwise just use the first point
        dst = v[0].s;
    
    return dst;
}

/*
 1.18 ~getFaces~ extracts all Faces from the RList representation of a
      region (including their holes)
 
*/
vector<Face> Face::getFaces(RList nl) {
    vector<Face> ret;
    
    for (unsigned int i = 0; i < nl.items.size(); i++) {
        Face r(nl.items[i], true);
        if (r.isEmpty())
            continue;
        ret.push_back(r);
    }

    return ret;
}

/*
  1.19 ~GetMiddle~ returns the center point of the bounding-box of this face.
  This is much more efficient to calculate than for example the centroid.
 
*/
Pt Face::GetMiddle() {
    Pt middle = (GetBoundingBox().second + GetBoundingBox().first) / 2;

    return middle;
}

/*
  1.20 ~GetArea~ returns the area of this polygon (ignoring holes) 

*/
double Face::GetArea() {
    return this->MakePoly(false).Area();
}

/*
 1.21 ~GetCentroid~ returns the centroid of this face disregarding holes.
 
*/
Pt Face::GetCentroid() {
    double area = GetArea();
    double x = 0, y = 0;
    
    if (isEmpty()) // Return anything if this is an empty face
        return Pt(0,0);

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
  1.22 ~distance~ calculates the distance of the center points of the bounding-
  boxes of this face to the given face.
 
*/
double Face::distance(Face r) {
    return r.GetMiddle().distance(GetMiddle());
}

/*
 1.23 ~ToString~gives a string-representation of this face (mainly for debugging
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
  1.24 ~GetBoundingBox~ of a list of faces returns the bounding box which
  encloses all faces.
 
*/
pair<Pt, Pt> Face::GetBoundingBox(vector<Face> fcs) {
    if (fcs.empty())
        return pair<Pt, Pt>(Pt(0, 0), Pt(0, 0));
    pair<Pt, Pt> ret = fcs[0].bbox;
    for (unsigned int i = 1; i < fcs.size(); i++) {
        pair<Pt, Pt> bbox = fcs[i].bbox;
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

pair<Pt, Pt> Face::GetBoundingBox(std::set<Face*> regs) {
    if (regs.empty())
        return pair<Pt, Pt>(Pt(0, 0), Pt(0, 0));
    
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
 1.25 ~GetBoundingBox~ returns the bounding-box of this face. It is not
 calculated here since this is always tracked when segments are added to this
 face.
 
*/
pair<Pt, Pt> Face::GetBoundingBox() {
    return bbox;
}

/*
 1.26 ~GetMSegs~ returns an MSegs-object containing MSeg-objects representing
 this face statically (initial and final segments equal the segments of this
 face). If ~triangles~ is set, then for each segment two MSeg-objects are
 created, one with degenerated initial and one with degenerated final segment.
 
*/
MSegs Face::GetMSegs(bool triangles, Pt off) {
    MSegs ret;

    for (unsigned int i = 0; i < v.size(); i++) {
        Seg s = v[i];
        if (triangles) {
            // Add two triangles, one with degenerated final
            ret.AddMSeg(MSeg(s.s, s.e, s.s+off, s.s+off)); 
            // and one with degenerated source segmemt
            ret.AddMSeg(MSeg(s.e, s.e, s.s+off, s.e+off));
        } else {
            // Add a trapezium here
            ret.AddMSeg(MSeg(s.s, s.e, s.s+off, s.e+off));
        }
    }

    for (unsigned int i = 0; i < holes.size(); i++) {
        
    }

    return ret;
}

MSegs Face::GetMSegs(bool triangles) {
    return GetMSegs(triangles, Pt(0,0));
}

/*
 1.26 ~GetMFace~ returns an MFace-object containing MSegs-objects representing
 this face statically with holes (initial and final segments equal the segments
 of this face). If ~triangles~ is set, then for each segment two MSeg-objects
 are created, one with degenerated initial and one with degenerated final
 segment.
 
*/
MFace Face::GetMFace(bool triangles, Pt off) {
    MFace ret;
    
    ret.face = GetMSegs(triangles, off);
    
    for (unsigned int i = 0; i < holes.size(); i++) {
        ret.AddConcavity(holes[i].GetMFace(triangles, off));
    }
    ret.MergeConcavities();

    return ret;
}

MFace Face::GetMFace(bool triangles) {
    return GetMFace(triangles, Pt(0,0));
}

/*
  1.27 ~ClipEar~ implements the Clipping-Ear-Algorithm.
  It finds an "Ear" in this Face, clips it and returns it as separate face.
  Note that the original Face is modified by this method!
  It is mainly used to implement evaporisation of faces
 
*/
Face Face::ClipEar() {
    Face ret;

    if (v.size() <= 3) {
        // Nothing to do if this Face consists only of three points
        return Face(v);
    } else {
        Pt a, b, c;
        unsigned int n = v.size();

        // Go through the corner-points, which are sorted counter-clockwise
        for (unsigned int i = 0; i < n; i++) {
            // Take the next three points
            a = v[(i + 0) % n].s;
            b = v[(i + 1) % n].s;
            c = v[(i + 2) % n].s;
            if (Pt::sign(a, b, c) < 0) {
                // If the third point c is right of the segment (a b), then
                // the three points don't form an "Ear"
                continue;
            }

            // Otherwise check, if any point is inside the triangle (a b c)
            bool inside = false;
            for (unsigned int j = 0; j < (n - 3); j++) {
                Pt x = v[(i + j + 3) % n].s;
                inside = Pt::insideTriangle(a, b, c, x) &&
                        !(a == x) && !(b == x) && !(c == x);
                if (inside) {
                    // If a point inside was found, we haven't found an ear.
                    break;
                }
            }

            if (!inside) {
                // No point was inside, so build the Ear-Face in "ret",
                ret.AddSeg(v[i + 0]);
                ret.AddSeg(v[(i + 1)%n]);
                Seg nw(v[(i + 1)%n].e, v[i + 0].s);
                ret.AddSeg(nw);
                
                // remove the Face-Segment (a b),
                v.erase(v.begin() + i);
                
                // and finally replace the segment (b c) by (a c)
                v[i].s = nw.e;
                v[i].e = nw.s;
                hullSeg.valid = 0;

                return ret;
            }
        }
    }
    
    DEBUG(1, "No ear found on face " << this->ToString() << "\n");
    // If we are here it means we haven't found an ear. This shouldn't happen.
    // One reason could be that the face wasn't valid in the first place.
    assert(false);
    return ret;
}

/*
  1.28 ~IntegrateHoles~ is used to integrate all holes of a face into the main
  cycle by creating "bridges". This usually yields an invalid face, it is
  exclusively used to triangulate a face.
 
*/
void Face::IntegrateHoles() {
    vector<Seg> allsegs = v;

    // First, get a list of all segments (inclusively all holes' segments)
    for (unsigned int i = 0; i < holes.size(); i++) {
        allsegs.insert(allsegs.end(), holes[i].v.begin(), holes[i].v.end());
    }

    for (unsigned int h = 0; h < holes.size(); h++) {
        Face hole = holes[h]; // Integrate one hole after another
        if (hole.isEmpty())
            continue;
        unsigned int i = 0, j;
        bool found = false;

        Pt s, e;
        Seg se;
        // Try to find a suitable bridge-segment by creating all segments
        // which connect the hole to the face and testing them for intersection
        // with any other segment
        while (!found && i < v.size()) {
            s = v[i].e;
            j = 0;
            while (!found && j < hole.v.size()) {
                e = hole.v[j].s;
                se = Seg(s, e); // A segment connecting the face to the hole
                bool intersects = false;
                for (unsigned int k = 0; k < allsegs.size(); k++) {
                    if (se.intersects(allsegs[k])) {
                        // We have found an intersection, so this segment is
                        // not our bridge
                        intersects = true;
                        break;
                    }
                }
                Seg tmp = v[(i+1)%v.size()];
                if (leftOf(tmp.s, tmp.e, e) && !intersects) {
                    // No intersection was found, so this is the bridge we use
                    found = true;
                    break;
                }
                j = j + 1;
            }
            if (!found) {
               i = i + 1;
            }
        }
        
        assert(found); // We should always be able to find a bridge

        // Now insert the bridge and the hole into the face-cycle
        vector<Seg> newsegs;
        // First copy the cycle from start to the begin of the bridge
        newsegs.insert(newsegs.end(), v.begin(), v.begin()+i+1);
        newsegs.push_back(Seg(s, e)); // Then add the bridge segment
        unsigned int n = hole.v.size();
        for (unsigned int k = 0; k < n; k++) {
            // Now add the hole-segments clockwise
            Seg ns = hole.v[(n + j - k - 1) % n];
            ns.ChangeDir(); // and change the orientation of each segment
            newsegs.push_back(ns);
        }
        newsegs.push_back(Seg(e, s)); // Bridge back to the original face
        // and add the rest of the original cycle
        newsegs.insert(newsegs.end(), v.begin() + i + 1, v.end());
        v = newsegs;
        // For the next holes we have to test intersection with the newly
        // created bridge, too.
        allsegs.push_back(Seg(s, e));
    }
    

    // All holes were integrated, so clear the list.
    holes.clear();
}

/*
 1.29 ~Evaporate~ splits this face in triangles and collapses these to a point
 inside. When the parameter ~close~ is false, then the triangles are expanded,
 making this the function "Condensate".
 
*/
vector<MSegs> Face::Evaporate(bool close) {
    vector<MSegs> ret;
    Face reg = *this; // Crate a copy first
    
    // At first, integrate all holes into the cycle
    reg.IntegrateHoles();

    while (reg.v.size() > 3) {
        // Then, repeatedly clip an ear until only a triangle is left
        Face r = reg.ClipEar();
        // and collapse (or expand) the triangles towards its centroid.
        MSegs s = r.collapse(close, r.GetCentroid());
        s.isevaporating = 1;
        ret.push_back(s);
    }
    // Finally, handle the last triangle left.
    MSegs s = reg.collapse(close, reg.GetCentroid());
    s.isevaporating = 1;
    ret.push_back(s);

    return ret;
}

/*
 1.30 ~Check~ tests, if the cycle of this face is valid.
 
*/
bool Face::Check() {
    bool ret = true;
    
    if (v.size() == 0) // Accept an empty face
        return true;

    if (v.size() < 3) {
        DEBUG(3, "too few segments: ");
        ret = false;
    }

    for (unsigned int i = 0; i < v.size(); i++) {
        if (v[i].s == v[i].e) {
            DEBUG(3, "degenerated segment");
            ret = false;
        }
        for (unsigned int j = 0; j < v.size(); j++) {
            if (i == j)
                continue;
            if (v[i].intersects(v[j])) {
                DEBUG(3, "ERROR: Segments intersect");
                ret = false;
            }
            if (v[i].s == v[j].s) {
                DEBUG(3, "ERROR: Same startpoint");
                ret = false;
            }
            if (v[i].e == v[j].e) {
                DEBUG(3, "ERROR: Same endpoint");
                ret = false;
            }
        }
    }

    int nr = (int) v.size();
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
            DEBUG(3, "Angle-Check failed!");
            ret = false;
        }
    }
#endif

    for (int i = 0; i < (nr - 1); i++) {
        if (!(v[i].e == v[i + 1].s)) {
            DEBUG(3, "ERROR: Region not contiguous");
            ret = false;
        }
    }
    
    if (nr > 0) {
        if (!(v[nr - 1].e == v[0].s)) {
            DEBUG(3, "ERROR: Region not closed\n");
            ret = false;
        }
    
        if (v[0].angle() > v[nr - 1].angle()) {
            DEBUG(3, "ERROR: Region not in counter-clockwise order");
            ret = false;
        }
    }

    if (!ret) {
        DEBUG(2, "Invalid Region:" <<  endl << this->ToString());
        assert(false);
    }
    
    if (STRICT)
       assert(ret);
    else if (!ret) {
        v.clear();
        convexhull.clear();
        holes.clear();
    }

    return ret;
}

/*
  1.31 ~AddHole~ takes a cycle and checks, if one if its segments is identical
  with one of this faces segments. If this is the case, then the cycle is
  integrated into the face. Otherwise it is added to the list of holes.
 
*/
void Face::AddHole(Face hface) {
    vector<Seg> hole = hface.v;
    bool ishole = true;
    
    if (hface.isEmpty() || isEmpty())
        return;

    std::sort(v.begin(), v.end());
    std::sort(hole.begin(), hole.end());

    // First, eliminate all matching segments by sorting both lists and
    // traversing them in parallel
    std::vector<Seg>::iterator i = hole.begin();
    std::vector<Seg>::iterator j = v.begin();
    while (i != hole.end() && j != v.end()) {
        if (*i == *j) {
            i = hole.erase(i);
            j = v.erase(j);
            ishole = false;
        } else if (*i < *j) {
            i++;
        } else {
            j++;
        }
    }
    
    if (ishole) {
        // We didn't find a matching segment, so this is a hole
        Face r(hole);
        r.ishole = true;
        r.Close();
        bool merged = false;
        for (unsigned int i = 0; i < holes.size(); i++) {
            merged = holes[i].Merge(r);
            if (merged)
                break;
        }
        if (!merged)
            holes.push_back(r);
    } else {
        // We found matching segments, so change the orientation of the
        // hole-segments and integrate them into the face-cycle
        for (unsigned int i = 0; i < hole.size(); i++) {
            hole[i].ChangeDir();
            v.push_back(hole[i]);
        }
    }
    
    Sort();
    Check();
}

/*
 1.32 ~CreateMFaces~ creates a static MFaces-object from a list of faces.

*/
MFaces Face::CreateMFaces(vector<Face> *faces) {
    MFaces ret;
    
    for (unsigned int i = 0; i < faces->size(); i++) {
        MFace mf = (*faces)[i].GetMSegs(false);
        for (unsigned int j = 0; j < (*faces)[i].holes.size(); j++) {
           mf.AddConcavity((*faces)[i].holes[j].GetMSegs(false));
        }
        mf.MergeConcavities();
        ret.AddMFace(mf);
    }
    
    return ret;
}

/*
 1.33 ~isEmpty~ determines, if this is an empty face without segments.

*/
bool Face::isEmpty() const {
    return v.size() < 3;
}

// Helper function for Face::Merge, which defines an order which is independent
// of the orientation of the function.
static bool segsLess2 (const Seg a, const Seg b) {
    Pt p1a = (a.s < a.e) ? a.s : a.e;
    Pt p2a = (b.s < b.e) ? b.s : b.e;
    
    Pt p1b = (a.s < a.e) ? a.e : a.s;
    Pt p2b = (b.s < b.e) ? b.e : b.s;
    
    return ((p1a < p2a) || ((p1a == p2a) && (p1b < p2b)));
}

/*
 1.34 ~Merge~ merges two adjacent faces into one face.
      Returns true, if that succeeded.
 
*/
bool Face::Merge(Face m) {
    vector<Seg> segs = v;
    vector<Seg> nsegs = m.v;
    DEBUG(4, "Trying to merge " << ToString() << " and " << m.ToString());
    
    sort(segs.begin(), segs.end(), segsLess2);
    sort(nsegs.begin(), nsegs.end(), segsLess2);
    
    std::vector<Seg>::iterator i = nsegs.begin();
    std::vector<Seg>::iterator j = segs.begin();
    
    bool found = false;
    while (i != nsegs.end() && j != segs.end()) {
        if (i->s == j->e && i->e == j->s) {
            i = nsegs.erase(i);
            j = segs.erase(j);
            found = true;
        } else if (segsLess2(*i, *j)) {
            i++;
        } else {
            j++;
        }
    }
    
    if (found) {
        DEBUG(3, "Merged faces");
        v = segs;
        v.insert(v.end(), nsegs.begin(), nsegs.end());
        holes.insert(holes.end(), m.holes.begin(), m.holes.end());
        Sort();
        Check();
    }
    
    return found;
}

Pt Face::findX(double x) {
    for (unsigned int i = 0; i < v.size(); i++) {
        if (v[i].s.x == x)
            return v[i].s;
    }
    
    return Pt(0, 0);
}

Pt Face::findY(double y) {
    for (unsigned int i = 0; i < v.size(); i++) {
        if (v[i].s.y == y)
            return v[i].s;
    }
    
    return Pt(0, 0);
}
