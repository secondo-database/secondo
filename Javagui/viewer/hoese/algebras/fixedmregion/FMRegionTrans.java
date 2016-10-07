package viewer.hoese.algebras.fixedmregion;

import java.util.LinkedList;
import java.util.List;

/**
 * FMRegionTrans represents a Transformation Unit of an FMRegion.
 * 
 * A Transformation Unit consists of:
 * 
 * center: A rotation center point
 * v0: initial translation
 * a0: initial rotation
 * v : translation during time interval
 * a : rotation during time interval
 * iv: the time interval
 * 
 * A Point p is thus projected at instant curtime:
 * 
 * (p.rotate(center, a0) + v0).rotate(center+v0, a*t) + v*t
 * with t = (curtime-iv.start)/(iv.end-iv.start)
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class FMRegionTrans implements Comparable<FMRegionTrans> {
    /** Center point of rotation */
    private Point center;
    /** Initial translation */
    private Point v0;
    /** Translation during time interval */
    private Point v;
    /** Initial rotation */
    private double a0;
    /** Rotation during time interval */
    private double a;
    /** The time interval */
    private Interval iv = new Interval();

    /**
     * Construct a static transformation (no movement)
     */
    public FMRegionTrans() {
        center = new Point(0, 0);
        v0 = new Point(0, 0);
        v = new Point(0, 0);
    }

    /**
     * Construct a transformation unit from all parameters.
     * 
     * @param center Center point of rotation
     * @param v0 Initial translation
     * @param v  Translation during time interval
     * @param a0 Initial rotation
     * @param a  Rotation during time interval
     * @param start Start instant
     * @param end   End   instant
     */
    public FMRegionTrans(Point center, Point v0, Point v, double a0, double a, double start, double end) {
        this.center = center;
        this.v0 = v0;
        this.v = v;
        this.a0 = a0;
        this.a = a;
        iv.setStart(start);
        iv.setEnd(end);
        iv.setLeftClosed(true);
        iv.setRightClosed(true);
    }

    /**
     * Construct a transformation unit from parameters without initial
     * translation/rotation
     * 
     * @param center Center point of rotation
     * @param v  Translation during time interval
     * @param a  Rotation during time interval
     * @param start Start instant
     * @param end   End   instant
     */
    public FMRegionTrans(Point center, Point v, double a, double start, double end) {
        this.center = center;
        this.v = v;
        this.v0 = new Point(0, 0);
        this.a = a;
        iv.setStart(start);
        iv.setEnd(end);
        iv.setLeftClosed(true);
        iv.setRightClosed(true);
    }
    
    /**
     * Return a transformation from this transformation restricted to
     * the interval startTime - endTime
     * 
     * @param startTime Start time of new interval
     * @param endTime End time of new interval
     * @return The new transformation
     */
    public FMRegionTrans restrict (double startTime, double endTime) {
        Point nv0 = v0.add(v.mul(iv.getFrac(startTime)));
        Point nv  = v.mul(iv.getFrac(endTime) - iv.getFrac(startTime));
        double na0 = a0 + a * iv.getFrac(startTime);
        double na = a * (iv.getFrac(endTime) - iv.getFrac(startTime));
        FMRegionTrans frt = new FMRegionTrans(center, nv0, nv, na0, na, startTime, endTime);
        return frt;
    }

    /**
     * Calculate the Fixed Moving Segments for this transformation unit and the
     * given region.
     * 
     * @param region Region to calculate the segments
     * @return List of Fixed Movign Segments
     */
    public List<FMSeg> getFMSegs(Region region) {
        List<FMSeg> l = new LinkedList();

        for (Face f : region.getFaces()) {
            for (Seg s : f.getSegments()) {
                s = s.rotate(center, a0).translate(v0);
                FMSeg fmseg = new FMSeg(s, v, center.add(v0), a, iv.getStart(), iv.getEnd());
                l.add(fmseg);
            }
        }

        return l;
    }

    /**
     * Get the center point of rotation.
     * 
     * @return Center point of rotation
     */
    public Point getCenter() {
        return center;
    }

    /**
     * Set the center point of rotation.
     * 
     * @param center Center point of rotation
     */
    public void setCenter(Point center) {
        this.center = center;
    }

    /**
     * Get the initial translation.
     * 
     * @return initial translation
     */
    public Point getV0() {
        return v0;
    }

    /**
     * Set the initial translation.
     * 
     * @param v0 initial translation
     */
    public void setV0(Point v0) {
        this.v0 = v0;
    }

    /**
     * Get the translation vector.
     * 
     * @return translation vector
     */
    public Point getV() {
        return v;
    }

    /**
     * Set the translation vector.
     * 
     * @param v translation vector
     */
    public void setV(Point v) {
        this.v = v;
    }

    /**
     * Get the initial rotation.
     * 
     * @return initial rotation
     */
    public double getA0() {
        return a0;
    }

    /**
     * Set the initial rotation.
     * 
     * @param a0 initial rotation
     */
    public void setA0(double a0) {
        this.a0 = a0;
    }

    /**
     * Get the rotation during the time interval
     * 
     * @return rotation angle
     */
    public double getA() {
        return a;
    }

    /**
     * Set the rotation during the time interval
     * 
     * @param a rotation angle
     */
    public void setA(double a) {
        this.a = a;
    }

    /**
     * Get the start of the time interval
     * 
     * @return start of time interval
     */
    public Double getStart() {
        return iv.getStart();
    }

    /**
     * Set the start of the time interval
     * 
     * @param start start of time interval
     */
    public void setStart(double start) {
        iv.setStart(start);
    }

    /**
     * Get the end of the time interval
     * 
     * @return end of time interval
     */
    public Double getEnd() {
        return iv.getEnd();
    }

    /**
     * Set the end of the time interval
     * 
     * @param end end of time interval
     */
    public void setEnd(double end) {
        iv.setEnd(end);
    }

    /**
     * Create a Nested List representation of this transformation unit.
     * 
     * @return Nested List representation
     */
    public NL serialize() {
        NL nl = new NL();
        nl.addNL(center.toNL());
        nl.addNL(v0.toNL());
        nl.addNL(v.toNL());
        nl.addNr(a0);
        nl.addNr(a);
        nl.addNL(iv.toNL());

        return nl;
    }

    /**
     * Create a Transformation Unit from its Nested List representation.
     * 
     * @param nl Nested List representation
     * @return FMRegionTrans object
     */
    public static FMRegionTrans deserialize(NL nl) {
        FMRegionTrans ret = new FMRegionTrans();

        ret.setCenter(Point.fromNL(nl.get(0)));
        ret.setV0(Point.fromNL(nl.get(1)));
        ret.setV(Point.fromNL(nl.get(2)));
        ret.setA0(nl.get(3).getNr());
        ret.setA(nl.get(4).getNr());
        ret.setIv(Interval.fromNL(nl.get(5)));

        return ret;
    }

    /**
     * Test if interval is left closed.
     * 
     * @return true, if interval is left closed
     */
    public boolean isLeftClosed() {
        return iv.isLeftClosed();
    }

    /**
     * Set if interval is left closed.
     * 
     * @param leftClosed true, if interval is left closed.
     */
    public void setLeftClosed(boolean leftClosed) {
        iv.setLeftClosed(leftClosed);
    }

    /**
     * Test if interval is right closed.
     * 
     * @return true, if interval is right closed
     */
    public boolean isRightClosed() {
        return iv.isRightClosed();
    }

    /**
     * Set if interval is right closed.
     * 
     * @param rightClosed true, if interval is right closed.
     */
    public void setRightClosed(boolean rightClosed) {
        iv.setRightClosed(rightClosed);
    }

    /**
     * Get the time interval of this transformation unit.
     * 
     * @return time interval
     */
    public Interval getIv() {
        return iv;
    }

    /**
     * Set the time interval of this transformation unit.
     * 
     * @param iv the interval to set
     */
    public void setIv(Interval iv) {
        this.iv = iv;
    }

    /**
     * Compare this transformation unit to another one.
     * A transformation unit is considered smaller, if it starts at a smaller
     * instant. Used to sort chronologically.
     * 
     * @param transformation the transformation unit to compare with
     * @return <0 for smaller, =0 for equal, >0 for bigger
     */
    @Override
    public int compareTo(FMRegionTrans transformation) {
        if (iv.getStart() < transformation.iv.getStart())
            return -1;
        else if (iv.getStart() == transformation.iv.getStart()) {
            if (!iv.isLeftClosed() && transformation.iv.isLeftClosed())
                return -1;
            else if (iv.isLeftClosed() && !transformation.iv.isLeftClosed())
                return 1;
            else
                return 0;
        } else { /* iv.getStart() > t.iv.getStart() */
            return 1;
        }
    }
}
