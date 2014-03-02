package viewer.v3d;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import javax.vecmath.Point2d;
import javax.vecmath.Point3d;
import java.awt.Color;
import javax.vecmath.Color3b;
import sj.lang.ListExpr;
import gui.SecondoObject;
import javax.media.j3d.TriangleArray;

/**
 * This class represents a secondo-object, which should be displayed in 3D.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
class Face {

    // Sizes of the bounding box the object is to be scaled in
    static public final double RANGEX = 5,
            RANGEY = 5,
            RANGEZ = 5;

    // The colors of the 3d-triangles, add more colors here if needed
    private final Color3b[] colors = {
        new Color3b(Color.RED),
        new Color3b(Color.BLUE),
        new Color3b(Color.YELLOW),
        new Color3b(Color.GREEN),
        new Color3b(Color.MAGENTA),
        new Color3b(Color.CYAN),
        new Color3b(Color.ORANGE),
        new Color3b(Color.PINK),
        new Color3b(Color.GRAY)
    };

    private SecondoObject o; // The object to be displayed
    private boolean compensateTranslation = false;
    private boolean lightBackground = false;

    /**
     * Construct a new object to be displayed in 3d.
     * 
     * @param o the object to be displayed
     * @param compensateTranslation compensate translation of interval borders
     * @param lightBackground displayed on light background
     */
    Face(SecondoObject o, boolean compensateTranslation, boolean lightBackground) {
        this.o = o;
        this.compensateTranslation = compensateTranslation;
        this.lightBackground = lightBackground;
    }

    /**
     * Construct a new object to be displayed in 3d.
     * 
     * @param o the object to be displayed
     */
    Face(SecondoObject o) {
        this.o = o;
    }

    /**
     * Calculates and builds a Triangle-Array which represents a secondo-object
     * to be displayed by the java3d-framework
     * 
     * @return the array of triangles making up the secondo object
     */
    public TriangleArray GetTriangleArray() {
        List<Point3d> points = MRegionList2Triangles();
        TriangleArray ret = new TriangleArray(points.size(),
                TriangleArray.COORDINATES | TriangleArray.COLOR_3);

        Color3b c = nextColor();
        Color3b white = new Color3b(Color.WHITE);
        Color3b black = new Color3b(Color.BLACK);
        for (int i = 0; i < points.size(); i++) {
            // Each triangle is uniformy colored, so change the color every
            // third iteration only
            if (i % 3 == 0) {
                c = nextColor();
            }
            ret.setCoordinate(i, points.get(i));
            if (i < points.size() - 6) {
                ret.setColor(i, c);
            } else {
                // The last six values are the arrow, which we want to be black
                if (lightBackground) {
                    ret.setColor(i, black);
                } else {            // or white in case the background is black
                    ret.setColor(i, white);
                }
            }
        }

        return ret;
    }

    // Get the color for the next triangle
    private int cnr = 0; // the index of the next color to be displayed
    private Color3b nextColor() {
        return colors[cnr++ % colors.length];
    }

    /**
     * Get a list of corner points for the triangles of the object to display.
     * Three points in a row define one triangle.
     * 
     * @return list of triangle corner points
     */
    private List<Point3d> MRegionList2Triangles() {
        List<Point3d> ret = new LinkedList();
        ListExpr le = o.toListExpr();
        
        String type = le.first().textValue();
        le = le.second();

        if (type.equals("mregion")) {
            // We have an mregion-object which may consist of several intervals
            ListExpr p = le.first();
            while (p != null) {
                le = le.rest();

                // z1 and z2 receive the unix-timestamp of the intervals borders
                ListExpr interval = p.first();
                double z1 = parseInstant(interval.first().textValue());
                double z2 = parseInstant(interval.second().textValue());

                ListExpr uregion = p.second();
                ret.addAll(FacesList2Triangles(uregion, z1, z2));
                p = le.first();
            }
        } else if (type.equals("uregion")) {
            // a uregion consists of only one interval
            ListExpr p = le;

            // z1 and z2 receive the unix-timestamp of the intervals borders
            ListExpr interval = p.first();
            double z1 = parseInstant(interval.first().textValue());
            double z2 = parseInstant(interval.second().textValue());

            ListExpr uregion = p.second();
            ret.addAll(FacesList2Triangles(uregion, z1, z2));
            p = le.first();
        }

        // Now translate and scale the result
        FixCoordinates(ret);

        // Draw an arrow for orientation
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1, -RANGEZ / 2));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1 + 0.1, -RANGEZ / 2));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1 + 0.05, RANGEZ / 2));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1, RANGEZ / 2 * 0.9));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1+0.1, RANGEZ/2*0.95));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1 + 0.05, RANGEZ / 2));

        return ret;
    }

    /**
     * Move and scale the points to fill exactly the bounding box RANGE(X/Y/Z)
     * 
     * @param l A list of points
     */
    private static void FixCoordinates(List<Point3d> l) {
        double minx, maxx, miny, maxy, minz, maxz;
        Point3d fp = l.get(0);
        minx = maxx = fp.x;
        miny = maxy = fp.y;
        minz = maxz = fp.z;

        // Determine the minimum and maximum values of the coordinates for
        // each axis
        for (int i = 1; i < l.size(); i++) {
            Point3d p = l.get(i);
            if (p.x < minx) {
                minx = p.x;
            }
            if (p.x > maxx) {
                maxx = p.x;
            }
            if (p.y < miny) {
                miny = p.y;
            }
            if (p.y > maxy) {
                maxy = p.y;
            }
            if (p.z < minz) {
                minz = p.z;
            }
            if (p.z > maxz) {
                maxz = p.z;
            }
        }
        
        // Calculate offset and scale-factors from the result
        double offx = -minx;
        double offy = -miny;
        double offz = -minz;
        double scalex = RANGEX / (maxx - minx);
        double scaley = RANGEY / (maxy - miny);
        double scalez = RANGEZ / (maxz - minz);

        // and transform all points with that parameters
        for (int i = 0; i < l.size(); i++) {
            Point3d p = l.get(i);
            p.x = (p.x + offx) * scalex - RANGEX / 2;
            p.y = (p.y + offy) * scaley - RANGEY / 2;
            p.z = (p.z + offz) * scalez - RANGEZ / 2;
        }
    }

    /**
     * Transforms the points for each z-value to have the left corner of the
     * bounding box on (0/0).
     * 
     * @param l list of points to be transformed
     */
    private static void CompensateTranslation(List<Point3d> l) {
        Point2d[] off = new Point2d[2];
        double z1 = l.size() > 0 ? l.get(0).z : 0; // Only two z-values are
                                                   // possible, z1 and the other

        // Calculate the offset value for each z-value seperatly
        for (Point3d p : l) {
            int i = (p.z == z1) ? 0 : 1;
            if (off[i] == null) {
                off[i] = new Point2d(p.x, p.y);
            } else {
                if (p.x < off[i].x) {
                    off[i].x = p.x;
                }
                if (p.y < off[i].y) {
                    off[i].y = p.y;
                }
            }
        }

        // Transform the points accordingly
        for (Point3d p : l) {
            int i = (p.z == z1) ? 0 : 1;
            p.x -= off[i].x;
            p.y -= off[i].y;
        }
    }

    /**
     * Transforms several faces in a Nested List to triangles
     * 
     * @param le the nested list to be transformed to triangles
     * @param z1 the z-coordinate of the initial segments
     * @param z2 the z-coordinate of the final segments
     * @return List of triangle corner points
     */
    private List<Point3d> FacesList2Triangles(ListExpr le,
            double z1, double z2) {
        List<Point3d> ret = new LinkedList();

        ListExpr p = le.first();
        while (p != null) {
            le = le.rest();
            ret.addAll(FaceList2Triangles(p, z1, z2));
            p = le.first();
        }

        if (compensateTranslation) {
            CompensateTranslation(ret);
        }

        return ret;
    }

    /**
     * Transforms one face in a Nested List to triangles
     * 
     * @param le the nested list to be transformed to triangles
     * @param z1 the z-coordinate of the initial segments
     * @param z2 the z-coordinate of the final segments
     * @return List of triangle corner points
     */
    private static List<Point3d> FaceList2Triangles(ListExpr le,
            double z1, double z2) {
        List<Point3d> ret = new LinkedList();

        ListExpr p = le.first();
        while (p != null) {
            le = le.rest();
            ret.addAll(List2Triangles(p, z1, z2));
            p = le.first();
        }

        return ret;
    }

    /**
     * Transform a list of Moving Segments to triangles
     * 
     * @param le the nested list to be transformed to triangles
     * @param z1 the z-coordinate of the initial segments
     * @param z2 the z-coordinate of the final segments
     * @return List of triangle corner points
     */
    private static List<Point3d> List2Triangles(ListExpr le,
            double z1, double z2) {
        List<Point3d> ret = new LinkedList();

        List<HalfSeg> lhs = List2HS(le);
        for (int i = 0; i < lhs.size() - 1; i++) {
            ret.addAll(HS2Triangles(lhs.get(i), lhs.get(i + 1), z1, z2));

        }

        return ret;
    }

    /**
     * Transforms a Nested List of Half-Segments to triangles
     * 
     * @param le the nested list to be transformed to triangles
     * @return a list of Half-Segments
     */
    private static List<HalfSeg> List2HS(ListExpr le) {
        List<HalfSeg> ret = new LinkedList();

        ListExpr first = le.first();
        ListExpr p = le.first();
        while (p != null) {
            le = le.rest();
            ret.add(new HalfSeg(p));
            p = le.first();
        }
        ret.add(new HalfSeg(first));

        return ret;
    }

    /**
     * 
     * @param h1 first Half-Segment
     * @param h2 second Half-Segment
     * @param z1 z-coordinate of initial Segment
     * @param z2 z-coordinate of final Segment
     * @return List of points representing one or two triangles
     */
    private static List<Point3d> HS2Triangles(HalfSeg h1, HalfSeg h2,
            double z1, double z2) {
        List<Point3d> ret = new LinkedList();

        Point2d i1 = h1.Initial();
        Point2d f1 = h1.Final();
        Point2d i2 = h2.Initial();
        Point2d f2 = h2.Final();

        if (!i1.equals(i2)) { // Initial segment is not degenerated
            ret.add(new Point3d(i1.x, i1.y, z1));
            ret.add(new Point3d(i2.x, i2.y, z1));
            ret.add(new Point3d(f2.x, f2.y, z2));
        }
        if (!f1.equals(f2)) { // Final segment is not degenerated
            ret.add(new Point3d(i1.x, i1.y, z1));
            ret.add(new Point3d(f1.x, f1.y, z2));
            ret.add(new Point3d(f2.x, f2.y, z2));
        }
        
        return ret;
    }

    /**
     * Helper-class representing one Half-Segment consisting of an initial and
     * a final point.
     * 
     */
    private static class HalfSeg {

        double x1, y1, x2, y2;

        HalfSeg(ListExpr le) {
            x1 = le.first().realValue();
            y1 = le.second().realValue();
            x2 = le.third().realValue();
            y2 = le.fourth().realValue();
        }

        Point2d Initial() {
            return new Point2d(x1, y1);
        }

        Point2d Final() {
            return new Point2d(x2, y2);
        }
    }

    /**
     * Converts an instant-string to a unix-timestamp
     * 
     * @param instant a string representing an instant in time
     * @return the corresponding unix-timestamp
     */
    private static double parseInstant(String instant) {
        String format;
        int len = instant.length();

        // Only these formats can occur
        switch (len) {
            case 10:
                format = "yyyy-MM-dd";
                break;
            case 16:
                format = "yyyy-MM-dd-HH:mm";
                break;
            case 19:
            default:
                format = "yyyy-MM-dd-HH:mm:ss";
                break;
        }

        SimpleDateFormat sdf = new SimpleDateFormat(format);
        try {
            Date s = sdf.parse(instant);
            return (s.getTime() / 1000);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return 0;
    }
}
