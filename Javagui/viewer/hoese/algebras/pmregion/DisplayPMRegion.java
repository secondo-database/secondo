package viewer.hoese.algebras.pmregion;

import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import java.awt.geom.Rectangle2D;
import java.util.Vector;
import sj.lang.ListExpr;
import viewer.hoese.QueryResult;
import viewer.hoese.algebras.DisplayTimeGraph;

/**
 *
 * Base class for displaying an PMRegion object.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class DisplayPMRegion extends DisplayTimeGraph {
    /** The pmregion to display */
    private PMRegion pmregion;
    /** The bounds of this PMRegion */
    private Rectangle2D.Double bounds;
    
    private double cachetime = Double.NaN;
    private Region cache;
    
    private void updateCache () {
        double t = (RefLayer.getActualTime()+10959f)*86400000f;
        if (cachetime != t && pmregion != null) {
            cache = pmregion.project(t);
            cachetime = t;
        }
    }
    
    @Override
    public int numberOfShapes() {
        updateCache();
        if (cache != null)
            return cache.getFaces().size();
        else
            return 0;
    }

    @Override
    public Shape getRenderObject(int num, AffineTransform at) {
        updateCache();
        
        return cache.getFaces().get(num).getAreaObj();
    }
    
//    @Override
//    public Shape getRenderObject(int num, AffineTransform at) {
//        
//    }
    
    @Override
    public void init (String name, int nameWidth, int indent, ListExpr type, 
                      ListExpr value, QueryResult qr) {
        super.init(name, nameWidth, indent, type, value, qr);
        NL nl = new NL(value);
        pmregion = PMRegion.deserialize(nl);
        
        qr.addEntry(this);
        viewer.hoese.Interval iv = new 
               viewer.hoese.Interval(pmregion.min/86400000f - 10959f,
                                     pmregion.max/86400000f - 10959f, true, true
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
