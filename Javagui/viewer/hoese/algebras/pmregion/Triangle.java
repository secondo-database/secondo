/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.hoese.algebras.pmregion;

import java.util.Arrays;
import java.util.Comparator;
import viewer.hoese.algebras.fixedmregion.Point;
import viewer.hoese.algebras.fixedmregion.Seg;

/**
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class Triangle {
    public Point3D[] points;
    
    public Triangle (Point3D x1, Point3D x2, Point3D x3) {
        points = new Point3D[3];
        points[0] = x1;
        points[1] = x2;
        points[2] = x3;
        Arrays.sort(points, new Comparator<Point3D>() {
            @Override
            public int compare (Point3D p1, Point3D p2) {
                return (p1.z < p2.z) ? -1 : ((p1.z > p2.z) ? 1 : 0);
            }
        });
        
    }
    
    public double[] zrange () {
        return new double[] {points[0].z, points[2].z};
    }
    
    public double zdist () {
        return points[2].z - points[0].z;
    }
    
    public double frac (double t) {
        return (t - points[0].z)/zdist();
    }
    
    public Seg project (double t) {
        if (t <= points[0].z || t >= points[2].z)
            return null;
        
        Point3D h1 = new Point3D();
        Point3D h2 = new Point3D();
        double frac1 = (t - points[0].z) / (points[2].z - points[0].z);
        h1.x = points[0].x + (points[2].x - points[0].x)*frac1;
        h1.y = points[0].y + (points[2].y - points[0].y)*frac1;
        h1.z = t;
        
        if (t < points[1].z) {
            double frac2 = (t - points[0].z) / (points[1].z - points[0].z);
            h2.x = points[0].x + (points[1].x - points[0].x)*frac2;
            h2.y = points[0].y + (points[1].y - points[0].y)*frac2;
            h2.z = t;
        } else {
            double frac2 = (t - points[1].z) / (points[2].z - points[1].z);
            h2.x = points[1].x + (points[2].x - points[1].x)*frac2;
            h2.y = points[1].y + (points[2].y - points[1].y)*frac2;
            h2.z = t;
        }
        
        return new Seg(new Point(h1.x, h1.y), new Point(h2.x, h2.y));
    }
    
}



