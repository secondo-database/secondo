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

/*
  1.3 Convert this face to a region with the given offsets and scale-factors

*/
Region Face::MakeRegion(double offx, double offy, double scalex, double scaley)
{
    ListExpr cycle = nl->Empty();

    for (unsigned int i = 0; i < v.size(); i++) {
        ListExpr seg = nl->Empty();
        Append(seg, nl->RealAtom((v[i].s.x - offx) * scalex / SCALE));
        Append(seg, nl->RealAtom((v[i].s.y - offy) * scaley / SCALE));
        Append(cycle, seg);
    }

    ListExpr cycles = nl->OneElemList(cycle);

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
Region Face::MakeRegion() {
    return MakeRegion(0, 0, 1, 1);
}

/*
  1.5 Sort the segments of this Face.

*/
void Face::Sort() {
    v = sortSegs(v);
}

/* 
  1.6 Sort a list of segments to be in the correct order of a cycle.
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
        assert(found);
        if (cur.e == startseg.s)
            break;
    }
//    for (unsigned int j = 0; j < v.size(); j++) {
//        for (unsigned int i = 0; i < v.size(); i++) {
//            if ((v[i].s.x == cur.e.x) && (v[i].s.y == cur.e.y)) {
//                if (!(v[i] == v[start]))
//                    ret.push_back(v[i]);
//                cur = v[i];
//                break;
//            }
//        }
//    }

    return ret;
}

void Face::AddSeg(Seg a) {
    double minx, miny, maxx, maxy;
    if (a.s.x > a.e.x) {
        minx = a.e.x;
        maxx = a.s.x;
    } else {
        minx = a.s.x;
        maxx = a.e.x;
    }
    if (a.s.y > a.e.y) {
        miny = a.e.y;
        maxy = a.s.y;
    } else {
        miny = a.s.y;
        maxy = a.e.y;
    }

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

void Face::Close() {
    if (!v.size())
        return;
    int i = v.size() - 1;
    if (!(v[i].e == v[0].s)) {
        Seg s(v[i].e, v[0].s);
        AddSeg(s);
    }

    Sort();

    ConvexHull();
    Check();
}

vector<Pt> Face::getPoints() {
    vector<Pt> pt = vector<Pt > (v.size());

    for (unsigned int i = 0; i < v.size(); i++) {

        pt[i] = v[i].s;
    }

    return pt;
}

//#define GS1

static bool leftOf(Pt pt1, Pt pt2, Pt next) {
    long double sign = ((pt2.x - pt1.x)*(next.y - pt1.y)
            -(next.x - pt1.x)*(pt2.y - pt1.y));
    return sign
            >= 0
            ;
}

static bool sortAngle(const Pt& a, const Pt& b) {
    if (a.angle == b.angle) {
        if (a.dist > b.dist)
            return true;
    }
    return (a.angle < b.angle);
}

Face Face::ConvexHull() {
    assert(v.size() > 0);
    convexhull.erase(convexhull.begin(), convexhull.end());
    vector<Pt> lt = getPoints();

    std::sort(lt.begin(), lt.end());

    lt[0].angle = -1.0;
    for (unsigned int a = 1; a < lt.size(); a++) {
        lt[a].calcAngle(lt[0]);
    }
    std::sort(lt.begin(), lt.end(), sortAngle);

    std::vector<Pt>::iterator s = lt.begin() + 1, e = s;
    while (s->angle == e->angle)
        e++;

    std::reverse(s, e);

    vector<Pt> uh = vector<Pt> ();
    uh.push_back(lt[0]);
    uh.push_back(lt[1]);

    for (int a = 2; a < (int) lt.size();) {
        assert(uh.size() >= 2);
        Pt point1 = uh[uh.size() - 1];
        Pt point2 = uh[uh.size() - 2];
        Pt next = lt[a];
        if (leftOf(point2, point1, next)) {
            uh.push_back(next);
            a++;
        } else {
            uh.pop_back();
        }
    }

    for (unsigned int i = 0; i < uh.size(); i++) {
        Pt p1 = uh[i];
        Pt p2 = uh[(i + 1) % uh.size()];
        Seg s = Seg(p1, p2);
        convexhull.push_back(s);
    }

    return Face(convexhull);
}

void Face::Translate(int offx, int offy) {
    for (unsigned int i = 0; i < v.size(); i++) {
        v[i].s.x += offx;
        v[i].e.x += offx;
        v[i].s.y += offy;
        v[i].e.y += offy;
    }
}

void Face::Begin() {
    cur = 0;
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

int Face::End() {
    if (cur >= (int) v.size())
        return 1;
    return 0;
}

MSegs Face::collapse(bool close, Pt dst) {
    MSegs ret;

    if (v.size() < 3)
        return ret;

    Close();

    for (unsigned int i = 0; i < v.size(); i++) {
        if (close) {
            ret.AddMSeg(MSeg(v[i].s, v[i].e, dst, dst));
        } else {
            ret.AddMSeg(MSeg(dst, dst, v[i].s, v[i].e));
        }
    }

    if (close)
        ret.sreg = *this;
    else
        ret.dreg = *this;

    ret.iscollapsed = 1 + (close ? 0 : 1);

    return ret;
}

MFace Face::collapseWithHoles(bool close) {
    MFace ret(collapse(close));

    for (unsigned int i = 0; i < holes.size(); i++) {
        ret.AddConcavity(holes[i].collapse(close, collapsePoint()));
    }

    ret.MergeConcavities();

    return ret;
}

MSegs Face::collapse(bool close) {
    MSegs ret;


    if (v.size() < 3)
        return ret;

    Pt dst = collapsePoint();

    return collapse(close, dst);
}

Pt Face::collapsePoint() {
    Pt dst;

    if (peerPoint.valid)
        dst = peerPoint;
    else
        dst = v[0].s;

    return dst;
}

vector<Face> Face::getFaces(ListExpr le) {
    vector<Face> ret;

    while (!nl->IsEmpty(le)) {
        ListExpr l = nl->First(le);
        Face r(l);
        ret.push_back(r);
        le = nl->Rest(le);
    }

    return ret;
}

Pt Face::GetMiddle() {
    Pt middle = (GetBoundingBox().second + GetBoundingBox().first) / 2;

    return middle;
}

double Face::GetArea() {
    return this->MakeRegion().Area(NULL);
}

Pt Face::GetCentroid() {
    double area = GetArea();
    double x = 0, y = 0;

    if (area == 0)
        return v[0].s;

    unsigned int n = v.size();
    for (unsigned int i = 0; i < n; i++) {
        double tmp = (v[i].s.x * v[i].e.y - v[i].e.x * v[i].s.y);
        x += (v[i].s.x + v[i].e.x) * tmp;
        y += (v[i].s.y + v[i].e.y) * tmp;
    }
    x = x * 1 / (6 * area);
    y = y * 1 / (6 * area);

    return Pt(x, y);

    //    unsigned int n = v.size();
    //    double x = 0, y = 0;
    //    for (unsigned int i = 0; i < n; i++) {
    //        x = x + v[i].s.x;
    //        y = y + v[i].s.y;
    //    }
    //
    //    return Pt(x / n, y / n);
}

double Face::distance(Face r) {
    return r.GetMiddle().distance(GetMiddle());
}

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

pair<Pt, Pt> Face::GetBoundingBox() {
    return bbox;
}

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
        if (v[i].s == v[i].e)
            ret = false;
        for (unsigned int j = 0; j < v.size(); j++) {
            if (i == j)
                continue;
            if (v[i].intersects(v[j])) {
                ret = false;
            }
            if (v[i].s == v[j].s)
                ret = false;
            if (v[i].e == v[j].e)
                ret = false;
        }
    }

    unsigned int nr = (int) v.size();
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

    for (unsigned int i = 0; i < (nr - 1); i++) {
        if (!(v[i].e == v[i + 1].s))
            ret = false;
    }
    if (!(v[nr - 1].e == v[0].s))
        ret = false;
    if (v[0].angle() > v[nr - 1].angle())
        ret = false;

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

