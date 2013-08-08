/*
*/

#include "interpolate.h"

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

Reg::Reg(ListExpr le) {
    le = nl->First(le);

    while (nl->ListLength(le) > 1) {
        ListExpr pa = nl->First(le);
        ListExpr pb = nl->First(nl->Rest(le));
        //        cerr << nl->ToString(pa) << " " << nl->ToString(pb) << "\n";
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

    Close();
}

Reg::Reg() {

    v = vector<Seg > ();
    cur = 0;
    parent = NULL;
    parentseg = 0;
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

void Reg::AddSeg(Seg& a) {

    v.push_back(a);
}

string Reg::ToString() {
    std::ostringstream ss;

    for (unsigned int i = 0; i < v.size(); i++)
        cerr << v[i].ToString() << "\n";

    return ss.str();
}

void Reg::Close() {
    int i = v.size() - 1;
    if ((v[i].x2 != v[0].x1) || (v[i].y2 != v[0].y1)) {

        Seg s = Seg(v[i].x2, v[i].y2, v[0].x1, v[0].y1);
        AddSeg(s);
    }

    v = sortSegs(v);

    cerr << "Dumping Reg\n"
            << ToString()
            << "Dumping Reg End\n";
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
    cerr << "Calculating Convex Hull Start\n";

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
        cerr << "List len: " << uh.size() << "\n";
        assert(uh.size() >= 2);
        Pt point1 = uh[uh.size() - 1];
        Pt point2 = uh[uh.size() - 2];
        Pt next = lt[a];

        cerr << "P1: " << point1.ToString()
                << "P2: " << point2.ToString()
                << "Nx: " << next.ToString()
                << "\n";

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
    //    convexhull = sortSegs(convexhull);

    cerr << "Calculating Convex Hull End\n";
}

int depth = 1;

vector<Reg> Reg::Concavities() {
    vector<Reg> ret = vector<Reg > ();
    vector<Seg> ch = convexhull;
    unsigned int j = 0;

    cerr << "Calculating Concavities Start " << depth++ << "\n";
    cerr << "Hull\n";
    for (unsigned int a = 0; a < ch.size(); a++) {
        cerr << ch[a].ToString() << "\n";
    }
    cerr << "Pol\n";
    for (unsigned int a = 0; a < v.size(); a++) {
        cerr << v[a].ToString() << "\n";
    }

    for (j = 0; j < v.size(); j++) {
        if ((ch[0].x1 == v[j].x1) && (ch[0].y1 == v[j].y1)) {
            break;
        }
    }

    for (unsigned int i = 0; i < ch.size(); i++) {
        if (!(ch[i] == v[j])) {
            cerr << "Found new Concavity: " << depth << "\n";
            Reg r = Reg(this, i);
            unsigned int hpidx;
            if (j == 0) {
                hpidx = v.size() - 1;
            } else {
                hpidx = j - 1;
            }
            r.hullPoint = new Pt(v[hpidx].x1, v[hpidx].x2);
            cerr << "End: " << ch[i].x2 << "/" << ch[i].y2 << "\n";
            do {
                Seg s = Seg(v[j].x2, v[j].y2, v[j].x1, v[j].y1);
                r.AddSeg(s);
                j = (j + 1) % v.size();
            } while ((ch[i].x2 != v[j].x1) || (ch[i].y2 != v[j].y1));
            std::reverse(r.v.begin(), r.v.end());
            r.Close();
            //            r.Print();
            cerr << "End Found new Concavity: " << depth << "\n";
            ret.push_back(r);
        } else {
            j = (j + 1) % v.size();
        }
    }

    cvs = ret;

    cerr << "Found " << cvs.size() << " Concavities\n";

    cerr << "Calculating Concavities End " << --depth << "\n";

    return ret;
}

vector<Reg> Reg::Concavities2(Reg *reg2) {
    vector<Reg> ret;
    Reg *reg1 = this;

    reg1->Begin();
    reg2->Begin();

    Reg r1 = Reg(reg1->convexhull);
    Reg r2 = Reg(reg2->convexhull);
    cerr << "\n\nConcavities2: START\n";

    do {
        double a1 = r1.Cur().angle();
        double a2 = r2.Cur().angle();
        int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0,
                dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

        if (((a1 <= a2) && !r1.End()) || r2.End()) {
            cerr << "Concavities2/r1: Comparing " << r1.Cur().ToString()
                    << " (hull) / " << reg1->Cur().ToString() << " (region)\n";
            if (r1.Cur() == reg1->Cur()) {
                r1.Next();
                reg1->Next();
            } else {

                // We found a concavity in the source region

                cerr << "Concavities2: Found concavity\n";
                Reg ccv; // The concavity
                while (r1.Cur().x2 != reg1->Cur().x1 ||
                        r1.Cur().y2 != reg1->Cur().y1) {
                    sx1 = reg1->Cur().x1;
                    sy1 = reg1->Cur().y1;
                    sx2 = reg1->Cur().x2;
                    sy2 = reg1->Cur().y2;
                    dx1 = dx2 = r2.Cur().x1;
                    dy1 = dy2 = r2.Cur().y1;
                    reg1->Next();
                    //    msegs.AddMSeg(sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);
                    Seg s(sx1, sy1, sx2, sy2);
                    ccv.AddSeg(s);
                    cerr << "Concavities2: Adding segment " << s.ToString()
                            << "\n";
                }
                ccv.hullPoint = new Pt(reg1->Cur().x1, reg1->Cur().y1);
                ccv.peerPoint = new Pt(r2.Cur().x1, r2.Cur().y1);
                cerr << "HP" << ccv.hullPoint->ToString()
                        << " PP " << ccv.peerPoint->ToString() << "\n\n";
                cerr << "Concavities2: Found concavity end\n\n";
                ccv.Close();
                ret.push_back(ccv);
                r1.Next();
            }
        } else if ((a1 >= a2) || r1.End()) {
            cerr << "Concavities2/r2: Comparing " << r2.Cur().ToString()
                    << " (hull) / " << reg2->Cur().ToString() << " (region)\n";
            if (r2.Cur() == reg2->Cur()) {
                reg2->Next();
                r2.Next();
            } else {
                while (r2.Cur().x2 != reg2->Cur().x1 ||
                        r2.Cur().y2 != reg2->Cur().y1) {
                    reg2->Next();
                }
                r2.Next();
            }
        }

        if (r1.End() && r2.End())
            break;

    } while (1);
    cerr << "\nConcavities2: END\n\n";

    return ret;
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

Seg Reg::Next() {
    cur++;
    //    if (cur >= v.size())
    //        cur = 0;

    return Cur();
}

int Reg::End() {
    if (cur >= (int) v.size())
        return 1;
    return 0;
}

Seg Reg::Prev() {
    cur--;
    if (cur < 0)
        cur = v.size() - 1;

    return v[cur];
}

Seg Reg::Cur() {
    return v[cur % v.size()];
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

RotatingPlane::RotatingPlane(Reg *reg1, Reg *reg2) {
    MSegs msegs;

    Reg r1 = Reg(reg1->convexhull);
    Reg r2 = Reg(reg2->convexhull);

    do {
        double a1 = r1.Cur().angle();
        double a2 = r2.Cur().angle();
        int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0,
                dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

        if (((a1 <= a2) && !r1.End()) || r2.End()) {
            sx1 = r1.Cur().x1;
            sy1 = r1.Cur().y1;
            sx2 = r1.Cur().x2;
            sy2 = r1.Cur().y2;
            dx1 = dx2 = r2.Cur().x1;
            dy1 = dy2 = r2.Cur().y1;
            msegs.AddMSeg(sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);
            if (!(r1.Cur() == reg1->Cur())) {

                // We found a concavity in the source region
                Reg ccv;

                while (r1.Cur().x2 != reg1->Cur().x1 ||
                        r1.Cur().y2 != reg1->Cur().y1) {
                    sx1 = reg1->Cur().x1;
                    sy1 = reg1->Cur().y1;
                    sx2 = reg1->Cur().x2;
                    sy2 = reg1->Cur().y2;
                    dx1 = dx2 = r2.Cur().x1;
                    dy1 = dy2 = r2.Cur().y1;
                    Seg s(sx1, sy1, sx2, sy2);
                    ccv.AddSeg(s);
                    reg1->Next();
                }
                ccv.hullPoint = new Pt(reg1->Cur().x1, reg1->Cur().y1);
                ccv.peerPoint = new Pt(r2.Cur().x1, r2.Cur().y1);
                ccv.Close();
                scvs.push_back(ccv);
                
                r1.Next();
            } else {
                r1.Next();
                reg1->Next();
            }
        } else if ((a1 >= a2) || r1.End()) {
            sx1 = sx2 = r1.Cur().x1;
            sy1 = sy2 = r1.Cur().y1;
            dx1 = r2.Cur().x1;
            dy1 = r2.Cur().y1;
            dx2 = r2.Cur().x2;
            dy2 = r2.Cur().y2;
            msegs.AddMSeg(sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);
            if (!(r2.Cur() == reg2->Cur())) {

                // We found a concavity in the destination region
                Reg ccv;

                while (r2.Cur().x2 != reg2->Cur().x1 ||
                        r2.Cur().y2 != reg2->Cur().y1) {
                    sx1 = sx2 = r1.Cur().x1;
                    sy1 = sy2 = r1.Cur().y1;
                    dx1 = reg2->Cur().x1;
                    dy1 = reg2->Cur().y1;
                    dx2 = reg2->Cur().x2;
                    dy2 = reg2->Cur().y2;
                    Seg s(dx1, dy1, dx2, dy2);
                    ccv.AddSeg(s);
                    reg2->Next();
                }
                ccv.hullPoint = new Pt(reg2->Cur().x1, reg2->Cur().y1);
                ccv.peerPoint = new Pt(r1.Cur().x1, r1.Cur().y1);
                ccv.Close();
                dcvs.push_back(ccv);
                r2.Next();
            } else {
                reg2->Next();
                r2.Next();
            }
        }

        if (r1.End() && r2.End())
            break;

    } while (1);

    face = MFace(msegs);
}