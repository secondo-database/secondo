/*
*/

#include "interpolate.h"

static Reg GetConcavity(Reg *reg, Reg *hull, Reg *peer);

RotatingPlane::RotatingPlane(Reg *sreg, Reg *dreg, int depth, bool evap) {
    MSegs msegs;

    msegs.sreg = *sreg;
    msegs.dreg = *dreg;

    sreg->Begin();
    dreg->Begin();

    Reg shull = sreg->ConvexHull();
    Reg dhull = dreg->ConvexHull();
    
    cerr << "Rotating Plane depth " << depth << "\n";
    cerr << sreg->ToString() << "\n with\n" << dreg->ToString() << "\n";

    do {
        double a1 = shull.Cur().angle();
        double a2 = dhull.Cur().angle();
        Pt is, ie, fs, fe;

        if (((a1 <= a2) && !shull.End()) || dhull.End()) {
            is = shull.Cur().s;
            ie = shull.Cur().e;
            fs = fe = dhull.Cur().s;
            msegs.AddMSeg(MSeg(is, ie, fs, fe));
            if (!(shull.Cur() == sreg->Cur())) {
                // We found a concavity in the source region
                Reg concavity = GetConcavity(sreg, &shull, &dhull);
                scvs.push_back(concavity);
            } else {
                sreg->Next();
            }
            shull.Next();
        } else if ((a1 >= a2) || shull.End()) {
            is = ie = shull.Cur().s;
            fs = dhull.Cur().s;
            fe = dhull.Cur().e;
            msegs.AddMSeg(MSeg(is, ie, fs, fe));
            if (!(dhull.Cur() == dreg->Cur())) {
                // We found a concavity in the destination region
                Reg concavity = GetConcavity(dreg, &dhull, &shull);
                dcvs.push_back(concavity);
            } else {
                dreg->Next();
            }
            dhull.Next();
        }

        if (shull.End() && dhull.End())
            break;

    } while (1);

    cerr << "scvs " << scvs.size() << " ";
    cerr << "dcvs " << dcvs.size() << " end\n";
    
    for (unsigned int i = 0; i < sreg->holes.size(); i++) {
        scvs.push_back(sreg->holes[i]);
    }
    for (unsigned int i = 0; i < dreg->holes.size(); i++) {
        dcvs.push_back(dreg->holes[i]);
    }

    if (evap) {
        msegs = shull.GetMSegs(true);
        msegs.sreg = *sreg;
        msegs.dreg = *dreg;
    }

    face = MFace(msegs);
    
    cerr << "Rotating Plane depth " << depth << " end\n";

}

static Reg GetConcavity(Reg *reg, Reg *hull, Reg *peer) {
    Reg concavity;

    concavity.hullSeg = hull->Cur();
    while (!(hull->Cur().e == reg->Cur().s)) {
        concavity.AddSeg(reg->Cur());
        reg->Next();
    }
    concavity.hullPoint = reg->Cur().s;
    concavity.peerPoint = peer->Cur().s;
    concavity.parent = reg;
    concavity.Close();
    
    cerr << "Found concavity " << concavity.ToString() << "\n";

    return concavity;
}