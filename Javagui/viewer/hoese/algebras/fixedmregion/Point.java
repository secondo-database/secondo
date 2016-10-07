package viewer.hoese.algebras.fixedmregion;

/**
 * Class Point represents a 2D point.
 * Several operations are defined on a point, i.e. add, sub, rotate, length etc.
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class Point {
    /** x and y coordinate of this point */
    public double x, y;

    /**
     * Construct a point from its coordinates.
     * 
     * @param x x coordinate
     * @param y y coordinate
     */
    public Point(double x, double y) {
        this.x = x;
        this.y = y;
    }

    /**
     * Copy Constructor.
     * 
     * @param p point to copy
     */
    public Point(Point p) {
        this.x = p.x;
        this.y = p.y;
    }

    /**
     * Create a new point as the sum of this point and point p.
     * 
     * @param p point to add
     * @return sum of points
     */
    public Point add(Point p) {
        return new Point(this.x + p.x, this.y + p.y);
    }

    /**
     * Create a new point as the difference of this point and point p.
     * 
     * @param p point to subtract
     * @return difference of points
     */
    public Point sub(Point p) {
        return new Point(this.x - p.x, this.y - p.y);
    }

    /**
     * Create a new point by scaling this point with a factor m
     * 
     * @param m scale factor
     * @return scaled point
     */
    public Point mul(double m) {
        return new Point(this.x * m, this.y * m);
    }

    /**
     * Calculate the distance of two points.
     * 
     * @param p second point
     * @return distance between points
     */
    public double dist(Point p) {
        Point d = this.sub(p);
        return Math.sqrt(d.x * d.x + d.y * d.y);
    }
    
    /**
     * Calculate the distance of this point to the origin.
     * This can be seen as the length of a vector.
     * 
     * @return length of vector
     */
    public double length() {
        return Math.sqrt(x*x+y*y);
    }
    
    /**
     * Calculate the position of this point rotated by angle around center.
     * 
     * @param center rotation center
     * @param angle rotation angle
     * @return position of new point
     */
    public Point rotate (Point center, double angle) {
        Point p = this.sub(center);
        double s = Math.sin(angle);
        double c = Math.cos(angle);
        double xn = p.x * c - p.y * s;
        double yn = p.x * s + p.y * c;
        Point ret = new Point(xn, yn).add(center);
        
        return ret;
    }
    
    /**
     * Calculate the position of this point rotated by angle around the origin.
     * 
     * @param angle rotation angle
     * @return position of new point
     */
    public Point rotate (double angle) {
        return rotate(new Point(0,0), angle);
    }

    /**
     * Equality operator. Two points are equal if their coordinates are equal.
     * 
     * @param p point to compare
     * @return true, if the points are equal
     */
    public boolean equals(Point p) {
        return x == p.x && y == p.y;
    }
    
    public static final double TOLERANCE = 0.00001;
    
    /**
     * Nearly equal operation.
     * Two points are equal if their coordinates match within a certain
     * tolerance.
     * 
     * @param p point to compare
     * @return true, if the points are nearly equal
     */
    public boolean nequals(Point p) {
        if (p == null)
            return false;
        double xd = Math.abs(x-p.x);
        double yd = Math.abs(y-p.y);
        return ((xd < TOLERANCE) && (yd < TOLERANCE));
    }

    /**
     * Get the polar angle of this point.
     * 
     * @return polar angle
     */
    public double getAngleRad() {
        double ret = Math.atan2(y, x);

        return ret;
    }
    
    /**
     * Set x coordinate of this point
     * 
     * @param x new x coordinate
     * @return this point
     */
    public Point setX(double x) {
        this.x = x;
        return this;
    }
    
    /**
     * Set y coordinate of this point
     * 
     * @param y new y coordinate
     * @return this point
     */
    public Point setY(double y) {
        this.y = y;
        return this;
    }
    
    /**
     * Return the nested list representation of this point.
     * 
     * @return nested list representation
     */
    public NL toNL() {
        NL nl = new NL();
        nl.addNr(x);
        nl.addNr(y);
        
        return nl;
    }
    
    /**
     * Create a Point from a nested list representation
     * 
     * @param nl nested list representation
     * @return a point
     */
    public static Point fromNL(NL nl) {
        return new Point(nl.get(0).getNr(), nl.get(1).getNr());
    }
    
    /**
     * Return a string representation of this point.
     * 
     * @return string representation
     */
    @Override
    public String toString() {
        return x + "/" + y;
    }
}
