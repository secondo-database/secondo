package viewer.v3d;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import javax.vecmath.Point2d;
import javax.vecmath.Point3d;
import sj.lang.ListExpr;

public class Face {

    static public final double RANGEX = 5,
            RANGEY = 5,
            RANGEZ = 5;

    static List<Point3d> MRegionList2Triangles(ListExpr le,
            boolean compensate) {
        List<Point3d> ret = new LinkedList();

        String type = le.first().textValue();
        le = le.second();

        if (type.equals("mregion")) {
            ListExpr p = le.first();
            while (p != null) {
                le = le.rest();
                ListExpr interval = p.first();

                String start = interval.first().textValue();
                String stop = interval.second().textValue();

                double z1 = parseInstant(start);
                double z2 = parseInstant(stop);

                ListExpr uregion = p.second();
                ret.addAll(FacesList2Triangles(uregion, z1, z2, compensate));
                p = le.first();
            }
        } else if (type.equals("uregion")) {
            ListExpr p = le;
            ListExpr interval = p.first();

            String start = interval.first().textValue();
            String stop = interval.second().textValue();

            double z1 = parseInstant(start);
            double z2 = parseInstant(stop);

            ListExpr uregion = p.second();
            ret.addAll(FacesList2Triangles(uregion, z1, z2, compensate));
            p = le.first();
        }

        FixCoordinates(ret);

        // Draw an arrow for orientation
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1, -RANGEZ / 2));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1 + 0.1, -RANGEZ / 2));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1 + 0.05, RANGEZ / 2));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1, RANGEZ / 2 * 0.9));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1 + 0.1, RANGEZ / 2 * 0.95));
        ret.add(new Point3d(-RANGEX / 2 * 1.1, -RANGEY / 2 * 1.1 + 0.05, RANGEZ / 2));

        return ret;
    }

    static void FixCoordinates(List<Point3d> l) {
        double minx, maxx, miny, maxy, minz, maxz;
        Point3d fp = l.get(0);
        minx = maxx = fp.x;
        miny = maxy = fp.y;
        minz = maxz = fp.z;

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
        double offx = -minx;
        double offy = -miny;
        double offz = -minz;
        double scalex = RANGEX / (maxx - minx);
        double scaley = RANGEY / (maxy - miny);
        double scalez = RANGEZ / (maxz - minz);

        for (int i = 0; i < l.size(); i++) {
            Point3d p = l.get(i);
            p.x = (p.x + offx) * scalex - RANGEX / 2;
            p.y = (p.y + offy) * scaley - RANGEY / 2;
            p.z = (p.z + offz) * scalez - RANGEZ / 2;
        }
    }

    static void CompensateTranslation(List<Point3d> l, double z1, double z2) {
        Point2d[] off = new Point2d[2];

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

        for (Point3d p : l) {
            int i = (p.z == z1) ? 0 : 1;
            p.x -= off[i].x;
            p.y -= off[i].y;
        }
    }

    static List<Point3d> FacesList2Triangles(ListExpr le, double z1, double z2,
            boolean compensate) {
        List<Point3d> ret = new LinkedList();

        ListExpr p = le.first();
        while (p != null) {
            le = le.rest();
            ret.addAll(FaceList2Triangles(p, z1, z2));
            p = le.first();
        }

        if (compensate) {
            CompensateTranslation(ret, z1, z2);
        }

        return ret;
    }

    static List<Point3d> FaceList2Triangles(ListExpr le, double z1, double z2) {
        List<Point3d> ret = new LinkedList();

        ListExpr p = le.first();
        while (p != null) {
            le = le.rest();
            ret.addAll(List2Triangles(p, z1, z2));
            p = le.first();
        }

        return ret;
    }

    static List<Point3d> List2Triangles(ListExpr le, double z1, double z2) {
        List<Point3d> ret = new LinkedList();

        List<HalfSeg> lhs = List2HS(le);
        for (int i = 0; i < lhs.size() - 1; i++) {
            ret.addAll(HS2Triangles(lhs.get(i), lhs.get(i + 1), z1, z2));

        }

        return ret;
    }

    static List<HalfSeg> List2HS(ListExpr le) {
        List<HalfSeg> ret = new LinkedList();
        System.out.println(le.toString());

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

    static List<Point3d> HS2Triangles(HalfSeg h1, HalfSeg h2, double z1, double z2) {
        List<Point3d> ret = new LinkedList();

        Point2d i1 = h1.Initial();
        Point2d f1 = h1.Final();
        Point2d i2 = h2.Initial();
        Point2d f2 = h2.Final();

        if (i1.equals(i2)) {
            ret.add(new Point3d(i1.x, i1.y, z1));
            ret.add(new Point3d(f1.x, f1.y, z2));
            ret.add(new Point3d(f2.x, f2.y, z2));
        } else if (f1.equals(f2)) {
            ret.add(new Point3d(i1.x, i1.y, z1));
            ret.add(new Point3d(i2.x, i2.y, z1));
            ret.add(new Point3d(f2.x, f2.y, z2));
        } else {
            ret.add(new Point3d(i1.x, i1.y, z1));
            ret.add(new Point3d(f1.x, f1.y, z2));
            ret.add(new Point3d(f2.x, f2.y, z2));
            ret.add(new Point3d(i1.x, i1.y, z1));
            ret.add(new Point3d(i2.x, i2.y, z1));
            ret.add(new Point3d(f2.x, f2.y, z2));
        }

        return ret;
    }

    static class HalfSeg {

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

    static double parseInstant(String instant) {
        String format;
        int len = instant.length();

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
