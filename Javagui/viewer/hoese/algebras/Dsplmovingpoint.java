
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

  /** returns the first index in Intervals containing t
    * if not an intervals containing t exists -1 is returned
    */
  private int getTimeIndex(double t,Vector Intervals){
     for(int i=0;i<Intervals.size();i++)
        if( ((Interval) Intervals.get(i)).isDefinedAt(t))
	    return i;
     return -1;
  }


  /**
   * Gets the shape of this instance at the ActualTime
   * @param at The actual transformation, used to calculate the correct size.
   * @return Rectangle or Circle Shape if ActualTime is defined otherwise null.
   * @see <a href="Dsplmovingpointsrc.html#getRenderObject">Source</a>
   */
  public Shape getRenderObject (AffineTransform at) {
    double t = RefLayer.getActualTime();

    int index = getTimeIndex(t,Intervals);
    if(index<0){
      RenderObject = null;
      return  RenderObject;
    }

    PointMap pm = (PointMap) PointMaps.get(index);
    Interval in = (Interval)Intervals.get(index);
    double t1 = in.getStart();
    double t2 = in.getEnd();
    double Delta = (t-t1)/(t2-t1);
    double x = pm.x1+Delta*(pm.x2-pm.x1);
    double y = pm.y1+Delta*(pm.y2-pm.y1);

    //System.out.println("interval:"+in+"\nTime  "+DateTime.getString(t)+"\n PointMap="+pm);
    //System.out.println("Delta ="+Delta+"\n(x,y)= "+x+","+y+"\n");

    point = new Point2D.Double(x, y);

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
    return  new PointMap(value[0].doubleValue(), value[1].doubleValue(), value[2].doubleValue(),
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
      int L = aunit.listLength();
      if(L!=2 && L!=8)
         return;
      // deprecated version of external representation
      Interval in=null;
      PointMap pm=null;
      if (L == 8){
         System.out.println("Warning: using deprecated external representation of a moving point !");
         in = LEUtils.readInterval(ListExpr.fourElemList(aunit.first(),
                                   aunit.second(), aunit.third(), aunit.fourth()));
         aunit = aunit.rest().rest().rest().rest();
         pm = readPointMap(ListExpr.fourElemList(aunit.first(), aunit.second(),
                           aunit.third(), aunit.fourth()));
      }
      // the corrected version of external representation
      if(L==2){
         in = LEUtils.readInterval(aunit.first());
         pm = readPointMap(aunit.second());
      }

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
      System.out.println("Dsplmovingpoint Error in ListExpr :parsing aborted");
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
      PointMap pm = (PointMap)PointMaps.elementAt(j);
      Rectangle2D.Double r = new Rectangle2D.Double(pm.x1,pm.y1,0,0);
      r = (Rectangle2D.Double)r.createUnion(new Rectangle2D.Double(pm.x2,pm.y2,0,0));



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
    double x1,x2,y1,y2;

    public PointMap (double x1, double y1, double x2, double y2) {
       this.x1 = x1;
       this.y1 = y1;
       this.x2 = x2;
       this.y2 = y2;

    }

    public String toString(){
      return ("[x1,y1 | x2,y2] = ["+x1+","+y1+" <> "+x2+","+y2+"]");
    }
  }
}



