

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
   * @see <a href="Dsplmovingregionsrc.html#getRenderObjectAtTime">Source</a>
   */
  private Shape getRenderObjectAtTime (double t) {
    bufferedTime = t;
    RegionMap rm = (RegionMap)getMapAt(t, Intervals, RegionMaps);
    if (rm == null)
      return  null;
    Area area = new Area();
    for (int i = 0; i < rm.Regions.size(); i++) {
      //create Path from EdgeMaps
      GeneralPath path = null;
      ListIterator li = ((Vector)rm.Regions.elementAt(i)).listIterator();
      //System.out.println(rm.Regions.elementAt(i));
      while (li.hasNext()) {
        EdgeMap em = (EdgeMap)li.next();
        if (path == null) {
          path = new GeneralPath();
          path.moveTo((float)(em.ax*t + em.bx), (float)(em.ay*t + em.by));
        }
        else
          path.lineTo((float)(em.ax*t + em.bx), (float)(em.ay*t + em.by));
      }
      //create area a from Path
      Area a = new Area(path);
      //subtract holes from a
      ListIterator li2 = ((Vector)rm.Holes.elementAt(i)).listIterator();
      while (li2.hasNext()) {
        Vector hole = (Vector)li2.next();
        path = null;
        li = hole.listIterator();
        while (li.hasNext()) {
          EdgeMap em = (EdgeMap)li.next();
          if (path == null) {
            path = new GeneralPath();
            path.moveTo((float)(em.ax*t + em.bx), (float)(em.ay*t + em.by));
          }
          else
            path.lineTo((float)(em.ax*t + em.bx), (float)(em.ay*t + em.by));
        }
        Area b = new Area(path);
        a.subtract(b);
      }
      //add a to area
      area.add(a);
    }           //end for
    //	ListIterator li=rm.holes.listIterator();
    //	while (li.hasNext(){
    //		RegionMap rm=(RegionMap) li.next();
    return  area;
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
    for (int i = 0; i < 4; i++) {
      value[i] = LEUtils.readNumeric(le.first());
      if (value[i] == null)
        return  null;
      le = le.rest();
    }
    return  new EdgeMap(value[0].doubleValue(), value[2].doubleValue(), value[1].doubleValue(),
        value[3].doubleValue());
  }

  /**
   * Reads the all the EdgeMaps out of a ListExpr.
   * @param maps The ListExpr out of which the EdgeMaps are read
   * @return A Vector of all EdgeMaps
   * @see <a href="Dsplmovingregionsrc.html#readEdgeMaps">Source</a>
   */
  private Vector readEdgeMaps (ListExpr maps) {
    Vector v = new Vector(5, 1);
    while (!maps.isEmpty()) {
      EdgeMap em = readEdgeMap(maps.first());
      if (em == null)
        return  null;
      v.add(em);
      maps = maps.rest();
    }
    return  v;
  }

  /**
   * Reads the RegionMap i.e. all the regions and hole coefficients
   * @param reg The ListExpr out of which the RegionMap is read
   * @return The created regionMap
   * @see <a href="Dsplmovingregionsrc.html#readRegionMap">Source</a>
   */
  private RegionMap readRegionMap (ListExpr reg) {
    if (reg.isEmpty())
      return  null;
    Vector regions = new Vector(5, 1);
    Vector HoleLists = new Vector(5, 1);
    while (!reg.isEmpty()) {
      //first entry is region the rest are holes
      ListExpr face = reg.first();              // face
      Vector em = readEdgeMaps(face.first());
      if (em == null)
        return  null;
      Vector tempHoles = new Vector(5, 1);
      face = face.rest();
      while (!face.isEmpty()) {
        Vector e = readEdgeMaps(face.first());
        if (e == null)
          return  null;
        tempHoles.add(e);
        face = face.rest();
      }
      regions.add(em);
      HoleLists.add(tempHoles);
      reg = reg.rest();
    }
    return  new RegionMap(regions, HoleLists);
  }

  /**
   * Scans the representation of a movingregion datatype
   * @param v A list of start and end intervals with regionmap value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingregionsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr value) {
    err = true;
    //System.out.println(value.writeListExprToString());
    // 	 areas = new Area();
    while (!value.isEmpty()) {                  // value while
      ListExpr face = value.first();
      int L = face.listLength();
      if (L != 5 & L!=2)
        return;
      Interval in = null;
      RegionMap rm = null;

      if(L==5){
         System.out.println("Warning: use a deprecated version of external representation of a moving region!");
         in = LEUtils.readInterval(ListExpr.fourElemList(face.first(),
                                   face.second(), face.third(), face.fourth()));
         rm = readRegionMap(face.fifth());
      }
      if(L==2){
          in = LEUtils.readInterval(face.first());
          rm = readRegionMap(face.second());
      }

      if ((in == null) || (rm == null))
        return;
      Intervals.add(in);
      RegionMaps.add(rm);
      value = value.rest();
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
   * @see <a href="Dsplmovingregionsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    Intervals = new Vector(5, 1);
    RegionMaps = new Vector(5, 1);
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
    for (int j = 0; j < Intervals.size(); j++) {
      Interval in = (Interval)Intervals.elementAt(j);
      Interval i = new Interval(in.getStart() + 0.0001, in.getEnd() - 0.0001, 
          true, true);
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


  class RegionMap {
    Vector Regions;             //list of list of EdgeMap
    Vector Holes;               //list of Regions s.o.

    /**
     * Constructor
     * @param     Vector v1 A list of regions
     * @param     Vector v2 A list of holes
     */
    public RegionMap (Vector v1, Vector v2) {
      Regions = v1;
      Holes = v2;
    }
  }

  class EdgeMap {
    double ax, bx, ay, by;

    /**
     * Constructor
     * @param     double x1
     * @param     double x2
     * @param     double y1
     * @param     double y2
     */
    public EdgeMap (double x1, double x2, double y1, double y2) {
      ax = x1;
      bx = x2;
      ay = y1;
      by = y2;
    }
  }
}



