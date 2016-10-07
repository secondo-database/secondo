package viewer.hoese.algebras.fixedmregion;

import java.awt.geom.Line2D;
import java.util.List;

/**
 * Class Seg represents a line segment.
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class Seg {
    /** Start and end point of line segment */
    public Point s, e;

    /**
     * Construct an empty line segment.
     */
    public Seg() {
    }

    /**
     * Copy constructor
     * 
     * @param s line segment to copy
     */
    public Seg(Seg s) {
        this.s = new Point(s.s);
        this.e = new Point(s.e);
    }

    /**
     * Constructor from start and end point
     * 
     * @param s start point
     * @param e end point
     */
    public Seg(Point s, Point e) {
        this.s = new Point(s);
        this.e = new Point(e);
    }

    /**
     * Constructor from start and end coordinates
     * 
     * @param sx x coordinate of start point
     * @param sy y coordinate of start point
     * @param ex x coordinate of end point
     * @param ey y coordinate of end point
     */
    public Seg(double sx, double sy, double ex, double ey) {
        this.s = new Point(sx, sy);
        this.e = new Point(ex, ey);
    }

    /**
     * Return a copy of this segment rotated by angle around center
     * 
     * @param center rotation center
     * @param angle rotation angle
     * @return rotated segment
     */
    public Seg rotate(Point center, double angle) {
        Point sn = s.rotate(center, angle);
        Point en = e.rotate(center, angle);
        return new Seg(sn, en);
    }

    /**
     * Return a copy of this segment translated by vector.
     * 
     * @param vector translation vector
     * @return translated segment
     */
    public Seg translate(Point vector) {
        Point sn = s.add(vector);
        Point en = e.add(vector);
        return new Seg(sn, en);
    }
    
    /**
     * Calculate the length of this segment
     * 
     * @return length of this segment
     */
    public double length() {
        return s.dist(e);
    }
    
    
   /**
    * Calculate the sign of this segment with a point p.
    * 
    * The sign is:
    * positive if p is left  of this segment
    * negative if p is right of this segment
    * zero     if the point is _on_ the (prolonged) segment.
    */ 
    public double sign(Point p) {
        return ((p.x-e.x)*(s.y-e.y)-(p.y-e.y)*(s.x-e.x));
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(" ( ").append(s.x).append("/").append(s.y).append(" ")
                .append(e.x).append("/").append(e.y).append(" ) ");

        return sb.toString();
    }
}
