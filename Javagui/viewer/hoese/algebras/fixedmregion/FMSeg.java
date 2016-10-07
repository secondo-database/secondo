package viewer.hoese.algebras.fixedmregion;

/**
 * Fixed Moving Segments are the elements of a Fixed Moving Region.
 * 
 * They are defined by an initial segment (which is the fmseg at time t=0), a
 * translation vector, a rotation angle and center point and a start/end time.
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class FMSeg {

    /** start and end time of time interval */
    private double startTime, endTime;
    /** initial segment */
    private Seg i;
    /** rotation angle */
    private double rotate;
    /** rotation center point */
    private Point C;
    /** translation vector */
    private Point v;

    /**
     * Create an FMSeg from all parameters.
     * 
     * @param i initial segment
     * @param vector translation vector
     * @param center rotation center
     * @param rotation rotation angle
     * @param startTime start time of time interval
     * @param endTime end time of time interval
     */
    public FMSeg(Seg i, Point vector, Point center, double rotation, double startTime, double endTime) {
        this.i = new Seg(i);
        this.startTime = startTime;
        this.endTime = endTime;
        this.v = vector;
        this.C = center;
        this.rotate = rotation;
    }
    
    /**
     * Construct an FMSeg from a segment and a transformation unit.
     * Also honours initial translation/rotation of the transformation unit.
     * 
     * @param i initial segment
     * @param trans transformation unit
     */
    public FMSeg (Seg i, FMRegionTrans trans) {
        this.i = i.rotate(trans.getCenter(), trans.getA0()).translate(trans.getV0());
        this.v = trans.getV();
        this.C = trans.getCenter().add(trans.getV0());
        this.rotate = trans.getA();
        this.startTime = trans.getIv().getStart();
        this.endTime = trans.getIv().getEnd();
    }
    
    /**
     * Calculate a projection of this FMSeg at time currentTime.
     * 
     * @param currentTime The time of the projection
     * @return The calculated segment
     */
    public Seg project(long currentTime) {
        if (currentTime < startTime || currentTime > endTime
                || startTime >= endTime) {
            return null;
        }

        double frac = ((double) (currentTime - startTime)) / ((double) (endTime - startTime));

        return project(frac);
    }

    /**
     * Calculate a projection of this FMSeg at a specified fraction of the
     * time interval.
     * For example, t=0.5 is the middle of the time interval.
     * 
     * @param t The fraction of the time interval
     * @return The calculated segment
     */
    public Seg project(double t) {
        double angle = rotate * t;
        Point vec = v.mul(t);

        return i.rotate(C, angle).translate(vec);
    }

    /**
     * Get the bounding box for this FMSeg, which is valid at all instants.
     * 
     * @return bounding box
     */
    public Seg getBoundingBox() {
        double minx, maxx, miny, maxy;
        double sdist = i.s.dist(C);
        double edist = i.e.dist(C);

        Seg f = new Seg(i.s.add(v), i.e.add(v));

        maxx = i.s.x + sdist;
        minx = i.s.x - sdist;
        if (i.e.x - edist < minx) {
            minx = i.e.x - edist;
        }
        if (i.e.x + edist > maxx) {
            maxx = i.e.x + edist;
        }
        if (f.s.x - sdist < minx) {
            minx = f.s.x - sdist;
        }
        if (f.s.x + sdist > maxx) {
            maxx = f.s.x + sdist;
        }
        if (f.e.x - edist < minx) {
            minx = f.e.x - edist;
        }
        if (f.e.x + edist > maxx) {
            maxx = f.e.x + edist;
        }

        maxy = i.s.y + sdist;
        miny = i.s.y - sdist;
        if (i.e.y - edist < miny) {
            miny = i.e.y - edist;
        }
        if (i.e.y + edist > maxy) {
            maxy = i.e.y + edist;
        }
        if (f.s.y - sdist < miny) {
            miny = f.s.y - sdist;
        }
        if (f.s.y + sdist > maxy) {
            maxy = f.s.y + sdist;
        }
        if (f.e.y - edist < miny) {
            miny = f.e.y - edist;
        }
        if (f.e.y + edist > maxy) {
            maxy = f.e.y + edist;
        }

        return new Seg(new Point(minx, miny), new Point(maxx, maxy));
    }

    /**
     * Returns a string representation of this FMSeg.
     * 
     * @return string representation of FMSeg
     */
    public String toString() {
        return " ( " + getI() + " => " + v.toString() + "@" + rotate * 360 / (2 * Math.PI) + "° ) ";
    }

    /**
     * Get start time of the time interval.
     * 
     * @return start time of the time interval
     */
    public double getStartTime() {
        return startTime;
    }

    /**
     * Set start time of the time interval.
     * 
     * @param startTime start time of the time interval
     */
    public void setStartTime(long startTime) {
        this.startTime = startTime;
    }

    /**
     * Get end time of the time interval.
     * 
     * @return end time of the time interval
     */
    public double getEndTime() {
        return endTime;
    }

    /**
     * Set end time of the time interval.
     * 
     * @param endTime end time of the time interval
     */
    public void setEndTime(long endTime) {
        this.endTime = endTime;
    }

    /**
     * Get rotation angle of this FMSeg
     * 
     * @return rotation angle
     */
    public double getRotation() {
        return getRotate();
    }

    /**
     * Set rotation angle of this FMSeg
     * 
     * @param rotation rotation angle
     */
    public void setRotation(double rotation) {
        this.setRotate(rotation);
    }

    /**
     * Get the projected center point at time currentTime.
     * 
     * @param currentTime time instant to project the center
     * @return projected center point
     */
    Point getProjectedCenter(double currentTime) {
        double frac = (currentTime - getStartTime()) / (getEndTime() - getStartTime());
        return getC().add(getV().mul(frac));
    }

    /**
     * Get initial segment.
     * 
     * @return the initial segment
     */
    public Seg getI() {
        return i;
    }

    /**
     * Set initial segment.
     * 
     * @param i the initial segment
     */
    public void setI(Seg i) {
        this.i = i;
    }

    /**
     * Get the rotation angle
     * 
     * @return the rotation angle
     */
    public double getRotate() {
        return rotate;
    }

    /**
     * Set the rotation angle
     * 
     * @param rotate the rotation angle
     */
    public void setRotate(double rotate) {
        this.rotate = rotate;
    }

    /**
     * Get the center point of rotation
     * 
     * @return center point
     */
    public Point getC() {
        return C;
    }

    /**
     * Set the center point of rotation
     * 
     * @param C center point
     */
    public void setC(Point C) {
        this.C = C;
    }

    /**
     * Get the translation vector
     * 
     * @return translation vector
     */
    public Point getV() {
        return v;
    }

    /**
     * Set the translation vector
     * 
     * @param v translation vector
     */
    public void setV(Point v) {
        this.v = v;
    }
}
