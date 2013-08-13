/*
*/

#include "interpolate.h"

Reg::Reg() : cur(0) {
}

Reg::Reg(ListExpr tle) : cur(0) {
    ListExpr le = nl->First(tle);
    while (nl->ListLength(le) > 1) {
        ListExpr pa = nl->First(le);
        ListExpr pb = nl->First(nl->Rest(le));
        double p1 = nl->RealValue(nl->First(pa));
        pa = nl->Rest(pa);
        double p2 = nl->RealValue(nl->First(pa));
        pa = nl->Rest(pa);
        double p3 = nl->RealValue(nl->First(pb));
        pb = nl->Rest(pb);
        double p4 = nl->RealValue(nl->First(pb));
        pb = nl->Rest(pb);
        le = nl->Rest(le);
        Seg s = Seg(Pt(p1, p2), Pt(p3, p4));
        AddSeg(s);
    }
    while (nl->ListLength(tle) > 1) {
        tle = nl->Rest(tle);
        Reg hole(tle);
        holes.push_back(hole);
    }
    Close();
}

Reg::Reg(vector<Seg> v) : cur(0), v(v) {
}

Region Reg::MakeRegion(int offx, int offy) {
    Region ret(0);

    ret.StartBulkLoad();
    int edgeno = 0;
    for (unsigned int i = 0; i < v.size(); i++) {
        Point s(true, v[i].s.x + offx, v[i].s.y + offy);
        Point e(true, v[i].e.x + offx, v[i].e.y + offy);
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
    v = Seg::sortSegs(v);
}

void Reg::AddSeg(Seg& a) {
    v.push_back(a);
}

void Reg::Close() {
    int i = v.size() - 1;
    if (!(v[i].e == v[0].s)) {
        Seg s(v[i].e, v[0].s);
        AddSeg(s);
    }

    v = Seg::sortSegs(v);

    ConvexHull();
}

vector<Pt> Reg::getPoints() {
    vector<Pt> pt = vector<Pt > (v.size());

    for (unsigned int i = 0; i < v.size(); i++) {

        pt[i] = v[i].s;
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
    convexhull.erase(convexhull.begin(), convexhull.end());
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
        Seg s = Seg(p1, p2);
        convexhull.push_back(s);
    }
}

void Reg::Translate(int offx, int offy) {
    for (unsigned int i = 0; i < v.size(); i++) {
        v[i].s.x += offx;
        v[i].e.x += offx;
        v[i].s.y += offy;
        v[i].e.y += offy;
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

    if (peerPoint.valid)
        dst = peerPoint;
    else
        dst = v[0].s;

    for (unsigned int i = 0; i < v.size(); i++) {
        if (close) {
            ret.AddMSeg(MSeg(v[i].s, v[i].e, dst, dst));
        } else {
            ret.AddMSeg(MSeg(dst, dst, v[i].s, v[i].e));
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
    Pt middle = (GetMaxXY() + GetMinXY()) / 2;
    
    return middle;
}

double Reg::distance(Reg r) {
    return r.GetMiddle().distance(GetMiddle());
}

string Reg::ToString() const {
    std::ostringstream ss;

    for (unsigned int i = 0; i < v.size(); i++)
        ss << v[i].ToString() << "\n";

    return ss.str();
}

Pt Reg::GetMinXY(vector<Reg> regs) {
    if (regs.empty())
        return Pt(0, 0);
    assert(regs.size() > 0);
    assert(regs[0].v.size() > 0);
    double minx = regs[0].v[0].s.x;
    double miny = regs[0].v[0].s.y;
    for (unsigned int i = 0; i < regs.size(); i++) {
        for (unsigned int j = 0; j < regs[i].v.size(); j++) {
            if (regs[i].v[j].s.x < minx) {
                minx = regs[i].v[j].s.x;
            }
            if (regs[i].v[j].s.y < miny) {
                miny = regs[i].v[j].s.y;
            }
        }
    }

    return Pt(minx, miny);
}

Pt Reg::GetMaxXY(vector<Reg> regs) {
    if (regs.empty())
        return Pt(0, 0);
    assert(regs.size() > 0);
    assert(regs[0].v.size() > 0);
    double maxx = regs[0].v[0].s.x;
    double maxy = regs[0].v[0].s.y;
    for (unsigned int i = 0; i < regs.size(); i++) {
        for (unsigned int j = 0; j < regs[i].v.size(); j++) {
            if (regs[i].v[j].s.x > maxx) {
                maxx = regs[i].v[j].s.x;
            }
            if (regs[i].v[j].s.y > maxy) {
                maxy = regs[i].v[j].s.y;
            }
        }
    }

    return Pt(maxx, maxy);
}

Pt Reg::GetMinXY() {
    double minx = v[0].s.x;
    double miny = v[0].s.y;
    for (unsigned int j = 0; j < v.size(); j++) {
        if (v[j].s.x < minx) {
            minx = v[j].s.x;
        }
        if (v[j].s.y < miny) {
            miny = v[j].s.y;
        }
    }

    return Pt(minx, miny);
}

Pt Reg::GetMaxXY() {
    double maxx = v[0].s.x;
    double maxy = v[0].s.y;
    for (unsigned int j = 0; j < v.size(); j++) {
        if (v[j].s.x > maxx) {
            maxx = v[j].s.x;
        }
        if (v[j].s.y > maxy) {
            maxy = v[j].s.y;
        }
    }

    return Pt(maxx, maxy);
}
