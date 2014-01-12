/*
*/

#include "interpolate.h"

static bool hullIdentical(Reg *reg1, Reg *reg2) {
    if (reg1->v.size() != reg2->v.size())
        return false;

    for (unsigned int i = 0; i < reg1->v.size(); i++) {
        if (!(reg1->v[i] == reg2->v[i]))
            return false;
    }

    return true;
}

RotatingPlane::RotatingPlane(Reg *reg1, Reg *reg2, int depth) {

    MSegs msegs;
    msegs.sreg = *reg1;
    msegs.dreg = *reg2;


    reg1->ConvexHull();
    reg2->ConvexHull();

    reg1->Begin();
    reg2->Begin();

    Reg r1 = Reg(reg1->convexhull);
    Reg r2 = Reg(reg2->convexhull);

    if (!hullIdentical(reg1, reg2)) {
        do {
            double a1 = r1.Cur().angle();
            double a2 = r2.Cur().angle();
            Pt is, ie, fs, fe;

            if (((a1 <= a2) && !r1.End()) || r2.End()) {
                is = r1.Cur().s;
                ie = r1.Cur().e;
                fs = fe = r2.Cur().s;
                msegs.AddMSeg(MSeg(is, ie, fs, fe));
                if (!(r1.Cur() == reg1->Cur())) {
                    // We found a concavity in the source region
                    Reg ccv;

                    ccv.hullSeg = r1.Cur();
                    while (!(r1.Cur().e == reg1->Cur().s)) {
                        Seg s(reg1->Cur().s, reg1->Cur().e);
                        ccv.AddSeg(s);
                        reg1->Next();
                    }
                    ccv.hullPoint = reg1->Cur().s;
                    ccv.peerPoint = r2.Cur().s;
                    ccv.parent = reg1;
                    ccv.Close();
                    scvs.push_back(ccv);

                    r1.Next();
                } else {
                    r1.Next();
                    reg1->Next();
                }
            } else if ((a1 >= a2) || r1.End()) {
                is = ie = r1.Cur().s;
                fs = r2.Cur().s;
                fe = r2.Cur().e;
                msegs.AddMSeg(MSeg(is, ie, fs, fe));
                if (!(r2.Cur() == reg2->Cur())) {

                    // We found a concavity in the destination region
                    Reg ccv;

                    ccv.hullSeg = r2.Cur();
                    while (!(r2.Cur().e == reg2->Cur().s)) {
                        Seg s(reg2->Cur().s, reg2->Cur().e);
                        ccv.AddSeg(s);
                        reg2->Next();
                    }
                    ccv.hullPoint = reg2->Cur().s;
                    ccv.peerPoint = r1.Cur().s;
                    ccv.parent = reg2;
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
    } else {
        msegs = reg1->GetMSegs();
    }

    for (unsigned int i = 0; i < reg1->holes.size(); i++) {
        scvs.push_back(reg1->holes[i]);
    }
    for (unsigned int i = 0; i < reg2->holes.size(); i++) {
        dcvs.push_back(reg2->holes[i]);
    }

    face = MFace(msegs);

}
