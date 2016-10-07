package viewer.hoese.algebras.fixedmregion;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

/**
 * Represents a Fixed Moving Region.
 * 
 * This class represents a Fixed Moving Region. This is a region of fixed
 * shape, which moves and rotates over one or more given time intervals.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class FMRegion {
    /** Shape of the region. */
    private Region region;
    /** One or more transformation units, which describe the movement. */
    private List<FMRegionTrans> transformations = new LinkedList();

    /** The moving segments calculated from region and transformations. */
    private List<FMSeg> fmsegs = new LinkedList();
    /** Start and end of the time interval. */
    private Double start, end;

    /**
     * Creates an empty FMRegion.
     */
    public FMRegion() {
    }

    /**
     * Constructs an FMRegion from a given region.
     * No transformation unit is initially added.
     * 
     * @param region Shape of the Fixed Moving Region
     */
    public FMRegion(Region region) {
        this.region = region;
    }

    /**
     * Get the shape of this Fixed Moving Region.
     * 
     * @return Shape of this Fixed Moving Region
     */
    public Region getRegion() {
        return region;
    }

    /**
     * Set the shape of this Fixed Moving Region.
     * 
     * @param region New shape of this Fixed Moving Region
     */
    public void setRegion(Region region) {
        this.region = region;
    }

    /**
     * Creates the Fixed Moving Segments for this FMRegion and the given
     * transformation.
     * 
     * @param transformation Transformation unit to create FMSegs for
     */
    private void createFMSegs(FMRegionTrans transformation) {
        if (region == null) {
            return;
        }
        fmsegs.addAll(transformation.getFMSegs(region));
        if (start == null || start > transformation.getStart()) {
            start = transformation.getStart();
        }
        if (end == null || end < transformation.getEnd()) {
            end = transformation.getEnd();
        }
    }

    /**
     * Clear all existing FMSegs and create new ones for the current data.
     * Used if the underlying region or transformation units changed.
     * 
     */
    public void renewFMSegs() {
        fmsegs.clear(); // Remove all segments
        start = end = null; // Clear the time interval boundaries
        for (FMRegionTrans t : transformations) {
            createFMSegs(t); // Create new fmsegs for all transformation units
        }
    }

    /**
     * Add a new transformation unit to this FMRegion.
     * 
     * @param transformation New transformation unit
     */
    public void addFMRegionTrans(FMRegionTrans transformation) {
        transformations.add(transformation);
        Collections.sort(transformations);
        createFMSegs(transformation);
    }

    /**
     * 
     * Add a new transformation unit from its parameters to this FMRegion.
     * 
     * Adds a new transformation unit. This function also calculates the
     * necessary initial translation and rotation, if this is not the first
     * transformation unit.
     * 
     * @param center Center point of rotation
     * @param v Translation vector
     * @param a Rotation angle
     * @param duration Duration of the time interval
     */
    public void addFMRegionTrans(Point center, Point v, double a, long duration) {
        Point v0 = new Point(0, 0);
        double a0 = 0;
        double start = 0;
        int nrtrans = transformations.size();
        if (nrtrans > 0) {
            FMRegionTrans t = transformations.get(nrtrans - 1);
            v0 = t.getV0().add(t.getV());
            a0 = t.getA0() + t.getA();
            // Compensate change of center
            Point dc = center.rotate(t.getCenter(), a0).sub(center);
            v0 = v0.add(dc);
            start = t.getEnd();
            t.setRightClosed(false);
        }
        FMRegionTrans t = new FMRegionTrans(center, v0, v, a0, a, start, start + duration);
        addFMRegionTrans(t);
    }

    /**
     * Returns the overall start time of this FMRegion.
     * This is the earliest instant of all transformation unit intervals.
     * 
     * @return start time
     */
    public double getStartTime() {
        return start != null ? start : 0;
    }

    /**
     * Returns the overall end time of this FMRegion.
     * This is the latest instant of all transformation unit intervals.
     * 
     * @return end time
     */
    public double getEndTime() {
        return end != null ? end : 0;
    }
    
    /**
     * Calculate and return the bounding box of thie FMRegion.
     * This is the bounding box, which is valid for all instants of this
     * FMRegion.
     * 
     * @return Bounding box of this FMRegion
     */
    public Seg getBoundingBox() {
        Seg bb = null;
        for (FMRegionTrans t : getTransformations()) {
            for (FMSeg fm : t.getFMSegs(region)) {
                Seg bb2 = fm.getBoundingBox();
                if (bb == null)
                    bb = bb2;
                else {
                    if (bb2.s.x < bb.s.x)
                        bb.s.x = bb2.s.x;
                    if (bb2.e.x > bb.e.x)
                        bb.e.x = bb2.e.x;
                    if (bb2.s.y < bb.s.y)
                        bb.s.y = bb2.s.y;
                    if (bb2.e.y > bb.e.y)
                        bb.e.y = bb2.e.y;
                }
            }
        }
        return bb;
    }

    /**
     * Create an FMRegion object from its Nested List representation
     * 
     * @param nl Nested List representation of FMRegion
     * @return FMRegion object
     */
    public static FMRegion deserialize(NL nl) {
        Region r = (Region) new Region().deserialize(nl.get(0));
        System.out.println(r.serialize().toString());
        FMRegion ret = new FMRegion(r);
        NL trafos = nl.get(1);
        for (int i = 0; i < trafos.size(); i++) {
            FMRegionTrans t = FMRegionTrans.deserialize(trafos.get(i));
            ret.addFMRegionTrans(t);
        }

        return ret;
    }

    /**
     * Create a nested list representation from this FMRegion.
     * 
     * @return Nested List representation of FMRegion
     */
    public NL serialize() {
        NL nl = new NL(), ret = nl;
        nl.addNL(region.serialize());
        nl = nl.nest();
        for (FMRegionTrans t : transformations) {
            nl.addNL(t.serialize());
        }

        return ret;
    }

    /**
     * Get the calculated Fixed Moving Segments.
     * 
     * @return Fixed Moving Segments
     */
    public List<FMSeg> getFMSegs() {
        return fmsegs;
    }
    
    /**
     * Returns the last transformation of this FMRegion.
     * 
     * @return Last transformation unit
     */
    public FMRegionTrans getLastTransformation() {
        int nrtrans = transformations.size();
        if (nrtrans == 0)
            return new FMRegionTrans();
        FMRegionTrans t = transformations.get(nrtrans-1);
        return t;
    }
    
    /**
     * Returns a list of all transformation units.
     * 
     * @return transformation units
     */
    public List<FMRegionTrans> getTransformations() {
        return transformations;
    }

    /**
     * Sets the transformation units.
     * 
     * @param transformations the transformation units to set
     */
    public void setTransformations(List<FMRegionTrans> transformations) {
        this.transformations = transformations;
    }
    
    /**
     * Create a projection of this FMRegion at instant currentTime.
     * The result is a Region object.
     * 
     * @param currentTime Instant to project
     * @return Resulting Region object
     */
    Region project (double currentTime) {
        Region ret = new Region();
        FMRegionTrans trans = null;
        for (FMRegionTrans t : transformations) {
            if (t.getStart() < currentTime && t.getEnd() > currentTime) {
                trans = t;
            }
        }
        if (trans == null)
            return null;
        for (Face f : region.getFaces()) {
            Face n = f.project(trans, currentTime);
            ret.addFace(n);
        }
        
        return ret;
    }
}
