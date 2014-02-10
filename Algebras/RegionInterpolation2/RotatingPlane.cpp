/*
  1 Class RotatingPlane
  
  Attributes:
  ~MFace~ face
  vector<Face> scvs, dcvs
  
  This class implements a variation of the Rotating-Plane-Algorithm.

*/

#include "interpolate.h"

static Face getConcavity(Face *reg, Face *hull, Face *peer);

/*
  1.1 Constructor RotatingPlane
  
  The class is constructed with two faces sreg and dreg, calculates
  MovingSegments using the Rotating-Plane-Algorithm and stores them into the
  attribute ~face~.
  Furthermore it detects all concavities in the source- and destination-regions
  and puts them in lists (~scvs~ for source- and ~dcvs~ for destination-
  concavities) together with the holes of these faces (holes and concavities
  are treated equal most of the time)
  
  The flag ~evap~ is set, if we currently are in the evaporation-phase, in
  which case the MovingSegments are constructed a little different.
  
  
*/
RotatingPlane::RotatingPlane(Face *sreg, Face *dreg, int depth, bool evap) {
    MSegs msegs;

    /* Store a copy of the original source- and destination-regions for further
     * reference (e.g. collapse or expand in case of intersections) */
    msegs.sreg = *sreg;
    msegs.dreg = *dreg;

    // Reset the segment position to the begin of the faces cycles
    sreg->Begin();
    dreg->Begin();

    // Calculate the convex hulls of the faces
    Face shull = sreg->ConvexHull();
    Face dhull = dreg->ConvexHull();
    
    do {
        // Calculate the angles of the hull-segments relative to the x-axis
        double asrc = shull.Cur().angle(), adst = dhull.Cur().angle();
        
        if ( ((asrc <= adst) || dhull.End()) && !shull.End() ) {
            Pt is = shull.Cur().s, ie = shull.Cur().e; // Initial segment
            Pt fs = dhull.Cur().s, fe = dhull.Cur().s; // Final segment
            msegs.AddMSeg(MSeg(is, ie, fs, fe));
            if (!(shull.Cur() == sreg->Cur())) {
                // We found a concavity in the source region
                Face concavity = getConcavity(sreg, &shull, &dhull);
                scvs.push_back(concavity);
            } else {
                sreg->Next();
            }
            shull.Next();
        }
        
        if ( ((asrc >= adst) || shull.End()) && !dhull.End() ) {
            Pt is = shull.Cur().s, ie = shull.Cur().s; // Initial segment
            Pt fs = dhull.Cur().s, fe = dhull.Cur().e; // Final segment
            msegs.AddMSeg(MSeg(is, ie, fs, fe));
            if (!(dhull.Cur() == dreg->Cur())) {
                // We found a concavity in the destination region
                Face concavity = getConcavity(dreg, &dhull, &shull);
                dcvs.push_back(concavity);
            } else {
                dreg->Next();
            }
            dhull.Next();
        }

    } while (!shull.End() || !dhull.End());

    
    /* Add the holes of the faces to the list of concavities */
    for (unsigned int i = 0; i < sreg->holes.size(); i++) {
        scvs.push_back(sreg->holes[i]);
    }
    for (unsigned int i = 0; i < dreg->holes.size(); i++) {
        dcvs.push_back(dreg->holes[i]);
    }

    /* If we currently are in the evaporation-phase, we discard the calculated
     * MovingSegments and recreate them directly from the hull. The source-hull
     * and destination-hull should be identical in this phase, since the only
     * differences should be missing concavities.
     */
    if (evap) {
        msegs = shull.GetMSegs(true);
        msegs.sreg = *sreg;
        msegs.dreg = *dreg;
    }

    mface = MFace(msegs);
}

/*
  1.2 Retrieves the next concavity from ~face~.
  The current segment from ~reg~ differs from the current ~hull~-segment, so
  the concavity starts here and ends with the endpoint of the ~hull~-segment.
  ~peer~ is the current state of the other face of the RotatingPlane and is
  used to store the peerpoint of the concavity (the collapse-point)

*/
static Face getConcavity(Face *face, Face *hull, Face *peer) {
    Face concavity;

    // hullSeg is the segment of the hull which shortcuts the concavity.
    concavity.hullSeg = hull->Cur();
    
    // the concavity ends with the endpoint of the hull-segment
    while (!(hull->Cur().e == face->Cur().s)) {
        concavity.AddSeg(face->Cur());
        face->Next();
    }
    concavity.peerPoint = peer->Cur().s;
    concavity.parent = face;
    concavity.Close();
    
    return concavity;
}