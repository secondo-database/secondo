package viewer.hoese.algebras.pmregion;

import java.util.Random;

/**
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class Point implements Comparable {

    static Random r;

    static {
        r = new Random();
        r.setSeed(System.currentTimeMillis());
    }
    public double x, y;

    public Point(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public Point(Point p) {
        this.x = p.x;
        this.y = p.y;
    }
    
    @Override
    public int compareTo (Object T) {
        if (!(T instanceof Point))
            return -1;
        Point p = (Point)T;
        if ((x < p.x) || (x == p.x && y < p.y)) {
            return -1;
        } else if (x == p.x && y == p.y) {
            return 0;
        } else {
            return 1;
        }
    }

    public Point add(Point p) {
        return new Point(this.x + p.x, this.y + p.y);
    }

    public Point sub(Point p) {
        return new Point(this.x - p.x, this.y - p.y);
    }

    public Point mul(double m) {
        return new Point(this.x * m, this.y * m);
    }
    
    public Point div(double m) {
        return new Point(this.x / m, this.y / m);
    }
    
    public Point mul(Point p) {
        return new Point(this.x * p.x, this.y * p.y);
    }

    public double dist(Point p) {
        Point d = this.sub(p);
        return Math.sqrt(d.x * d.x + d.y * d.y);
    }
    
    public double length() {
        return Math.sqrt(x*x+y*y);
    }
    
    public Point rotate (Point center, double angle) {
        Point p = this.sub(center);
        double s = Math.sin(angle);
        double c = Math.cos(angle);
        double xn = p.x * c - p.y * s;
        double yn = p.x * s + p.y * c;
        Point ret = new Point(xn, yn).add(center);
        
        return ret;
    }
    
    public Point rotate (double angle) {
        return rotate(new Point(0,0), angle);
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof Point) {
            Point p = (Point)o;
            return x == p.x && y == p.y;
        }
        
        return false;
    }

    @Override
    public int hashCode() {
        int hash = 5;
        hash = 37 * hash + (int) (Double.doubleToLongBits(this.x) ^ (Double.doubleToLongBits(this.x) >>> 32));
        hash = 37 * hash + (int) (Double.doubleToLongBits(this.y) ^ (Double.doubleToLongBits(this.y) >>> 32));
        return hash;
    }
    
    public static final double TOLERANCE = 0.000000001;
    
    public boolean nequals(Point p) {
        if (p == null)
            return false;
        double xd = Math.abs(x-p.x);
        double yd = Math.abs(y-p.y);
        return ((xd < TOLERANCE) && (yd < TOLERANCE));
    }

    public static Point random(double max) {
        do {
            Point ret = new Point(r.nextInt() % max - max / 2, r.nextInt() % max - max / 2);
            if (ret.x == 0 && ret.y == 0) {
                continue;
            }
            return ret;
        } while (true);
    }
    
    public double getAngleRad() {
        double ret = Math.atan2(y, x);

        return ret;
    }
    
    public Point setX(double x) {
        this.x = x;
        return this;
    }
    
    public Point setY(double y) {
        this.y = y;
        return this;
    }
    
    public NL toNL() {
        NL nl = new NL();
        nl.addNr(x);
        nl.addNr(y);
        
        return nl;
    }
    
    public static Point fromNL(NL nl) {
        return new Point(nl.get(0).getNr(), nl.get(1).getNr());
    }
    
    public String toString() {
        return x + "/" + y;
    }
}
