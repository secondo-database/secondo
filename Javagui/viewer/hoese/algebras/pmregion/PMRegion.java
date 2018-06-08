package viewer.hoese.algebras.pmregion;

import java.util.LinkedList;
import java.util.List;
import viewer.hoese.algebras.fixedmregion.Face;
import viewer.hoese.algebras.fixedmregion.NL;
import viewer.hoese.algebras.fixedmregion.Point;
import viewer.hoese.algebras.fixedmregion.Region;
import viewer.hoese.algebras.fixedmregion.Seg;

/**
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class PMRegion {
    private List<Triangle> triangles = new LinkedList();
    private Seg bb;
    public Double min, max;
    
    Region project (double t) {
        List<Seg> segs = new LinkedList();
        for (Triangle tri : triangles) {
            Seg s = tri.project(t);
            if (s != null)
                segs.add(s);
        }
        
        int faces = 0;
        Region reg = new Region();
        while (!segs.isEmpty()) {
            Seg prev = segs.get(0);
            Point start = prev.s, last = null;
            Face f = new Face();
            f.addPoint(prev.e);
            segs.remove(prev);
            while (!segs.isEmpty()) {
                boolean found = false;
                for (int i = 0; i < segs.size(); i++) {
                    Seg cur = segs.get(i);
                    if (cur.s.equals(prev.e)) {
                        f.addPoint(cur.e);
                        prev = cur;
                        segs.remove(cur);
                        found = true;
                        last = cur.e;
                    } else if (cur.e.equals(prev.e)) {
                        f.addPoint(cur.s);
                        prev = new Seg(cur.e, cur.s);
                        segs.remove(cur);
                        found = true;
                        last = cur.s;
                    }
                }
                if (!found) {
                    System.err.println("ERROR: No successor found!");
                    return null;
                }
                if (start.equals(last)) {
                    f.close();
                    reg.addFace(f);
                    faces++;
                    break;
                }
            }
        }
        
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
