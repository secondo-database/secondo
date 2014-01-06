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

vector<pair<Reg *, Reg *> > _matchFacesLua(vector<Reg> *src, vector<Reg> *dst,
        int depth);

static vector<pair<Reg *, Reg *> > matchFacesLua(vector<Reg> *src,
        vector<Reg> *dst, int depth) {
    return _matchFacesLua(src, dst, depth);
}

static vector<pair<Reg *, Reg *> > matchFacesSimple(vector<Reg> *src,
        vector<Reg> *dst, int depth) {
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

static vector<pair<Reg *, Reg *> > matchFacesNull(vector<Reg> *src,
        vector<Reg> *dst, int depth) {
    vector<pair<Reg *, Reg *> > ret;

    for (unsigned int i = 0; i < src->size(); i++) {
        pair<Reg *, Reg *> p(&((*src)[i]), NULL);
        ret.push_back(p);
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        pair<Reg *, Reg *> p(NULL, &((*dst)[i]));
        ret.push_back(p);
    }

    return ret;
}

static vector<pair<Reg *, Reg *> > matchFacesDistance(vector<Reg> *src,
        vector<Reg> *dst, int depth) {
    vector<pair<Reg *, Reg *> > ret;
    
    for (unsigned int i = 0; i < src->size(); i++) {
        (*src)[i].used = 0;
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        (*dst)[i].used = 0;
    }
    
    Pt srcoff = Reg::GetBoundingBox(*src).first;
    Pt dstoff = Reg::GetBoundingBox(*dst).first;

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
        vector<Reg> *dregs, Instant *ti2, int depth) {
    MFaces ret;
    
    ret.sregs = sregs;
    ret.dregs = dregs;
    
    if (sregs->empty() && dregs->empty())
        return ret;
    
    for (unsigned int i = 0; i < sregs->size(); i++) {
        (*sregs)[i].isdst = 0;
    }
    for (unsigned int i = 0; i < dregs->size(); i++) {
        (*dregs)[i].isdst = 1;
    }
    
    vector<pair<Reg *, Reg *> > ps;
    
    ps = matchFacesLua(sregs, dregs, depth);
    MFaces fcs;

    for (unsigned int i = 0; i < ps.size(); i++) {
        pair<Reg *, Reg *> p = ps[i];

        Reg *src = p.first;
        Reg *dst = p.second;

        if (src && dst) {
            RotatingPlane rp(src, dst);
            fcs = interpolate(&rp.scvs, ti1, &rp.dcvs, ti2, depth+1);
            
            for (int i = 0; i < (int) fcs.faces.size(); i++) {
                MSegs *s1 = &fcs.faces[i].face;
                if (s1->ignore)
                    continue;
                for (unsigned int j = i+1; j < fcs.faces.size(); j++) {
                    MSegs *s2 = &fcs.faces[j].face;
                    if (s2->ignore)
                        continue;
                    if (s1->intersects(*s2)) {
                        pair<MSegs, MSegs> ss;
                        if (!s1->iscollapsed) {
                            ss = s1->kill();
                            s1->ignore = 1;
                        } else if (!s2->iscollapsed) {
                            ss = s2->kill();
                            s2->ignore = 1;
                        } else {
                            cerr << "Intersection found, but cannot "
                                    "compensate!\n";
                            continue;
                        }
                        cerr << "Intersection found: " << ss.first.ToString()
                             << "\n" << ss.second.ToString() << "\n";
                        fcs.faces.push_back(ss.first);
                        fcs.faces.push_back(ss.second);
                        i = -1;
                        break;
                    }
                }
            }
            
            for (unsigned int i = 0; i < fcs.faces.size(); i++) {
                if (fcs.faces[i].face.ignore)
                    continue;
                rp.face.AddMsegs(fcs.faces[i].face);
                for (unsigned int j = 0; j < fcs.faces[i].holes.size(); j++) {
                    MFace fc(fcs.faces[i].holes[j]);
                    ret.AddFace(fc);
                }
            }
            rp.face.MergeConcavities();
            ret.AddFace(rp.face);
        } else {
            Reg *r = src ? src : dst;
            MSegs coll = r->collapse(r == src);
            MFace f(coll);
            ret.AddFace(f);
        }
    }

    return ret;
}

Word InMRegion(const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct);

int interpolatevalmap(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s) {
    result = qp->ResultStorage(s);

    Instant* ti1 = static_cast<Instant*> (args[1].addr);
    Instant* ti2 = static_cast<Instant*> (args[3].addr);
    CcReal* mode = static_cast<CcReal*> (args[4].addr);
    MRegion* m = static_cast<MRegion*> (result.addr);

    Interval<Instant> iv(*ti1, *ti2, true, true);

    ListExpr _r1 = OutRegion(nl->Empty(), args[0]);
    ListExpr _r2 = OutRegion(nl->Empty(), args[2]);

    vector<Reg> reg1 = Reg::getRegs(_r1);
    vector<Reg> reg2 = Reg::getRegs(_r2);

    MFaces mf = interpolate(&reg1, ti1, &reg2, ti2, 0);
    cerr << mf.ToString();

    ListExpr mreg = mf.ToMListExpr(iv);
    nl->WriteListExpr(mreg);
    ListExpr err;
    bool correct;
    Word w = InMRegion(nl->Empty(), mreg, 0, err, correct);
//    result.setAddr(w.addr);
    
    MRegion mr = mf.ToMRegion(iv);
    *m = mr;
    
    return 0;
}

