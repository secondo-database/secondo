/*
*/

#include "interpolate.h"

Reg::Reg() : cur(0), ishole(false) {
}

Reg::Reg(ListExpr tle) : cur(0), parent(NULL), ishole(false) {
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
        hole.ishole = true;
        holes.push_back(hole);
    }
    Close();
}

Reg::Reg(vector<Seg> v) : cur(0), parent(NULL), v(v), ishole(false) {
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
    int minx, miny, maxx, maxy;
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

MSegs Reg::collapse(bool close, Pt dst) {
    MSegs ret;

    if (v.size() < 3)
        return ret;


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

    ret.iscollapsed = 1 + (close?0:1);

    return ret;
}


MSegs Reg::collapse(bool close) {
    MSegs ret;

    Pt dst;

    if (v.size() < 3)
        return ret;

    if (peerPoint.valid)
        dst = peerPoint;
    else
        dst = v[0].s;

    return collapse(close, dst);
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
    Pt middle = (GetBoundingBox().second + GetBoundingBox().first) / 2;

    return middle;
}

Pt Reg::GetCentroid() {
    unsigned int n = v.size();
    double x = 0, y = 0;
    for (unsigned int i = 0; i < n; i++) {
        x = x + v[i].s.x;
        y = y + v[i].s.y;
    }

    return Pt(x/n, y/n);
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

pair<Pt,Pt> Reg::GetBoundingBox(vector<Reg> regs) {
    if (regs.empty())
        return pair<Pt,Pt>(Pt(0, 0), Pt(0, 0));
    assert(regs[0].v.size() > 0);
    pair<Pt,Pt> ret = regs[0].bbox;
    for (unsigned int i = 1; i < regs.size(); i++) {
        pair<Pt,Pt> bbox = regs[i].bbox;
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

pair<Pt,Pt> Reg::GetBoundingBox(set<Reg*> regs) {
    if (regs.empty())
        return pair<Pt,Pt>(Pt(0, 0), Pt(0, 0));
//    assert(*(*(regs.begin()))->v.size() > 0);
    
//    Reg *r = *(regs.begin());
    pair<Pt,Pt> ret = (*(regs.begin()))->bbox;
    
    for (std::set<Reg*>::iterator it=regs.begin(); it != regs.end(); ++it) {
        pair<Pt,Pt> bbox = (*it)->bbox;
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

pair<Pt,Pt> Reg::GetBoundingBox() {
    return bbox;
}

MSegs Reg::GetMSegs() {
    MSegs ret;
    
    for (unsigned int i = 0; i < v.size(); i++) {
        Seg s = v[i];
        ret.AddMSeg(MSeg(s.s, s.e, s.s, s.e));
    }
    
    return ret;
}

Reg Reg::ClipEar() {
    Reg ret;
    
    if (v.size() <= 3) {
        return Reg(v);
    } else {
        Pt a, b, c;
        unsigned int n = v.size();
        
        for (unsigned int i = 0; i < n; i++) {
            a = v[(i+0)%n].s;
            b = v[(i+1)%n].s;
            c = v[(i+2)%n].s;
            if (Pt::sign(a, b, c) < 0) {
                continue;
            }
            
            bool inside = false;
            for (unsigned int j = 0; j < (n-3); j++) {
                Pt x = v[(i+j+3)%n].s;
                inside = Pt::insideTriangle(a, b, c, x);
                if (inside) {
                    break;
                }
            }
            
            if (!inside) {
                ret.AddSeg(v[i+0]);
                ret.AddSeg(v[i+1]);
                Seg nw(v[i+1].e, v[i+0].s);
                ret.AddSeg(nw);
                v.erase(v.begin()+i);
                v[i].s = nw.e;
                v[i].e = nw.s;
                hullPoint = v[0].s;
                
                return ret;
            }
        }
    }
    assert(false);
    // Shouldn't ever happen
    return ret;
}

vector<MSegs> Reg::Evaporate(bool close) {
    vector<MSegs> ret;
    Reg reg(v);
    
    while (reg.v.size() > 3) {
        Reg r = reg.ClipEar();
        ret.push_back(r.collapse(close, r.GetCentroid()));
    }
    ret.push_back(reg.collapse(close, reg.GetCentroid()));
    
    return ret;
}

Reg Reg::Merge(Reg r) {
    double mindist = -1;
    Pt p1, p2;
    Reg ret;
    
    if (0 && r.parent == parent && r.hullSeg.valid && hullSeg.valid) {
        
    } else {
        for (unsigned int i = 0; i < v.size(); i++) {
            for (unsigned int j = 0; j < r.v.size(); j++) {
                double dist = v[i].s.distance(r.v[j].s);
                if (mindist < 0 || mindist > dist) {
                    mindist = dist;
                    p1 = v[i].s;
                    p2 = r.v[j].s;
                }
            }
        }
        for (unsigned int i = 0; i < v.size(); i++) {
            if (v[i].s == p1) {
                Seg s1(p1, p2);
                ret.AddSeg(s1);
                int index = 0;
                while (!(r.v[index].s == p2))
                    index++;
                for (unsigned int j = 0; j < r.v.size(); j++) {
                    ret.AddSeg(r.v[(index+j)%r.v.size()]);
                }
                Seg s2(p2, p1);
                ret.AddSeg(s2);
            }
            ret.AddSeg(v[i]);
        }
    }
    ret.Close();
    
    return ret;
}
