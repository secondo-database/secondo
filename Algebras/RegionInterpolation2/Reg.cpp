/*
*/

#include "interpolate.h"

Reg::Reg() {

    v = vector<Seg > ();
    cur = 0;
    parent = NULL;
    parentseg = 0;
}

Reg::Reg(ListExpr tle) {
    ListExpr le = nl->First(tle);
    while (nl->ListLength(le) > 1) {
        ListExpr pa = nl->First(le);
        ListExpr pb = nl->First(nl->Rest(le));
        int p1 = nl->RealValue(nl->First(pa));
        pa = nl->Rest(pa);
        int p2 = nl->RealValue(nl->First(pa));
        pa = nl->Rest(pa);
        int p3 = nl->RealValue(nl->First(pb));
        pb = nl->Rest(pb);
        int p4 = nl->RealValue(nl->First(pb));
        pb = nl->Rest(pb);
        le = nl->Rest(le);
        Seg s = Seg(p1, p2, p3, p4);
        AddSeg(s);
    }
    while (nl->ListLength(tle) > 1) {
        tle = nl->Rest(tle);
        Reg hole(tle);
        holes.push_back(hole);
    }
    Close();
}

Reg::Reg(vector<Seg> v) : v(v) {
    cur = 0;
    parent = NULL;
    parentseg = 0;
}

Reg::Reg(Reg *parent, int parentseg) : parent(parent), parentseg(parentseg) {

    v = vector<Seg > ();
    cur = 0;
}


Region Reg::MakeRegion(int offx, int offy) {
    Region ret(0);

    ret.StartBulkLoad();
    int edgeno = 0;
    for (unsigned int i = 0; i < v.size(); i++) {
        Point s(true, v[i].x1 + offx, v[i].y1 + offy);
        Point e(true, v[i].x2 + offx, v[i].y2 + offy);
        HalfSegment hs(false, s, e);
        hs.attr.faceno = 0;
        hs.attr.cycleno = 0;
        hs.attr.edgeno = edgeno;
        hs.attr.partnerno = edgeno;
        edgeno++;
        ret += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        ret += hs;
    }
    ret.EndBulkLoad(true, true, true, true);

    return ret;
}

Region Reg::MakeRegion() {
    return MakeRegion(0, 0);
}

void Reg::Sort() {
    v = sortSegs(v);
}


void Reg::AddSeg(Seg& a) {
   v.push_back(a);
}

void Reg::Close() {
    int i = v.size() - 1;
    if ((v[i].x2 != v[0].x1) || (v[i].y2 != v[0].y1)) {

        Seg s = Seg(v[i].x2, v[i].y2, v[0].x1, v[0].y1);
        AddSeg(s);
    }

    v = sortSegs(v);

    ConvexHull();
}

vector<Pt> Reg::getPoints() {
    vector<Pt> pt = vector<Pt > (v.size());

    for (unsigned int i = 0; i < v.size(); i++) {

        pt[i] = Pt(v[i].x1, v[i].y1);
    }

    return pt;
}

static bool leftOf(Pt pt1, Pt pt2, Pt next) {
    return ((pt2.x - pt1.x)*(next.y - pt1.y)
            -(next.x - pt1.x)*(pt2.y - pt1.y)) >= 0;
}

static bool sortAngle(const Pt& a, const Pt& b) {
    return (a.angle < b.angle);
}

void Reg::ConvexHull() {
    vector<Pt> lt = getPoints();
    std::sort(lt.begin(), lt.end());

    lt[0].angle = -1.0;
    for (unsigned int a = 1; a < lt.size(); a++) {
        lt[a].calcAngle(lt[0]);
    }
    std::sort(lt.begin(), lt.end(), sortAngle);

    vector<Pt> uh = vector<Pt > ();
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
        Seg s = Seg(p1.x, p1.y, p2.x, p2.y);
        convexhull.push_back(s);
    }
}

void Reg::Translate(int offx, int offy) {
    for (unsigned int i = 0; i < v.size(); i++) {
        v[i].x1 += offx;
        v[i].x2 += offx;
        v[i].y1 += offy;
        v[i].y2 += offy;
    }
}


void Reg::Begin() {
    cur = 0;
}

Seg Reg::Prev() {
    cur--;
    if (cur < 0)
        cur = v.size() - 1;

    return v[cur];
}

Seg Reg::Next() {
    cur++;

    return Cur();
}

Seg Reg::Cur() {
    return v[cur % v.size()];
}

int Reg::End() {
    if (cur >= (int) v.size())
        return 1;
    return 0;
}

MSegs Reg::collapse(bool close) {
    MSegs ret;

    Pt dst;

    if (peerPoint)
        dst = *peerPoint;
    else
        dst = Pt(v[0].x1, v[0].y1);

    if (peerPoint) {
        for (unsigned int i = 0; i < v.size(); i++) {
            if (close) {
                ret.AddMSeg(v[i].x1, v[i].y1, v[i].x2, v[i].y2,
                        dst.x, dst.y, dst.x, dst.y);
            } else {
                ret.AddMSeg(dst.x, dst.y, dst.x, dst.y,
                        v[i].x1, v[i].y1, v[i].x2, v[i].y2);
            }
        }
    }

    return ret;
}

vector<Reg> Reg::getRegs(ListExpr le) {
    vector<Reg> ret;

    while (!nl->IsEmpty(le)) {
        ListExpr l = nl->First(le);
        Reg r(l);
        ret.push_back(r);
        le = nl->Rest(le);
    }

    return ret;
}

Pt Reg::GetMiddle() {
    int x = 0, y = 0;
    for (unsigned int i = 0; i < v.size(); i++) {
        x += v[i].x1;
        y += v[i].y1;
    }
    x /= v.size();
    y /= v.size();
    
    return Pt(x,y);
}

string Reg::ToString() {
    std::ostringstream ss;

    for (unsigned int i = 0; i < v.size(); i++)
        ss << v[i].ToString() << "\n";

    return ss.str();
}

