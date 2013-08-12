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

//static vector<pair<Reg *, Reg *> > matchFaces(vector<Reg> *src,
//        vector<Reg> *dst) {
//    vector<pair<Reg *, Reg *> > ret;
//
//    int minsx = INT_MAX;
//    int minsy = INT_MAX;
//    int mindx = INT_MAX;
//    int mindy = INT_MAX;
//
//    for (unsigned int i = 0; i < src->size(); i++) {
//        Pt p = (*src)[i].GetMinXY();
//        if (p.x < minsx)
//            minsx = p.x;
//        if (p.y < minsy)
//            minsy = p.y;
//    }
//    for (unsigned int i = 0; i < dst->size(); i++) {
//        Pt p = (*dst)[i].GetMinXY();
//        if (p.x < mindx)
//            mindx = p.x;
//        if (p.y < mindy)
//            mindy = p.y;
//    }
//
//    vector<Region> sregs, dregs;
//
//    for (unsigned int i = 0; i < src->size(); i++) {
//        Region r = (*src)[i].MakeRegion(-minsx, -minsy);
//        sregs.push_back(r);
//    }
//    for (unsigned int i = 0; i < dst->size(); i++) {
//        Region r = (*dst)[i].MakeRegion(-mindx, -mindy);
//        dregs.push_back(r);
//    }
//
//    for (unsigned int i = 0; i < sregs.size(); i++) {
//        for (unsigned int j = 0; j < dregs.size(); j++) {
//            Region sr = sregs[i];
//            Region dr = dregs[j];
//            //          cerr << "Intersecting " << nl->ToString(OutRegion(sr))
//            //            << " with " << nl->ToString(OutRegion(dr)) << "\n";
//            cerr << "Intersecting " << sr << " with " << dr << "\n";
//            Region ir(0);
//            sr.Intersection(dr, ir, NULL);
//            ir.ComputeRegion();
//            cerr << "Size Sregion " << sr.SpatialSize() << " Dregion " <<
//                    dr.SpatialSize() << "\n";
//            cerr << "IRegion " << ir << " / " << ir.IsDefined() << "\n";
//            cerr << "SRegion " << i << " intersects DRegion " << j << " by "<<
//                    ir.SpatialSize() << "\n";
//        }
//    }
//
//
//    unsigned int i;
//    for (i = 0; i < src->size(); i++) {
//        if (i < dst->size()) {
//            pair<Reg *, Reg *> p(&((*src)[i]), &((*dst)[dst->size() - i -1]));
//            ret.push_back(p);
//        } else {
//            pair<Reg *, Reg *> p(&((*src)[i]), NULL);
//            ret.push_back(p);
//        }
//    }
//
//    while (i < dst->size()) {
//        pair<Reg *, Reg *> p(NULL, &((*dst)[i]));
//        ret.push_back(p);
//        i++;
//    }
//
//    return ret;
//}
//
static vector<pair<Reg *, Reg *> > matchFacesSimple(vector<Reg> *src,
        vector<Reg> *dst) {
    vector<pair<Reg *, Reg *> > ret;

    for (unsigned int i = 0; (i < src->size() || (i < dst->size())); i++) {
        if ((i < src->size()) && (i < dst->size())) {
            pair<Reg *, Reg *> p(&((*src)[i]), &((*dst)[i]));
            ret.push_back(p);
        } else if (i < src->size()) {
            pair<Reg *, Reg *> p(&((*src)[i]), NULL);
            ret.push_back(p);
        } else {
            pair<Reg *, Reg *> p(NULL, &((*dst)[i]));
            ret.push_back(p);
        }
    }

    return ret;
}
//
//static vector<pair<Reg *, Reg *> > matchFacesNull(vector<Reg> *src,
//        vector<Reg> *dst) {
//    vector<pair<Reg *, Reg *> > ret;
//
//    for (unsigned int i = 0; i < src->size(); i++) {
//        pair<Reg *, Reg *> p(&((*src)[i]), NULL);
//        ret.push_back(p);
//    }
//    for (unsigned int i = 0; i < dst->size(); i++) {
//        pair<Reg *, Reg *> p(NULL, &((*dst)[i]));
//        ret.push_back(p);
//    }
//
//    return ret;
//}

static vector<pair<Reg *, Reg *> > matchFacesDistance(vector<Reg> *src,
        vector<Reg> *dst) {
    vector<pair<Reg *, Reg *> > ret;
    
    for (unsigned int i = 0; i < src->size(); i++) {
        (*src)[i].used = 0;
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        (*dst)[i].used = 0;
    }
    
    Pt srcoff = Reg::GetMinXY(*src);
    Pt dstoff = Reg::GetMinXY(*dst);

    if (src->size() >= dst->size()) {
        for (unsigned int i = 0; i < dst->size(); i++) {
            int dist = 2000000000;
            int candidate = -1;
            for (unsigned int j = 0; j < src->size(); j++) {
                if ((*src)[j].used == 1)
                    continue;
                Pt srcm = (*src)[j].GetMiddle() - srcoff;
                Pt dstm = (*dst)[i].GetMiddle() - dstoff;
                int d2 = dstm.distance(srcm);
                if (d2 < dist) {
                    dist = d2;
                    candidate = j;
                }
            }
            pair<Reg *, Reg *> p(&(*src)[candidate], &(*dst)[i]);
            ret.push_back(p);
            (*src)[candidate].used = 1;
        }
        for (unsigned int j = 0; j < src->size(); j++) {
            if ((*src)[j].used)
                continue;
            pair<Reg *, Reg *> p(&(*src)[j], NULL);
            ret.push_back(p);
        }
    } else {
        for (unsigned int i = 0; i < src->size(); i++) {
            int dist = 2000000000;
            int candidate = -1;
            for (unsigned int j = 0; j < dst->size(); j++) {
                if ((*dst)[j].used == 1)
                    continue;
                Pt srcm = (*src)[i].GetMiddle() - srcoff;
                Pt dstm = (*dst)[j].GetMiddle() - dstoff;
                int d2 = dstm.distance(srcm);
                if (d2 < dist) {
                    dist = d2;
                    candidate = j;
                }
            }
            pair<Reg *, Reg *> p(&(*src)[i], &(*dst)[candidate]);
            ret.push_back(p);
            (*dst)[candidate].used = 1;
        }
        for (unsigned int j = 0; j < dst->size(); j++) {
            if ((*dst)[j].used)
                continue;
            pair<Reg *, Reg *> p(NULL, &(*dst)[j]);
            ret.push_back(p);
        }

    }

    return ret;
}

MFaces interpolate(vector<Reg> *sregs, Instant *ti1,
        vector<Reg> *dregs, Instant *ti2, int mode) {
    
    vector<pair<Reg *, Reg *> > ps = matchFacesDistance(sregs, dregs);
    MFaces ret, fcs;

    for (unsigned int i = 0; i < ps.size(); i++) {
        pair<Reg *, Reg *> p = ps[i];

        Reg *src = p.first;
        Reg *dst = p.second;

        if (src && dst) {
            RotatingPlane rp(src, dst);
            fcs = interpolate(&rp.scvs, ti1, &rp.dcvs, ti2, mode);
            
//            for (unsigned int i = 0; i < fcs.faces.size(); i++) {
//                for (unsigned int j = i+1; j < fcs.faces.size(); j++) {
//                    MSegs *s1 = &fcs.faces[i].face;
//                    MSegs *s2 = &fcs.faces[j].face;
//                    if (s1->ignore || s2->ignore || !mode)
//                        continue;
//                    if (s1->intersects(*s2) && mode) {
//                        pair<MSegs, MSegs> ss = s1->kill();
//                cerr << "Intersection found: " << ss.first.ToString() << "\n"
//                                << ss.second.ToString() << "\n";
//                        rp.face.AddMsegs(ss.first);
//                        rp.face.AddMsegs(ss.second);
//                        s1->ignore = 1;
//                    }
//                }
//            }
            
            for (unsigned int i = 0; i < fcs.faces.size(); i++) {
//                if (fcs.faces[i].face.ignore)
//                    continue;
                rp.face.AddMsegs(fcs.faces[i].face);
                for (unsigned int j = 0; j < fcs.faces[i].holes.size(); j++) {
                    MFace fc(fcs.faces[i].holes[j]);
                    ret.AddFace(fc);
                }
            }
            ret.AddFace(rp.face);
        } else {
            if (dst) {
                MSegs coll = dst->collapse(false);
                MFace f(coll);
                ret.AddFace(f);
            } else {
                MSegs coll = src->collapse(true);
                MFace f(coll);
                ret.AddFace(f);
            }
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

    MFaces mf = interpolate(&reg1, ti1, &reg2, ti2, mode->GetIntval());
    cerr << mf.ToString();
    MRegion mreg = mf.ToMRegion(iv);
    *m = mreg;
    
    return 0;
}

