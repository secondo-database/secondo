package viewer.hoese.algebras.pmregion;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class PMRegion {
    private List<Triangle> triangles = new ArrayList();
    private Seg bb;
    public Double min, max;
    
    Region project (double t) {
        SegSet ss = new SegSet();
        
        for (Triangle tri : triangles) {
            Seg s = tri.project(t);
            if (s != null)
                ss.add(s);
        }
        
        int faces = 0;
        Region reg = new Region();
        while (!ss.isEmpty()) {
            Seg prev = ss.getSomeSeg();
            Point start = prev.s, last = null;
            Face f = new Face();
            f.addPoint(prev.e);
            while (!ss.isEmpty()) {
                Seg cur = ss.getSuccessor(prev);
                if (cur == null) {
                    System.err.println("ERROR: No successor found!");
                    break;
                }
            
                f.addPoint(cur.e);
                if (start.equals(cur.e)) {
                    f.close();
//                    if (bb != null && f.getBoundingBox().overlaps(bb)) {
//                        f.ccw();
                        reg.addFace(f);
                        faces++;
//                    }
                    break;
                } else {
                }
                prev = cur;
            }
        }
        System.out.println("Rendered "+faces+" faces");

//        reg.fixHoles(); // FIXME!
        return reg;
    }
    
    
    static PMRegion deserialize (NL nl) {
        PMRegion ret = new PMRegion();
        NL pointslist = nl.get(0);
        NL facelist = nl.get(1);
        Point3D[] points = new Point3D[pointslist.size()];
        
        ret.bb = null;
        
        for (int i = 0; i < pointslist.size(); i++) {
            NL pl = pointslist.get(i);
            double x = pl.get(0).getNr();
            double y = pl.get(1).getNr();
            double z = pl.get(2).getNr();
            if (ret.bb == null) {
                ret.bb = new Seg(x, y, x, y);
            } else {
                if (ret.bb.s.x > x)
                    ret.bb.s.x = x;
                if (ret.bb.e.x < x)
                    ret.bb.e.x = x;
                if (ret.bb.s.y > y)
                    ret.bb.s.y = y;
                if (ret.bb.e.y < y)
                    ret.bb.e.y = y;
            }
            if (ret.min == null || ret.min > z)
                ret.min = z;
            if (ret.max == null || ret.max < z)
                ret.max = z;
            points[i] = new Point3D(x, y, z);
        }
        
        for (int i = 0; i < facelist.size(); i++) {
            NL tl = facelist.get(i);
            int t1 = (int)Math.round(tl.get(0).getNr());
            int t2 = (int)Math.round(tl.get(1).getNr());
            int t3 = (int)Math.round(tl.get(2).getNr());
            ret.triangles.add(new Triangle(points[t1], points[t2], points[t3]));
        }

        return ret;
    }
    
    Seg getBoundingBox () {
        return bb;
    }
}
