package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  viewer.*;
import viewer.hoese.*;
import  sj.lang.ListExpr;
import  java.util.*;

/**
 * A displayclass for the movingregion-type (spatiotemp algebra), 2D with TimePanel
 */
public class Dsplmovingregion extends DisplayTimeGraph {
  //AffineTransform internAT;
  Point2D.Double point;
  Vector RegionMaps;
  Rectangle2D.Double bounds;
  double bufferedTime;

  /**
   * Gets the shape of this instance at the ActualTime
   * @param at The actual transformation, used to calculate the correct size.
   * @return Area Shape if ActualTime != bufferedtime then it must be caclculated again.
   * @see <a href="Dsplmovingregionsrc.html#getRenderObject">Source</a>
   */

  public Shape getRenderObject (AffineTransform at) {
    double t = RefLayer.getActualTime();
    if (t != bufferedTime)
      RenderObject = getRenderObjectAtTime(t);
    return  RenderObject;
  }

  /**
   * Creates an area out of the stored maps at the time t.
   * @param t A time double
   * @return A Shape representing this instance at time t.
   */
  private Shape getRenderObjectAtTime (double t) {
    bufferedTime = t;
    int index = getTimeIndex(t,Intervals);
    if(index<0){
      return (RenderObject=null);
    }

   RegionMap rm = (RegionMap) RegionMaps.get(index);
   Interval in = (Interval) Intervals.get(index);
   double t1 = in.getStart();
   double t2 = in.getEnd();
   double delta = (t1==t2)?0 : (t-t1)/(t2-t1);
   if(rm.numberOfPoints<3){
      return (RenderObject=null);
   }
   Vector Cycles = rm.Cycles;
   GeneralPath GP = new GeneralPath(GeneralPath.WIND_EVEN_ODD,rm.numberOfPoints);
   try{
     for(int i=0;i<Cycles.size();i++){
         Vector SingleCycle = (Vector) Cycles.get(i);
         for(int j=0;j<SingleCycle.size();j++){
             EdgeMap em = (EdgeMap) SingleCycle.get(j);
             double tmpx = em.x1+delta*(em.x2-em.x1);
	     double tmpy = em.y1+delta*(em.y2-em.y1);
	     float x = (float)ProjectionManager.getPrjX(tmpx,tmpy);
	     float y = (float)ProjectionManager.getPrjY(tmpx,tmpy);
             if(j==0)
	        GP.moveTo(x,y);
	     else
	        GP.lineTo(x,y);
         }
     }
   } catch(Exception e){
     System.err.println("wrong parameter for choosed projection");
     return (RenderObject=null);
   }
   GP.closePath();
   return (RenderObject = new Area(GP));
  }

  /**
   *
   * @param le
   * @return a single EdgeMap consisting of ax,bx,ay,by coefficiants
   * @see <a href="Dsplmovingregionsrc.html#readEdgeMap">Source</a>
   */
  private EdgeMap readEdgeMap (ListExpr le) {
    Double value[] =  {
      null, null, null, null
    };
    if (le.listLength() != 4)
      return  null;
    Double X1 = LEUtils.readNumeric(le.first());
    Double Y1 = LEUtils.readNumeric(le.second());
    Double X2 = LEUtils.readNumeric(le.third());
    Double Y2 = LEUtils.readNumeric(le.fourth());
    if(X1==null || X2==null || Y1==null || Y2 == null)
      return null;
    return new EdgeMap(X1.doubleValue(),Y1.doubleValue(),X2.doubleValue(),Y2.doubleValue());
  }


  /** The method readUnit writes all cycles contained in a single Unit
    * to a Vector and returns it.
    **/
  private RegionMap readRegionMap(ListExpr reg) {
    Vector res = new Vector();
    int numberOfPoints = 0;
    while(!reg.isEmpty()){ // scan each face
       ListExpr Face = reg.first();
       reg = reg.rest();
       while(!Face.isEmpty()){  // scan each cycle
           ListExpr Cycle = Face.first();
	   Face = Face.rest();
	   Vector CycleV = new Vector(Cycle.listLength()+1);
	   while(!Cycle.isEmpty()){
               ListExpr Edge = Cycle.first();
	       Cycle = Cycle.rest();
               EdgeMap em = readEdgeMap(Edge);
	       CycleV.add(em);
	       numberOfPoints++;
	   }
	  numberOfPoints++; // close cycle
          res.add(CycleV);
       }
    }
    return new RegionMap(res,numberOfPoints);
  }

  /**
   * Scans the representation of a movingregion datatype
   * @param v A list of start and end intervals with regionmap value
   * @see sj.lang.ListExpr
   */
  public void ScanValue (ListExpr value) {
    err = true;
    //System.out.println(value.writeListExprToString());
    // 	 areas = new Area();
    RegionMaps = new Vector(value.listLength()+1);
    Intervals = new Vector(value.listLength()+1);
    while (!value.isEmpty()) {      // scan each unit
      ListExpr unit = value.first();
      value = value.rest();

      int L = unit.listLength();
      if (L != 5 & L!=2)
        return;

      Interval in = null;
      RegionMap rm = null;

      if(L==5){
         System.out.println("Warning: use a deprecated version of"+
	                    " external representation of a moving region!");
         in = LEUtils.readInterval(ListExpr.fourElemList(unit.first(),
                                   unit.second(), unit.third(), unit.fourth()));
         rm = readRegionMap(unit.fifth());
      }
      if(L==2){
          in = LEUtils.readInterval(unit.first());
          rm = readRegionMap(unit.second());
      }

      if ((in == null) || (rm == null))
        return;
      Intervals.add(in);
      RegionMaps.add(rm);
    }
    err = false;
  }

  /**
   * Init. the Dsplmovingregion instance and calculate the overall bounds and Timebounds
   * @param type The symbol movingregion
   * @param value A list of start and end intervals with regionmap value
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Dsplmovingregion Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GTA(mregion))"));
      return;
    }
    else
      qr.addEntry(this);
    //ListIterator li=iv.listIterator();
    bounds = null;
    TimeBounds = null;
    // compute bounding box for time and space
    for (int j = 0; j < Intervals.size(); j++) {
      Interval in = (Interval)Intervals.elementAt(j);
      Interval i = new Interval(in.getStart()+0.0001,in.getEnd()-0.0001,true, true);
      Rectangle2D.Double r = (Rectangle2D.Double)getRenderObjectAtTime(i.getStart()).getBounds2D();
      r = (Rectangle2D.Double)r.createUnion(getRenderObjectAtTime(i.getEnd()).getBounds2D());
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
   * @return The overall boundingbox of the movingregion
   * @see <a href="Dsplmovingregionsrc.html#getBounds">Source</a>
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }


   class RegionMap{
      Vector Cycles;
      int numberOfPoints;
      RegionMap(Vector V , int points){
          Cycles = V;
	  numberOfPoints = points;
      }
   }


  class EdgeMap {
    double x1, y1, x2, y2;

    /**
     * Constructor
     */
    public EdgeMap (double x1, double y1, double x2, double y2) {
      this.x1 = x1;
      this.y1 = y1;
      this.x2 = x2;
      this.y2 = y2;
    }
  }
}



