package  viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import viewer.*;
import viewer.hoese.*;
import sj.lang.ListExpr;
import java.util.*;
import gui.Environment;
import tools.Reporter;


/**
 * A displayclass for the cmpoint  for TemporalAlgebra
 */
public class Dsplcmpointregion extends DisplayTimeGraph implements LabelAttribute, RenderAttribute {
  Point2D.Double point;
  Area area;
  private Path2D.Double rect;
  private Shape shp;
  Vector PointMaps;
  Rectangle2D.Double bounds;
  double minValue = Integer.MAX_VALUE;
  double maxValue = Integer.MIN_VALUE;
  boolean defined;
  double radius = 0.0, dir=0;
  static java.text.DecimalFormat format = new java.text.DecimalFormat("#.#####");
  public int numberOfShapes(){
    return 1;
  }

  /** Returns a short text usable as label **/
  public String getLabel(double time){
    if(Intervals==null || PointMaps==null){
      return null;
    }
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    if(index<0){
      return null;
    }
    CPointMap pm = (CPointMap) PointMaps.get(index);
    Interval in = (Interval)Intervals.get(index);
    double t1 = in.getStart();
    double t2 = in.getEnd();
    double Delta = (time-t1)/(t2-t1);
    double e = pm.e;
    double x = pm.x1+Delta*(pm.x2-pm.x1);
    double y = pm.y1+Delta*(pm.y2-pm.y1);
    int hSign = pm.x1 < pm.x2 ? 1 :-1;
    int vSign = pm.y1 < pm.y2 ? 1 :-1;
    dir = Math.atan2(pm.y2 - pm.y1, pm.x2 - pm.x1) * Math.PI / 180 ;
    double tmpx = pm.x1 - hSign * Math.abs(Math.sin(dir)) * pm.e;   // em.x1+delta*(em.x2-em.x1);
    double tmpy = pm.y1 - vSign * Math.abs(Math.cos(dir)) * pm.e;   //
    return "(("+format.format(x)+", "+ format.format(y)+")"+format.format(e)+")";
    // return "("+format.format(e)+"("+format.format(tmpx)+", "+ format.format(tmpy)+"))"+ " DIR " + dir + " Math.sin(dir) " + Math.sin(dir) +  " Math.cos(dir) "+ Math.cos(dir) ;// +   "Index" + index + " t1" + format.format(t1) + " t2 "+ format.format(t2) + "  Delta " + format.format(Delta) + " time " + format.format(time) + " CMPoint "+ pm +  " PointMap " + PointMaps.size();

  }

  /**
   * Gets the shape of this instance at the ActualTime
   * @param at The actual transformation, used to calculate the correct size.
   * @return Rectangle or Circle Shape if ActualTime is defined otherwise null.
   */


  public Shape getRenderObject(int num,AffineTransform at) {
    if(num!=0){
      return null;
    }
    if(!defined){
      return null;
    }
    return  shp;
  }




  /**
   * Reads the coefficients out of ListExpr for the point and radius
   * @param le ListExpr of four reals.
   * @return The CPointMap that was read.
   */
  private CPointMap readCPointMap(double e, ListExpr le) {
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
    double x1, y1;

    double v0 = value[0].doubleValue();
    double v1 = value[1].doubleValue();
    double v2 = value[2].doubleValue();
    double v3 = value[3].doubleValue();
    if(minValue>v0) minValue=v0;
    if(maxValue<v0) maxValue=v0;
    if(minValue>v2) minValue=v2;
    if(maxValue<v2) maxValue=v2;

    if(!ProjectionManager.project(value[0].doubleValue(),value[1].doubleValue(),aPoint)){
      return null;
    }
    x1 = aPoint.x;
    y1 = aPoint.y;
    if(!ProjectionManager.project(value[2].doubleValue(),value[3].doubleValue(),aPoint)){
      return null;
    }
    return  new CPointMap(e,x1,y1,aPoint.x,aPoint.y);
  }

  /**
   * Scans the representation of a movingpoint datatype
   * @param v A list of start and end intervals with ax,bx,ay,by values
   * @see sj.lang.ListExpr
   */
  private void ScanValue (ListExpr v) {
    err = true;
    radius = 0.0;
    double tmpepsilon;
    if (v.isEmpty()){ //empty point
      Intervals=null;
      PointMaps=null;
      err=false;
      defined = false;
      return;
    }
    while (!v.isEmpty()) {      // unit While maybe empty
      ListExpr acunit = v.first();
      ListExpr tmp = v.first();
      int L = acunit.listLength();
      if(L!=3){
        Reporter.debug("wrong ListLength in reading cmpoint unit");
        defined = false;
        return;
      }
      // deprecated version of external representation
      Interval in=null;
      CPointMap pm=null;
      // the corrected version of external representation
      if(L==3){
        radius = LEUtils.readNumeric(acunit.third()).doubleValue();
        ListExpr aunit = acunit.second();
        int Ll = aunit.listLength();
        if(Ll!=4){
          Reporter.debug("wrong ListLength in reading  cmpoint unit");
          defined = false;
          return;
        }
        in = LEUtils.readInterval(acunit.first());
        pm = readCPointMap(radius, acunit.second());
      }

      if ((in == null) || (pm == null)){

        Reporter.debug("Error in reading Unit");
        Reporter.debug(tmp.writeListExprToString());
        if(in==null){
          Reporter.debug("Error in reading interval");
        }
        if(pm==null){
          Reporter.debug("Error in reading Start and EndPoint");
        }
        defined = false;
        return;
      }
      Intervals.add(in);
      PointMaps.add(pm);
      v = v.rest();
    }
    err = false;
    defined = true;

// create the region
    GeneralPath GP = new GeneralPath(GeneralPath.WIND_EVEN_ODD,PointMaps.size());
    try{
      for(int i=0;i<PointMaps.size();i++)
      {
      CPointMap pm = (CPointMap) PointMaps.get(i);
      int hSign = pm.x1 < pm.x2 ? 1 :-1;
      int vSign = pm.y1 < pm.y2 ? 1 :-1;
      dir = Math.atan2(pm.y2 - pm.y1, pm.x2 - pm.x1) * Math.PI / 180 ;
      double tmpx = pm.x1 - hSign * Math.abs(Math.sin(dir)) * pm.e;   // em.x1+delta*(em.x2-em.x1);
      double tmpy = pm.y1 - vSign * Math.abs(Math.cos(dir)) * pm.e;   //
        GP.moveTo(tmpx, tmpy);
        GP.lineTo(pm.x1 +  hSign * Math.abs(Math.sin(dir)) * pm.e, pm.y1 +  vSign * Math.abs(Math.cos(dir)) * pm.e );
        GP.lineTo(pm.x2 + hSign * Math.abs(Math.sin(dir)) * pm.e ,pm.y2 + vSign * Math.abs(Math.cos(dir)) * pm.e );
        GP.lineTo(pm.x2  - hSign * Math.abs(Math.sin(dir)) * pm.e ,pm.y2  - vSign * Math.abs(Math.cos(dir)) * pm.e);


      }
      } catch(Exception e){
        Reporter.writeError("wrong parameter for choosed projection");
    }
    GP.closePath();
    shp =  new Area(GP);
  }

  public boolean isPointType(int num){
    return true;
  }

  public void init (String name, int nameWidth, int indent,
                    ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    int length = value.listLength();
    double xlow;
    double ylow;
    double xhigh;
    double yhigh;
    Intervals = new Vector(length+2);
    PointMaps = new Vector(length+2);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Dsplcmpoint Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": (cmpoint))"));
      return;
    }
    else
      qr.addEntry(this);
    //ListIterator li=iv.listIterator();
    bounds = null;
    TimeBounds = null;
    if(Intervals==null) // empty moving point
      return;
    for (int j = 0; j < Intervals.size(); j++) {
      Interval in = (Interval)Intervals.elementAt(j);
      CPointMap pm = (CPointMap)PointMaps.elementAt(j);

      // To expand the bounding box by the epsilon-value, it is necessary to sort the x- and y-values:
      if (pm.x1 <= pm.x2) {
        xlow = pm.x1 - radius;
        xhigh = pm.x2 + radius;
      } else {
        xlow = pm.x2 - radius;
        xhigh = pm.x1 + radius;
      }
      if (pm.y1 <= pm.y2) {
        ylow = pm.y1 - radius;
        yhigh = pm.y2 + radius;
      } else {
        ylow = pm.y2 - radius;
        yhigh = pm.y1 + radius;
      }
      // Create the bounding box from the expanded coordinates:
      Rectangle2D.Double r = new Rectangle2D.Double(xlow,ylow,0,0);
      r = (Rectangle2D.Double)r.createUnion(new Rectangle2D.Double(xhigh,yhigh,0,0));
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
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }

  /** returns the minimum x value **/
  public double getMinRenderValue(){
    return minValue;
  }
  /** returns the maximum x value **/
  public double getMaxRenderValue(){
    return maxValue;
  }
  /** returns the current x value **/
  public double getRenderValue(double time){
    if(Intervals==null || PointMaps==null){
      return 0;
    }
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    if(index<0){
      return 0;
    }
    CPointMap pm = (CPointMap) PointMaps.get(index);
    Interval in = (Interval)Intervals.get(index);
    double t1 = in.getStart();
    double t2 = in.getEnd();
    double Delta = (time-t1)/(t2-t1);
    double x = pm.x1+Delta*(pm.x2-pm.x1);
    return  x;
  }

  public boolean mayBeDefined(){
    return defined;
  }

  public boolean isDefined(double time){
    if(!defined){
      return false;
    }
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    return index>=0;
  }


  class CPointMap{
    double e, x1,x2,y1,y2;

    public CPointMap(double e, double x1, double y1, double x2, double y2) {
      this.e = e;
      this.x1 = x1;
      this.y1 = y1;
      this.x2 = x2;
      this.y2 = y2;
    }

    public String toString(){
      return ("([x1,y1 | x2,y2]epsilon) = (["+x1+","+y1+" <> "+x2+","+y2+"] "+e+")");
    }
  }
}



