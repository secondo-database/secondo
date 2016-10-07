package viewer.hoese.algebras.fixedmregion;

import java.awt.Polygon;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import java.awt.geom.Rectangle2D;
import java.util.List;
import java.util.Vector;
import sj.lang.ListExpr;
import viewer.hoese.QueryResult;
import viewer.hoese.algebras.DisplayTimeGraph;

/**
 *
 * Base class for displaying an FMRegion object.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class DisplayFMRegion extends DisplayTimeGraph {
    /** The fmregion to display */
    private FMRegion fmregion;
    /** The bounds of this FMRegion */
    private Rectangle2D.Double bounds;
    
    @Override
    public int numberOfShapes() {
        return fmregion.getRegion().getFaces().size();
    }

    @Override
    public Shape getRenderObject(int num, AffineTransform at) {
        double t = (RefLayer.getActualTime()+10959)*86400000;
        if (fmregion == null)
            return null;
        Region r = fmregion.project(t);
        Area a = null;
        if (r != null) {
            a = getAreaFromFace(r.getFaces().get(num));
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
        System.out.println(nl.toString());
        fmregion = FMRegion.deserialize(nl);
        
        qr.addEntry(this);
        List<FMRegionTrans> fmt = fmregion.getTransformations();
        int s = fmt.size();
        if (s > 0) {
            viewer.hoese.Interval siv = Interval2Interval(fmt.get(0).getIv());
            viewer.hoese.Interval eiv = Interval2Interval(fmt.get(s-1).getIv());
            viewer.hoese.Interval iv = new viewer.hoese.Interval(siv.getStart(),
                    eiv.getEnd(), siv.isLeftclosed(), eiv.isRightclosed());
            setBoundingInterval(iv);
        }
        Vector intervals = new Vector();
        for (FMRegionTrans t : fmt) {
            intervals.add(Interval2Interval(t.getIv()));
        }
        setIntervals(intervals);
        Seg b = fmregion.getBoundingBox();
        bounds = new Rectangle2D.Double(b.s.x, b.s.y, b.e.x-b.s.x, b.e.y-b.s.y);
    }
    
    /**
     * Converts a libfmr interval to a hoese.Interval
     * 
     * @param iv The libfmr interval
     * @return The hoese interval
     */
    private viewer.hoese.Interval Interval2Interval (Interval iv) {
        double start = iv.getStart()/86400000 - 10959;
        boolean lc = iv.isLeftClosed();
        double end = iv.getEnd()/86400000 - 10959;
        boolean rc = iv.isRightClosed();
        
        return new viewer.hoese.Interval(start, end, lc, rc);
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
