/*
*/

#include "interpolate.h"

RotatingPlane::RotatingPlane(Reg *reg1, Reg *reg2) {
    MSegs msegs;
    
    reg1->Begin();
    reg2->Begin();

    Reg r1 = Reg(reg1->convexhull);
    Reg r2 = Reg(reg2->convexhull);

    do {
        cerr << "y1\n";
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
    
    for (unsigned int i = 0; i < reg1->holes.size(); i++) {
        scvs.push_back(reg1->holes[i]);
    }
    for (unsigned int i = 0; i < reg2->holes.size(); i++) {
        dcvs.push_back(reg2->holes[i]);
    }

    face = MFace(msegs);
}

