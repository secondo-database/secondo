
package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  viewer.*;
import  viewer.hoese.*;
import  sj.lang.ListExpr;
import  java.util.*;


/**
 * A displayclass for the movingpoint-type (spatiotemp algebra), 2D with TimePanel
 */
public class Dsplmovingpoint extends DisplayTimeGraph {
  //AffineTransform internAT;
  Point2D.Double point;
  Vector PointMaps;
  Rectangle2D.Double bounds;

  /**
   * Gets the shape of this instance at the ActualTime
   * @param at The actual transformation, used to calculate the correct size.
   * @return Rectangle or Circle Shape if ActualTime is defined otherwise null.
   * @see <a href="Dsplmovingpointsrc.html#getRenderObject">Source</a>
   */
  public Shape getRenderObject (AffineTransform at) {
    double t = RefLayer.getActualTime();
    PointMap pm = (PointMap)getMapAt(t, Intervals, PointMaps);
    if (pm == null) {
      RenderObject = null;
      return  RenderObject;
    }
    point = new Point2D.Double(pm.ax*t + pm.bx, pm.ay*t + pm.by);
    double pixy = Math.abs(Cat.getPointSize()/at.getScaleY());
    //double pix = Cat.getPointSize();
    double pix = Math.abs(Cat.getPointSize()/at.getScaleX());
    //Point2D p=at.transform(point,p);
    if (Cat.getPointasRect())
      RenderObject = new Rectangle2D.Double(point.getX()- pix/2, point.getY() - pixy/2, pix, pixy); 
    else {
      RenderObject = new Ellipse2D.Double(point.getX()- pix/2, point.getY() - pixy/2, pix, pixy);
    }
    return  RenderObject;
  }

  /**
   * Reads the coefficients out of ListExpr for a map
   * @param le ListExpr of four reals.
   * @return The PointMap that was read.
   * @see <a href="Dsplmovingpointsrc.html#readPointMap">Source</a>
   */
  private PointMap readPointMap (ListExpr le) {
    Double value[] =  {
      null, null, null, null
    };
    if (le.listLength() != 4)
      return  null;
    for (int i = 0; i < 4; i++) {
      value[i] = LEUtils.readNumeric(le.first());
      if (value[i] == null)
        return  null;
      le = le.rest();
    }
    return  new PointMap(value[0].doubleValue(), value[2].doubleValue(), value[1].doubleValue(), 
        value[3].doubleValue());
  }

  /**
   * Scans the representation of a movingpoint datatype 
   * @param v A list of start and end intervals with ax,bx,ay,by values
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingpointsrc.html#ScanValue">Source</a>
   */
  private void ScanValue (ListExpr v) {
    err = true;
    if (v.isEmpty())
      return;
    while (!v.isEmpty()) {      // unit While maybe empty
      ListExpr aunit = v.first();
      //     System.out.println(aunit.writeListExprToString());
      if (aunit.listLength() != 8)
        return;
      Interval in = LEUtils.readInterval(ListExpr.fourElemList(aunit.first(), 
          aunit.second(), aunit.third(), aunit.fourth()));
      aunit = aunit.rest().rest().rest().rest();
      //      System.out.println(aunit.writeListExprToString());
      PointMap pm = readPointMap(ListExpr.fourElemList(aunit.first(), aunit.second(), 
          aunit.third(), aunit.fourth()));
      if ((in == null) || (pm == null))
        return;
      Intervals.add(in);
      PointMaps.add(pm);
      v = v.rest();
    }
    err = false;
  }

  /**
   * Init. the Dsplmovingpoint instance and calculate the overall bounds and Timebounds
   * @param type The symbol movingpoint
   * @param value A list of start and end intervals with ax,bx,ay,by values
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingpointsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ispointType = true;         //to create the desired form
    Intervals = new Vector(10, 5);
    PointMaps = new Vector(10, 5);
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GTA(mpoint))"));
      return;
    } 
    else 
      qr.addEntry(this);
    //ListIterator li=iv.listIterator();
    bounds = null;
    TimeBounds = null;
    for (int j = 0; j < Intervals.size(); j++) {
      Interval in = (Interval)Intervals.elementAt(j);
      Interval i = new Interval(in.getStart() + 0.0001, in.getEnd() - 0.0001, 
          true, true);
      PointMap pm = (PointMap)PointMaps.elementAt(j);
      Rectangle2D.Double r = new Rectangle2D.Double(pm.ax*i.getStart() + pm.bx, 
          pm.ay*i.getStart() + pm.by, 0, 0);
      r = (Rectangle2D.Double)r.createUnion(new Rectangle2D.Double(pm.ax*i.getEnd()
          + pm.bx, pm.ay*i.getEnd() + pm.by, 0, 0));
      if (bounds == null) {
        bounds = r;
        TimeBounds = in;
      } 
      else {
        bounds = (Rectangle2D.Double)bounds.createUnion(r);
        TimeBounds = TimeBounds.union(in);
      }
    }
  }

  /** 
   * @return The overall boundingbox of the movingpoint
   * @see <a href="Dsplmovingpointsrc.html#getBounds">Source</a>
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }

  class PointMap {
    double ax, bx, ay, by;

    /**
     * Constructor
     * @param     double x1
     * @param     double x2
     * @param     double y1
     * @param     double y2
     */
    public PointMap (double x1, double x2, double y1, double y2) {
      ax = x1;
      bx = x2;
      ay = y1;
      by = y2;
    }
  }
}



