
/*
*/

#include "MovingRegionAlgebra.h"
#include "SpatialAlgebra.h"
#include "GenOps.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "DateTime.h"
#include "interpolate.h"

#include <string>

static int curface;


//MSegmentData tS (MSegmentData ms, double off) {
//    MSegmentData r = MSegmentData(
//            ms.GetFaceNo()+1,
//            ms.GetCycleNo(),
//            ms.GetSegmentNo(),
//            ms.GetInsideAbove(),
//            
//            ms.GetInitialStartX() + off,
//            ms.GetInitialStartY() + off,
//            ms.GetInitialEndX() + off,
//            ms.GetInitialEndY() + off,
//            ms.GetFinalStartX() + off,
//            ms.GetFinalStartY() + off,
//            ms.GetFinalEndX() + off,
//            ms.GetFinalEndY() + off
//            );
//    
//    return r;
//}


static vector<MSegmentData> makeConcavities(Reg *r1, bool close) {
    vector<MSegmentData> ret;

    vector<Seg> hull = sortSegs(r1->convexhull);
    vector<Seg> poly = sortSegs(r1->v);

    int i = 0;
    int j = 0;
    int c = 0;
    MSegmentData ms;

    do {
        if (hull[i] == poly[j]) {
            ms = MSegmentData(
                    curface, 0, c++, false,
                    hull[i].x1, hull[i].y1, hull[i].x2, hull[i].y2,
                    hull[i].x1, hull[i].y1, hull[i].x2, hull[i].y2
                    );
            ret.push_back(ms);
            j = (j + 1) % poly.size();
            i++;
        } else {
            if (close)
                ms = MSegmentData(
                    curface, 0, c++, false,
                    poly[j].x1, poly[j].y1, poly[j].x2, poly[j].y2,
                    hull[i].x1, hull[i].y1, hull[i].x1, hull[i].y1
                    );
            else
                ms = MSegmentData(
                    curface, 0, c++, false,
                    hull[i].x1, hull[i].y1, hull[i].x1, hull[i].y1,
                    poly[j].x1, poly[j].y1, poly[j].x2, poly[j].y2
                    );
            ret.push_back(ms);
            if (hull[i].x2 == poly[j].x2 && hull[i].y2 == poly[j].y2) {
                if (close)
                    ms = MSegmentData(
                        curface, 0, c++, false,
                        hull[i].x2, hull[i].y2, hull[i].x2, hull[i].y2,
                        hull[i].x1, hull[i].y1, hull[i].x2, hull[i].y2
                        );
                else
                    ms = MSegmentData(
                        curface, 0, c++, false,
                        hull[i].x1, hull[i].y1, hull[i].x2, hull[i].y2,
                        hull[i].x2, hull[i].y2, hull[i].x2, hull[i].y2
                        );
                ret.push_back(ms);
                i++;
            }
            j = (j + 1) % poly.size();
        }
        //        i = (i + 1)%hull.size();
    } while (i < (int) hull.size());

    return ret;
}

static URegion collapseRegion(Reg *reg, Instant *ti1, Instant *ti2, bool close){
    vector<MSegmentData> v = vector<MSegmentData > (0);
    int i = 0;

    int x = reg->Cur().x1;
    int y = reg->Cur().y1;

    do {
        int sx1 = reg->Cur().x1;
        int sy1 = reg->Cur().y1;
        int sx2 = reg->Cur().x2;
        int sy2 = reg->Cur().y2;
        MSegmentData ms;
        if (close) {
            ms = MSegmentData(
                    curface, 0, i++, false,
                    sx1, sy1, sx2, sy2,
                    x, y, x, y
                    );
        } else {
            ms = MSegmentData(
                    curface, 0, i++, false,
                    x, y, x, y,
                    sx1, sy1, sx2, sy2
                    );
        }
        v.push_back(ms);

        reg->Next();
        if (reg->End())
            break;
    } while (1);

    Interval<Instant> iv = Interval<Instant > (*ti1, *ti2, true, true);
    URegion ret(v, iv);
    curface++;

    return ret;
}

static vector<MSegmentData> createURegion(Reg r1, Reg r2) {
    vector<MSegmentData> v = vector<MSegmentData > (0);
    int i = 0;

    do {
        double a1 = r1.Cur().angle();
        double a2 = r2.Cur().angle();
        int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0,
                dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

        if (((a1 < a2) && !r1.End()) || r2.End()) {
            sx1 = r1.Cur().x1;
            sy1 = r1.Cur().y1;
            sx2 = r1.Cur().x2;
            sy2 = r1.Cur().y2;
            dx1 = dx2 = r2.Cur().x1;
            dy1 = dy2 = r2.Cur().y1;
            r1.Next();
        } else if ((a1 > a2) || r1.End()) {
            sx1 = sx2 = r1.Cur().x1;
            sy1 = sy2 = r1.Cur().y1;
            dx1 = r2.Cur().x1;
            dy1 = r2.Cur().y1;
            dx2 = r2.Cur().x2;
            dy2 = r2.Cur().y2;
            r2.Next();
        } else if (a1 == a2) { // Segments are collinear
            sx1 = r1.Cur().x1;
            sy1 = r1.Cur().y1;
            sx2 = r1.Cur().x2;
            sy2 = r1.Cur().y2;
            dx1 = r2.Cur().x1;
            dy1 = r2.Cur().y1;
            dx2 = r2.Cur().x2;
            dy2 = r2.Cur().y2;
            r1.Next();
            r2.Next();
        }

        MSegmentData ms = MSegmentData(
                curface, 0, i++, false,
                sx1, sy1, sx2, sy2,
                dx1, dy1, dx2, dy2
                );
        int found = 0;
        for (unsigned int j = 0; j < v.size(); j++) {
            if (
                    (v[j].GetInitialStartX() == ms.GetInitialStartX()) &&
                    (v[j].GetFinalStartX() == ms.GetFinalStartX()) &&
                    (v[j].GetInitialStartY() == ms.GetInitialStartY()) &&
                    (v[j].GetFinalStartY() == ms.GetFinalStartY()) &&
                    (v[j].GetInitialEndX() == ms.GetInitialEndX()) &&
                    (v[j].GetFinalEndX() == ms.GetFinalEndX()) &&
                    (v[j].GetInitialEndY() == ms.GetInitialEndY()) &&
                    (v[j].GetFinalEndY() == ms.GetFinalEndY())) {
                found = 1;
                break;
            }
        }
        if (found)
            break;

        v.push_back(ms);

        if (r1.End() && r2.End())
            break;

    } while (1);

    return v;
}

void translate(vector<MSegmentData> &v, double offx, double offy, bool source) {
    for (unsigned int i = 0; i < v.size(); i++) {
        if (source) {
//            v[i].SetInitialStartX(v[i].GetInitialStartX() + offx);
//            v[i].SetInitialStartY(v[i].GetInitialStartY() + offy);
//            v[i].SetInitialEndX(v[i].GetInitialEndX() + offx);
//            v[i].SetInitialEndY(v[i].GetInitialEndY() + offy);
        } else {
            v[i].SetFinalStartX(v[i].GetFinalStartX() + offx);
            v[i].SetFinalStartY(v[i].GetFinalStartY() + offy);
            v[i].SetFinalEndX(v[i].GetFinalEndX() + offx);
            v[i].SetFinalEndY(v[i].GetFinalEndY() + offy);
        }
    }
}

vector<URegion> interpolate1(Reg *reg1, Instant *ti1, Reg *reg2, Instant *ti4) {
    vector<URegion> ret;
    Reg reg1hull = reg1->convexhull;
    Reg reg2hull = reg2->convexhull;

    int distx = reg2->v[0].x1 - reg1->v[0].x1;
    int disty = reg2->v[0].y1 - reg1->v[0].y1;

    vector<MSegmentData> v1 = makeConcavities(reg1, true);
    translate(v1, distx / 3, disty / 3, false);
    reg1hull.Translate(distx / 3, disty / 3);
    reg2hull.Translate(-distx / 3, -disty / 3);
    vector<MSegmentData> v2 = createURegion(reg1hull, reg2hull);
    vector<MSegmentData> v3 = makeConcavities(reg2, false);
    translate(v3, -distx / 3, -disty / 3, true);


    int64_t x = (ti4->millisecondsToNull() - ti1->millisecondsToNull()) / 3;
    Instant ti2 = Instant(ti1->millisecondsToNull() + x);
    Instant ti3 = Instant(ti1->millisecondsToNull() + 2 * x);
    Interval<Instant> int1 = Interval<Instant > (*ti1, ti2, true, false);
    Interval<Instant> int2 = Interval<Instant > (ti2, ti3, true, false);
    Interval<Instant> int3 = Interval<Instant > (ti3, *ti4, true, true);

    URegion oc = URegion(v1, int1);
    URegion ch = URegion(v2, int2);
    URegion cc = URegion(v3, int3);
    ret.push_back(oc);
    ret.push_back(ch);
    ret.push_back(cc);
    curface++;

    return ret;
}

//static bool compareSegments (MSegmentData m1, MSegmentData m2) {
//        return (
//                    (m1.GetInitialStartX() == m2.GetInitialStartX()) &&
//                    (m1.GetFinalStartX() == m2.GetFinalStartX()) &&
//                    (m1.GetInitialStartY() == m2.GetInitialStartY()) &&
//                    (m1.GetFinalStartY() == m2.GetFinalStartY()) &&
//                    (m1.GetInitialEndX() == m2.GetInitialEndX()) &&
//                    (m1.GetFinalEndX() == m2.GetFinalEndX()) &&
//                    (m1.GetInitialEndY() == m2.GetInitialEndY()) &&
//                    (m1.GetFinalEndY() == m2.GetFinalEndY())
//                );
//}


MFace rotatingPlane(Reg *reg1, Reg *reg2, bool hullOnly) {
    MSegs msegs;

    Reg r1 = Reg(reg1->convexhull);
    Reg r2 = Reg(reg2->convexhull);

    do {
        double a1 = r1.Cur().angle();
        double a2 = r2.Cur().angle();
        int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0,
                dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

        if (((a1 <= a2) && !r1.End()) || r2.End()) {
            if (r1.Cur() == reg1->Cur() || hullOnly) {
                sx1 = r1.Cur().x1;
                sy1 = r1.Cur().y1;
                sx2 = r1.Cur().x2;
                sy2 = r1.Cur().y2;
                dx1 = dx2 = r2.Cur().x1;
                dy1 = dy2 = r2.Cur().y1;
                msegs.AddMSeg(sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);
                r1.Next();
                reg1->Next();
            } else {
                while (r1.Cur().x2 != reg1->Cur().x1 &&
                        r1.Cur().y2 != reg1->Cur().y1) {
                    sx1 = reg1->Cur().x1;
                    sy1 = reg1->Cur().y1;
                    sx2 = reg1->Cur().x2;
                    sy2 = reg1->Cur().y2;
                    dx1 = dx2 = r2.Cur().x1;
                    dy1 = dy2 = r2.Cur().y1;
                    reg1->Next();
                    msegs.AddMSeg(sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);
                }
                r1.Next();
            }
        } else if ((a1 >= a2) || r1.End()) {
            if (r2.Cur() == reg2->Cur() || hullOnly) {
                sx1 = sx2 = r1.Cur().x1;
                sy1 = sy2 = r1.Cur().y1;
                dx1 = r2.Cur().x1;
                dy1 = r2.Cur().y1;
                dx2 = r2.Cur().x2;
                dy2 = r2.Cur().y2;
                msegs.AddMSeg(sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);
                reg2->Next();
                r2.Next();
            } else {
                while (r2.Cur().x2 != reg2->Cur().x1 && 
                        r2.Cur().y2 != reg2->Cur().y1) {
                    sx1 = sx2 = r1.Cur().x1;
                    sy1 = sy2 = r1.Cur().y1;
                    dx1 = reg2->Cur().x1;
                    dy1 = reg2->Cur().y1;
                    dx2 = reg2->Cur().x2;
                    dy2 = reg2->Cur().y2;
                    msegs.AddMSeg(sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);
                    reg2->Next();
                }
                r2.Next();
            }
        }

        if (r1.End() && r2.End())
            break;

    } while (1);
    
    MFace ret = MFace(msegs);
    return ret;
}

vector<URegion> interpolate2(Reg *reg1, Instant *ti1, Reg *reg2, Instant *ti2) {
    vector<URegion> ret;

    MSegs _v  = rotatingPlane(reg1, reg2, true);
    MSegs _v2 = rotatingPlane(&reg1->cvs[0], &reg2->cvs[0], true);
    cerr << "Msegs _v" << _v.ToString();
    cerr << "Msegs _v2" << _v2.ToString();
    MFace face(_v);
    face.AddHole(_v2);
    vector<MSegmentData> v = _v.ToMSegmentData(curface, 0);
    vector<MSegmentData> v2 = _v2.ToMSegmentData(curface, 0);
    //    v.insert(v.end(), v2.begin(), v2.end());

    Interval<Instant> iv = Interval<Instant > (*ti1, *ti2, true, true);
    URegion u = URegion(v, iv);
    ret.push_back(u);
    curface++;

    return ret;
}

vector<URegion> do_interpolate(Reg *src, Instant *ti1, Reg *dst, Instant *ti2,
        int mode) {
    vector<URegion> ret;
    if (src == NULL) {
        URegion r = collapseRegion(dst, ti1, ti2, false);
        ret.push_back(r);
    } else if (dst == NULL) {
        URegion r = collapseRegion(src, ti1, ti2, true);
        ret.push_back(r);
    } else {
        if (mode == 1) {
            ret = interpolate1(src, ti1, dst, ti2);
        } else if (mode == 2) {
            ret = interpolate2(src, ti1, dst, ti2);
        } else {
            ret = interpolate2(src, ti1, dst, ti2);
        }
    }

    return ret;
}

static vector<pair<Reg *, Reg *> > matchFaces(vector<Reg> *src,
        vector<Reg> *dst) {
    vector<pair<Reg *, Reg *> > ret;

    int minsx = INT_MAX;
    int minsy = INT_MAX;
    int mindx = INT_MAX;
    int mindy = INT_MAX;

    for (unsigned int i = 0; i < src->size(); i++) {
        Pt p = (*src)[i].GetMinXY();
        if (p.x < minsx)
            minsx = p.x;
        if (p.y < minsy)
            minsy = p.y;
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        Pt p = (*dst)[i].GetMinXY();
        if (p.x < mindx)
            mindx = p.x;
        if (p.y < mindy)
            mindy = p.y;
    }

    vector<Region> sregs, dregs;

    for (unsigned int i = 0; i < src->size(); i++) {
        Region r = (*src)[i].MakeRegion(-minsx, -minsy);
        sregs.push_back(r);
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        Region r = (*dst)[i].MakeRegion(-mindx, -mindy);
        dregs.push_back(r);
    }

    for (unsigned int i = 0; i < sregs.size(); i++) {
        for (unsigned int j = 0; j < dregs.size(); j++) {
            Region sr = sregs[i];
            Region dr = dregs[j];
            //            cerr << "Intersecting " << nl->ToString(OutRegion(sr))
            //            << " with " << nl->ToString(OutRegion(dr)) << "\n";
            cerr << "Intersecting " << sr << " with " << dr << "\n";
            Region ir(0);
            sr.Intersection(dr, ir, NULL);
            ir.ComputeRegion();
            cerr << "Size Sregion " << sr.SpatialSize() << " Dregion " << 
                    dr.SpatialSize() << "\n";
            cerr << "IRegion " << ir << " / " << ir.IsDefined() << "\n";
            cerr << "SRegion " << i << " intersects DRegion " << j << " by " <<
                    ir.SpatialSize() << "\n";
        }
    }


    unsigned int i;
    for (i = 0; i < src->size(); i++) {
        if (i < dst->size()) {
            pair<Reg *, Reg *> p(&((*src)[i]), &((*dst)[dst->size() - i - 1]));
            ret.push_back(p);
        } else {
            pair<Reg *, Reg *> p(&((*src)[i]), NULL);
            ret.push_back(p);
        }
    }

    while (i < dst->size()) {
        pair<Reg *, Reg *> p(NULL, &((*dst)[i]));
        ret.push_back(p);
        i++;
    }

    return ret;
}

static vector<pair<Reg *, Reg *> > matchFacesSimple(vector<Reg> *src,
        vector<Reg> *dst) {
    vector<pair<Reg *, Reg *> > ret;
    
    unsigned int i;
    
    for (i = 0; (i < src->size() || (i < dst->size())); i++) {
        if ((i < src->size()) && (i < dst->size())) {
            pair<Reg *, Reg *> p(&((*src)[i]), &((*dst)[i]));
            ret.push_back(p);
        } else if (i < src->size()) {
            pair<Reg *, Reg *> p(&((*src)[i]), NULL);
            ret.push_back(p);
        } else {
            pair<Reg *, Reg *> p(&((*dst)[i]), NULL);
            ret.push_back(p);
        }
    }    
    
    return ret;
}

URegion interpolate3(Reg *reg1, Instant *ti1, Reg *reg2, Instant *ti2) {
    MSegs _v  = rotatingPlane(reg1, reg2, true);
    MSegs _v2 = rotatingPlane(&reg1->cvs[0], &reg2->cvs[0], true);
    MSegs _v3 = rotatingPlane(&reg1->cvs[1], &reg2->cvs[1], true);
    cerr << "Msegs _v" << _v.ToString();
    cerr << "Msegs _v2" << _v2.ToString();
    MFace face(_v);
    face.AddHole(_v2);
    if (_v3.intersects(_v2)) {
        cerr << "Concavity intersects, ignoring!" << "\n";
        face.AddHole(_v3);
    } else {
        cerr << "Concavity not intersects, not ignoring!\n";
        face.AddHole(_v3);
    }

    Interval<Instant> iv = Interval<Instant > (*ti1, *ti2, true, true);
    URegion ret = face.ToURegion(iv);

    return ret;
}

vector<MSegs> interpolate4 (vector<Reg> *sregs, Instant *ti1,
			    vector<Reg> *dregs, Instant *ti2) {
    vector<pair<Reg *, Reg *> > ps = matchFacesSimple(sregs, dregs);
    vector<MSegs> ret, msegs;
    
    for (unsigned int i = 0; i < ps.size(); i++) {
        pair<Reg *, Reg *> p = ps[i];
        
        Reg *src = p.first;
        Reg *dst = p.second;
        
        if (src && dst) {
            vector<Reg> scvs = src->Concavities();
            vector<Reg> dcvs = dst->Concavities();
	    Reg scvx(src->convexhull);
	    Reg dcvx(dst->convexhull);
            MFace f = rotatingPlane(&scvx, &dcvx, true);
            msegs = interpolate4(&scvs, ti1, &dcvs, ti2);
            for (unsigned int i = 0; i < msegs.size(); i++) {
                f.AddHole(msegs[i]);
            }
            
        } else {
            if (dst) {
                src = dst;
            }
            MSegs coll = src->collapse();
        }
    }
    
    return ret;
}

int interpolatevalmap(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s) {
    result = qp->ResultStorage(s);

    Instant* ti1 = static_cast<Instant*> (args[1].addr);
    Instant* ti2 = static_cast<Instant*> (args[3].addr);
    CcInt* mode = static_cast<CcInt*> (args[4].addr);
    MRegion* m = static_cast<MRegion*> (result.addr);

    Interval<Instant> iv(*ti1, *ti2, true, true);
    
    ListExpr _r1 = OutRegion(nl->Empty(), args[0]);
    ListExpr _r2 = OutRegion(nl->Empty(), args[2]);

    vector<Reg> reg1 = Reg::getRegs(_r1);
    vector<Reg> reg2 = Reg::getRegs(_r2);

    interpolate4(&reg1, ti1, &reg2, ti2);
    MFaces mf;
    *m = mf.ToMRegion(iv);

    // Merge URegions per time-interval
//    std::map<Interval<Instant>, URegion*> map;
//    for (unsigned int i = 0; i < uregs.size(); i++) {
//        Interval<Instant> t = uregs[i].timeInterval;
//        if (map.find(t) == map.end()) {
//            map[t] = &uregs[i];
//        } else {
//            map[t]->AddURegion(&uregs[i]);
//        }
//    }
//
//    // Assemble MRegion from URegions
//    *m = MRegion(1);
//    for (std::map<Interval<Instant>, URegion *>::iterator p = map.begin();
//         p != map.end(); p++) {
//        m->AddURegion(*p->second);
//    }

    return 0;
}

