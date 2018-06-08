package viewer.hoese.algebras.pmregion;

import java.awt.Polygon;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import java.awt.geom.Rectangle2D;
import java.util.Vector;
import sj.lang.ListExpr;
import viewer.hoese.Interval;
import viewer.hoese.QueryResult;
import viewer.hoese.algebras.DisplayTimeGraph;
import viewer.hoese.algebras.fixedmregion.Face;
import viewer.hoese.algebras.fixedmregion.NL;
import viewer.hoese.algebras.fixedmregion.Region;
import viewer.hoese.algebras.fixedmregion.Seg;

/**
 *
 * Base class for displaying an PMRegion object.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class DisplayPMRegion extends DisplayTimeGraph {
    /** The fmregion to display */
    private PMRegion pmregion;
    /** The bounds of this PMRegion */
    private Rectangle2D.Double bounds;
    
    @Override
    public int numberOfShapes() {
        return 1;
    }

    @Override
    public Shape getRenderObject(int num, AffineTransform at) {
        double t = (RefLayer.getActualTime()+10959)*86400000;
        if (pmregion == null)
            return null;
        Region r = pmregion.project(t);
        Area a = new Area();
        if (r != null) {
            for (int i = 0; i < r.getFaces().size(); i++) {
                a.add(getAreaFromFace(r.getFaces().get(i)));
            }
        }
        
        return a;
    }
    
    /**
     * Constructs a displayable Area object from a face.
     * 
     * @param face The face to construct the area from
     * @return The Area to display
     */
    private Area getAreaFromFace(Face face) {
        Polygon p = new Polygon();
        
        for (Seg s : face.getSegments()) {
            p.addPoint((int)s.s.x, (int)s.s.y);
        }
        Area a = new Area(p);
        for (Face h : face.getHoles()) {
            p = new Polygon();
            for (Seg s : h.getSegments()) {
                p.addPoint((int)s.s.x, (int)s.s.y);
            }
            a.subtract(new Area(p));
        }
        
        return a;
    }
    
    @Override
    public void init (String name, int nameWidth, int indent, ListExpr type, 
                      ListExpr value, QueryResult qr) {
        super.init(name, nameWidth, indent, type, value, qr);
        NL nl = new NL(value);
        pmregion = PMRegion.deserialize(nl);
        
        qr.addEntry(this);
        viewer.hoese.Interval iv = new 
               viewer.hoese.Interval(pmregion.min/86400000 - 10959,
                                     pmregion.max/86400000 - 10959, true, true
               );
	setBoundingInterval(iv);
        Vector intervals = new Vector();
        intervals.add(iv);
        setIntervals(intervals);
        Seg b = pmregion.getBoundingBox();
        bounds = new Rectangle2D.Double(b.s.x, b.s.y, b.e.x-b.s.x, b.e.y-b.s.y);
    }
    
        
  /**
   * @return The overall boundingbox of the movingregion
   * @see <a href="Dsplmovingregionsrc.html#getBounds">Source</a>
   */
    @Override
  public Rectangle2D.Double getBounds () {
    return bounds;
  }
}
