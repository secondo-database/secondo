package viewer.hoese.algebras.fixedmregion;

import java.util.LinkedList;
import java.util.List;

/**
 * Class RCurve represents a curved line segment of a CFace/CRegion
 * 
 * Currently three types of line segment are defined:
 *
 * - Trochoid      (type 'T')
 * - Ravdoid       (type 'R')
 * - Straight line (type 'S')
 * 
 * The time interval of the parametric functions is always normalized to [0;1]
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
enum SegmentType {
   Trochoid, Ravdoid, Straight, Unknown;

   @Override
   public String toString(){
      switch (this){
        case Trochoid: return "T";
        case Ravdoid : return "R";
        case Straight: return "S";
        default: return null;
      }
   }

   static SegmentType  fromString(String s){
      if(s.equals("T")) return Trochoid; 
      if(s.equals("R")) return Ravdoid; 
      if(s.equals("S")) return Straight; 
      return Unknown;
   }

};



public class RCurve {
    /** Start point of this RCurve */
    private Point start;
    /** Rotation angle (center is always the start point) */
    private double angle;
    /** Type of curve */
    private SegmentType type;
    /** curve-specific parameters */
    private double[] params;

    /**
     * Construct an rcurve from all parameters.
     * 
     * @param start  startpoint
     * @param angle  rotation angle
     * @param type   type of curve (T,R,S)
     * @param params curve specific parameters 
     */
    public RCurve(Point start, double angle, SegmentType type, double[] params) {
        this.start = start;
        this.type = type;
        this.params = params;
        this.angle = angle;
    }
    
    /**
     * Construct an rcurve from its parameters, but calculate the initial
     * rotation for a given center.
     * 
     * @param start  startpoint
     * @param type   type of curve (T,R,S)
     * @param params curve specific parameters 
     * @param center rotation center
     * @param angle  rotation angle
     */
    public RCurve(Point start, SegmentType type, double[] params, Point center, double angle) {
        this.start = start;
        this.type = type;
        this.params = params;
        rotate (angle, center);
    }
    
    /**
     * Set the rotation parameters for this RCurve from an angle and a given
     * center.
     * After rotation the start point, the new center will be the start point.
     * 
     * @param angle  the rotation angle
     * @param center the center point
     */
    public final void rotate (double angle, Point center) {
        this.angle = angle;
        start = start.rotate(center, angle);
    }
    
    /**
     * Calculate the point on the curve at time t for type "Straight" (S)
     * 
     * @param t the time
     * @return point on curve
     */
    public Point straight(double t) {
        double xd = params[0];
        double yd = params.length == 2 ? params[1] : 0;
        
        Point off = new Point(xd, yd).mul(t);
        
        return start.add(off);
    }

    /**
     * Calculate the point on the curve at time t for type "Trochoid" (T)
     * 
     * @param t the time
     * @return point on curve
     */
    public Point trochoid(double t) {
        double a = params[0];
        double b = params[1];
        double toff = params[2];
        double rot = params[3];

        double x = a * t * rot - b * (Math.sin(t * rot + toff) - Math.sin(toff));
        double y = b * (Math.cos(t * rot + toff) - Math.cos(toff));
        
        Point off = new Point(x, y);

        return start.add(off);
    }

    /**
     * Calculate the point on the curve at time t for type "Ravdoid" (R)
     * 
     * @param t the time
     * @return point on curve
     */
    public Point ravdoid(double t) {
        double hp = params[0];
        double cd = params[1];
        double toff = params[2];
        double rot = params[3];

        double x = hp * (2 * t * rot - Math.sin(2 * (t * rot + toff)) + Math.sin(2 * toff))
                + cd * (Math.cos(t * rot + toff) - Math.cos(toff));

        double y = hp * (Math.cos(2 * (t * rot + toff)) - Math.cos(2 * toff))
                + cd * (Math.sin(t * rot + toff) - Math.sin(toff));
        
        Point off = new Point(x, y);

        return start.add(off);
    }

    /**
     * Calculate the point on this curve for time t.
     * 
     * @param t the time
     * @return Point on curve
     */
    public Point f(double t) {
        Point ret = null; 
        switch (type) {
            case Straight:
                ret = straight(t);
                break;
            case Trochoid:
                ret = trochoid(t);
                break;
            case Ravdoid:
                ret = ravdoid(t);
                break;
        }

        return ret.rotate(start, angle);
    }

    /**
     * Calculate the x coordinate on this curve for time t.
     * 
     * @param t the time
     * @return x coordinate
     */
    public double fx(double t) {
        return f(t).x;
    }

    /**
     * Calculate the y coordinate on this curve for time t.
     * 
     * @param t the time
     * @return y coordinate
     */
    public double fy(double t) {
        return f(t).y;
    }
    
    /**
     * Calculate a list of nrpoints points of this curve to approximate its
     * shape.
     * The more points, the better the approximation.
     * 
     * @param nrpoints Number of points to calculate
     * @return list of points
     */
    public List<Point> getPoints (int nrpoints) {
        List<Point> ret = new LinkedList();
        if (type.equals("S"))
            nrpoints = 1;
        for (int i = 0; i < nrpoints; i++) {
            double t = ((double)i) / ((double)nrpoints);
            ret.add(f(t));
        }
        
        return ret;
    }
    
    /**
     * Construct an RCurve from its nested list representation.
     * 
     * @param nl Nested List representation
     * @return rcurve object
     */
    public static RCurve fromNL(NL nl) {
        double sx = nl.get(0).getNr();
        double sy = nl.get(1).getNr();
        double angle = nl.get(2).getNr();
        SegmentType type = SegmentType.fromString(nl.get(3).getSym());
        double[] params = null;
        if (nl.size() > 4) {
            params = new double[nl.size() - 4];
            for (int i = 4; i < nl.size(); i++) {
                params[i - 4] = nl.get(i).getNr();
            }
        }

        return new RCurve(new Point(sx, sy), angle, type, params);
    }
    
    /**
     * Convert this RCurve to its nested list representation
     * 
     * @return nested list representation of this rcurve
     */
    public NL toNL() {
        NL nl = new NL();
        nl.addNr(start.x);
        nl.addNr(start.y);
        nl.addNr(angle);
        nl.addSym(type.toString());
        for (double d : params) {
            nl.addNr(d);
        }
        
        return nl;
    }
    
    /**
     * Get a string representation of this RCurve
     * 
     * @return string representation
     */
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("( ").append(start.x).append(" ")
                .append(start.y).append(" ").append(angle).append(" ").append(type);
        if (params != null) {
            for (double v : params) {
                sb.append(" ").append(v);
            }
        }
        sb.append(" )");
        return sb.toString();
    }

    /**
     * Get the angle of this RCurve segment
     * 
     * @return angle of segment
     */
    public double getAngle() {
        return angle;
    }

    /**
     * Set the angle of this RCurve segment
     * 
     * @param angle angle of segment
     */
    public void setAngle(double angle) {
        this.angle = angle;
    }
}
