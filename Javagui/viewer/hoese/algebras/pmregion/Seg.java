package viewer.hoese.algebras.pmregion;

import java.awt.geom.Line2D;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.TreeSet;

/**
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class Seg {

    private boolean shrinkable = false;
    private Set<Point> intersectionPoints = new TreeSet(new Comparator<Point>() {
        @Override
        public int compare(Point p1, Point p2) {
            if (p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y)) {
                return -1;
            }
            if (p1.x == p2.x && p1.y == p2.y) {
                return 0;
            }
            return 1;
        }
    });
    public Point s, e;

    public Seg() {
    }

    public Seg(Seg s) {
        this.s = new Point(s.s);
        this.e = new Point(s.e);
        this.shrinkable = s.shrinkable;
    }

    public Seg(Point s, Point e) {
        this.s = new Point(s);
        this.e = new Point(e);
//        this.s = s;
//        this.e = e;
    }

    public Seg(Point s, Point e, boolean shrinkable) {
        this.s = new Point(s);
        this.e = new Point(e);
        this.shrinkable = shrinkable;
    }

    public Seg(double sx, double sy, double ex, double ey) {
        this.s = new Point(sx, sy);
        this.e = new Point(ex, ey);
    }

    public Seg rotate(Point center, double angle) {
        Point sn = s.rotate(center, angle);
        Point en = e.rotate(center, angle);
        return new Seg(sn, en);
    }

    public Seg translate(Point vector) {
        s = s.add(vector);
        e = e.add(vector);

        return this;
    }

    public Seg transform(Seg.TransformParam tp, boolean change) {
        Point sn = s.rotate(tp.center, tp.angle).add(tp.vector);
        System.out.println("Center: " + tp.center.toString() + " Vector: " + tp.vector.toString() + " Angle: " + tp.angle);
        System.out.println("Point before: " + s.toString() + "  after: " + sn.toString());
        Point en = e.rotate(tp.center, tp.angle).add(tp.vector);
        if (change) {
            s = sn;
            e = en;
            return this;
        } else {
            return new Seg(sn, en);
        }
    }

    public Seg transform(Seg.TransformParam tp) {
        return transform(tp, false);
    }

    public boolean similar(Seg seg, double tolerance) {
        double limit = seg.length() * tolerance;
        double diff1 = seg.s.sub(s).length();
        double diff2 = seg.e.sub(e).length();
        System.out.println(this.toString() + " ~= " + seg.toString());
        System.out.println("D1: " + diff1 + "  D2: " + diff2 + "   Limit: " + limit);
        return (diff1 < limit && diff2 < limit);
    }

    public double length() {
        return s.dist(e);
    }

    public Point getVector() {
        return e.sub(s);
    }

    /* Sign is >0 if p is left of this segment, otherwise <0. If it equals zero,
     the point is _on_ the line which prolonges this segment.
     */
    public double sign(Point p) {
        return p.x*s.y - p.x*e.y - e.x*s.y - p.y*s.x + p.y*e.x + e.y*s.x;
    }

    public boolean intersects(Seg seg) {
        if (seg.s.equals(e) || seg.e.equals(s)) {
            return false;
        }
        return Line2D.linesIntersect(seg.s.x, seg.s.y, seg.e.x, seg.e.y,
                s.x, s.y, e.x, e.y);
    }

    public static Point calculateIntersectionPoint(Point s1, Point d1, Point s2, Point d2) {
//        double sNumerator = s1.y * d1.x + s2.x * d1.y - s1.x * d1.y - s2.y * d1.x;
//        double sDenominator = d2.y * d1.x - d2.x * d1.y;
//
//        if (sDenominator == 0) {
//            return null;
//        }
//
//        double s = sNumerator / sDenominator;
//
//        double t;
//        if (d1.x != 0) {
//            t = (s2.x + s * d2.x - s1.x) / d1.x;
//        } else {
//            t = (s2.y + s * d2.y - s1.y) / d1.y;
//        }
//
//        Point i1 = new Point(s1.x + t * d1.x, s1.y + t * d1.y);
//        
//        return i1;

        double x1 = s1.x;
        double x2 = d1.x;
        double x3 = s2.x;
        double x4 = d2.x;
        double y1 = s1.y;
        double y2 = d1.y;
        double y3 = s2.y;
        double y4 = d2.y;
        double denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
        if (denom == 0.0) { // Lines are parallel.
            return null;
        }
        double ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
        double ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;
        if (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f) {
            // Get the intersection point.
            return new Point((x1 + ua * (x2 - x1)), (y1 + ua * (y2 - y1)));
        }

        return null;

    }

    public Point intersection(Seg seg) {
        return calculateIntersectionPoint(s, e, seg.s, seg.e);
    }

    public void addIntersectionPoint(Point p) {
        if (p == null) {
            return;
        }
        intersectionPoints.add(p);
    }

    public void clearIntersectionPoints() {
        intersectionPoints.clear();
        intersectionPoints.add(s);
        intersectionPoints.add(e);
    }

    public List<Seg> splitSegmentAtIntersectionPoints() {
        List<Seg> ret = new LinkedList();
        Point prev = null;
        for (Point p : intersectionPoints) {
            if (prev != null) {
                ret.add(new Seg(prev, p));
            }
            prev = p;
        }

        return ret;
    }

    public boolean intersects(List<Seg> segs) {
        for (Seg seg : segs) {
            if (this.intersects(seg)) {
                return true;
            }
        }

        return false;
    }

    public void changeDirection() {
        Point tmp = s;
        s = e;
        e = tmp;
    }

    public Seg reverse() {
        Seg ret = new Seg(this);
        ret.changeDirection();

        return ret;
    }

    public double getAngle() {
        double ret = Math.toDegrees(Math.atan2(e.y - s.y, e.x - s.x)) + 450;
        while (ret > 360) {
            ret -= 360;
        }

        return 360 - ret;
    }

    public double getAngle(Seg s) {
        double a = getAngle();
        double b = s.getAngle();
        double ret = b - a;
        if (ret < 0) {
            ret += 360;
        }

        return ret;
    }

    public double getAngleXAxis() {
        double ret = Math.toDegrees(Math.atan2(e.y - s.y, e.x - s.x));
        if (ret < 0) {
            ret += 360.0;
        }

        return ret;
    }

    public double getAngleRad() {
        double ret = Math.atan2(e.y - s.y, e.x - s.x);

        return ret;
    }

    public TransformParam getTransformParam(Seg seg, Point desiredcenter, double tolerance) {
        double mylength = length();
        double slength = seg.length();
        double diff = Math.abs(mylength - slength);
        if (diff > mylength * tolerance) {
            return null;
        }

        double angle = (getAngle() - seg.getAngle()) * Math.PI / 180.0;
        if (angle <= -Math.PI) {
            angle += 2 * Math.PI;
        } else if (angle > Math.PI) {
            angle -= 2 * Math.PI;
        }

        Point rotatedPoint = s.rotate(desiredcenter, angle);
        Point vector = seg.s.sub(rotatedPoint);

        return new TransformParam(desiredcenter, vector, angle);
    }

    public String nl() {
        return "   ( " + s.x + " " + s.y + " )\n";
    }

    public String nlmf() {
        return "       ( " + s.x + " " + s.y + " " + s.x + " " + s.y + " )\n";
    }

    public double[] getMC() {

        if (e.x == s.x) {
            return new double[]{Double.NaN, Double.NaN};
        }

        double m = (e.y - s.y) / (e.x - s.x);
        double c = s.y - m * s.x;

        return new double[]{m, c};
    }

    public boolean pointInsideBBox(Point p) {

        return (((s.x < e.x) && (p.x >= s.x) && (p.x <= e.x))
                || ((s.x >= e.x) && (p.x <= s.x) && (p.x >= e.x)))
                && (((s.y < e.y) && (p.y >= s.y) && (p.y <= e.y))
                || ((s.y >= e.y) && (p.y <= s.y) && (p.y >= e.y)));

    }

    public Point nearestPoint(Point c) {
        double x, y;

        if (s.x == e.x) {
            x = s.x;
            y = c.y;
        } else {
            double t = -((c.x - s.x) * (e.x - s.x) + (c.y - s.y) * (e.y - s.y))
                    / ((e.x - s.x) * (e.x - s.x) + (e.y - s.y) * (e.y - s.y));
            x = s.x - t * (e.x - s.x);
            y = s.y - t * (e.y - s.y);
        }

        if (((x < s.x) && (s.x <= e.x))
                || ((x > s.x) && (s.x >= e.x))
                || ((y < s.y) && (s.y <= e.y))
                || ((y > s.y) && (s.y >= e.y))) {
            return s;
        }
        if (((x < e.x) && (e.x <= s.x))
                || ((x > e.x) && (e.x >= s.x))
                || ((y < e.y) && (e.y <= s.y))
                || ((y > e.y) && (e.y >= s.y))) {
            return e;
        }

        return new Point(x, y);
    }

    public boolean isDegenerated() {
        return s.x == e.x && s.y == e.y;
    }

    public Seg copy(Point off, Point scale) {
        Seg ret = new Seg();
        ret.s = s.add(off).mul(scale);
        ret.e = e.add(off).mul(scale);
        ret.shrinkable = shrinkable;
//        ret.s = s.mul(scale).add(off);
//        ret.e = e.mul(scale).add(off);

        return ret;
    }
    
    public Seg orient() {
        if (s.x < e.x || (s.x == e.x && s.y < e.y))
            return new Seg(s, e);
        else
            return new Seg(e, s);
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof Seg) {
            Seg seg = (Seg) o;
            return seg.s.equals(s) && seg.e.equals(e);
        }
        return false;
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 83 * hash + Objects.hashCode(this.s);
        hash = 83 * hash + Objects.hashCode(this.e);
        return hash;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(" ( ").append(s.x).append("/").append(s.y).append(" ")
                .append(e.x).append("/").append(e.y).append(" )@").append(getAngleRad()).append(" ");

        return sb.toString();
    }

    /**
     * @return the shrinkable
     */
    public boolean isShrinkable() {
        return shrinkable;
    }
    
    public boolean congruent(Seg seg) {
        return (seg.s.equals(s) && seg.e.equals(e)) ||
               (seg.s.equals(e) && seg.e.equals(s));
    }

    /**
     * @param shrinkable the shrinkable to set
     */
    public void setShrinkable(boolean shrinkable) {
        this.shrinkable = shrinkable;
    }

    public static class TransformParam {

        public Point center;
        public Point vector;
        public double angle;

        public TransformParam(Point center, Point vector, double angle) {
            this.center = center;
            this.vector = vector;
            this.angle = angle;
        }

        public boolean similar(Seg.TransformParam tp, double tolerance) {
            if (!(this.center.equals(tp.center))) {
                return false;
            }
            if (this.vector.sub(tp.vector).length() > this.vector.length() * tolerance) {
                return false;
            }
            if (Math.abs(this.angle - angle) > this.angle * tolerance) {
                return false;
            }

            return true;
        }
    }
}
