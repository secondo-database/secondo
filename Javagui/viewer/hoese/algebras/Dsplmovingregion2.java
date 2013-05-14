//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  java.math.*;
import  viewer.*;
import viewer.hoese.*;
import  sj.lang.ListExpr;
import  java.util.*;
import tools.Reporter;

/**
 * A displayclass for the movingregion2-types mregion2 and uregion2
 */
public class Dsplmovingregion2 extends DisplayTimeGraph{
  //AffineTransform internAT;
  Point2D.Double point;
  Vector RegionMaps;
  Rectangle2D.Double bounds;
  double bufferedTime;
  int bufferedIndex=-1;

  private Shape shp;

  /** defined flag */
  boolean defined;

  public int numberOfShapes(){
     return 1;
  }

  /**
   * Gets the shape of this instance at the ActualTime
   * @param at The actual transformation, used to calculate the correct size.
   * @return Area Shape if ActualTime != bufferedtime then it must be caclculated again.
   * @see <a href="Dsplmovingregion2src.html#getRenderObject">Source</a>
   */

  public Shape getRenderObject(int num,AffineTransform at) {
    if(num!=0){
      return null;
    }
    if(!defined){
      return null;
    }
    double t = RefLayer.getActualTime();
    if((t != bufferedTime) || (bufferedIndex <0))
      shp = getRenderObjectAtTime(t);
    return  shp;
  }

  /**
   * Creates an area out of the stored maps at the time t.
   * @param t A time double
   * @return A Shape representing this instance at time t.
   */
  private Shape getRenderObjectAtTime (double t) {
    if(!defined){
       return null;
    }
    bufferedTime = t;
    int index;
    if(bufferedIndex>=0){
      Interval lastInterval = (Interval) Intervals.get(bufferedIndex);
      if(lastInterval.isDefinedAt(t))
         index = bufferedIndex;
      else
         index = IntervalSearch.getTimeIndex(t,Intervals); 
    }else{
       index = IntervalSearch.getTimeIndex(t,Intervals);
    }
    bufferedIndex = index;
    if(index<0){
      return (shp=null);
    }

   RegionMap rm = (RegionMap) RegionMaps.get(index);
   Interval in = (Interval) Intervals.get(index);
   double t1 = in.getStart();
   double t2 = in.getEnd();
   double delta = (t1==t2)?0 : (t-t1)/(t2-t1);
   if(rm.numberOfPoints<3){
      return (shp=null);
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
       if(!ProjectionManager.project(tmpx,tmpy,aPoint)){
          Reporter.writeError("wrong parameter for choosed projection");
          return (shp=null);
       } 
	     float x = (float)aPoint.x;
	     float y = (float)aPoint.y;
       if(j==0)
	        GP.moveTo(x,y);
	     else
	        GP.lineTo(x,y);
         }
     }
   } catch(Exception e){
     Reporter.writeError("wrong parameter for choosed projection");
     return (shp=null);
   }
   GP.closePath();
   return (shp = new Area(GP));
  }

  /**
   *
   * @param le
   * @return a single EdgeMap consisting of ax,bx,ay,by coefficiants
   * @see <a href="Dsplmovingregion2src.html#readInterval">Source</a>
   */
  protected Interval readInterval (ListExpr le) {
    if (le.listLength() != 5)
      return  null;

      Interval in = LEUtils.readInterval(ListExpr.fourElemList(le.first(),le.second(), le.third(), le.fourth()));
      le = le.fifth();

      if(!le.isEmpty())
      {
	BigDecimal sBD = new BigDecimal(0);
	BigDecimal eBD = new BigDecimal(0);
	String sprec = le.first().textValue();
	String eprec = le.second().textValue();

	try {
	  if (sprec.contains("/")) 
	  {
	    String[] startprec = sprec.split("/");
	    sBD = new BigDecimal( startprec[0] ); 
	    sBD = sBD.divide( new BigDecimal( startprec[1] ), 1024, RoundingMode.HALF_UP); 
	  }
	  else
	    sBD = new BigDecimal( sprec );

	  if (eprec.contains("/")) 
	  {
	    String[] endprec = eprec.split("/");
	    eBD = new BigDecimal( endprec[0]); 
	    eBD = eBD.divide( new BigDecimal( endprec[1]), 1024, RoundingMode.HALF_UP); 
	  }
	  else
	    eBD = new BigDecimal( eprec );

	  sBD = sBD.add( new BigDecimal( in.getStart() )); 
	  eBD = eBD.add( new BigDecimal( in.getEnd() ));
	}
	catch(Exception e) {
	  Reporter.writeError("Error in Dividing!");
	  return null;
	}

	boolean sb = in.isLeftclosed();
	boolean eb = in.isRightclosed();
	in = new Interval(sBD.doubleValue(), eBD.doubleValue(), sb, eb);
      }

      return in;
  }


  /**
   *
   * @param le
   * @return a single EdgeMap consisting of ax,bx,ay,by coefficiants
   * @see <a href="Dsplmovingregion2src.html#readEdgeMap">Source</a>
   */
  protected EdgeMap readEdgeMap (ListExpr le, int s) {
    if (le.listLength() != 5)
      return  null;

    BigDecimal scaleBD = new BigDecimal(10);
    scaleBD = scaleBD.pow(s);

    BigDecimal X1 = new BigDecimal(0);
    BigDecimal Y1 = new BigDecimal(0);
    BigDecimal X2 = new BigDecimal(0);
    BigDecimal Y2 = new BigDecimal(0);

    if (le.fifth().isEmpty())
    {
      X1 = new BigDecimal( le.first().intValue());
      Y1 = new BigDecimal( le.second().intValue());
      X2 = new BigDecimal( le.third().intValue());
      Y2 = new BigDecimal( le.fourth().intValue());
    }
    else
    {
      String x1precpart = le.fifth().first().textValue();
      String y1precpart = le.fifth().second().textValue();
      String x2precpart = le.fifth().third().textValue();
      String y2precpart = le.fifth().fourth().textValue();
      try {
	if (x1precpart.contains("/")) 
	{
	  String[] x1prec = x1precpart.split("/");
          X1 = new BigDecimal( x1prec[0]);  //numerator x1
          X1 = X1.divide( new BigDecimal( x1prec[1]), 1024, RoundingMode.HALF_UP);  //numerator x1 / denominator x1
	}
	else
	  X1 = new BigDecimal( x1precpart );

	if (y1precpart.contains("/")) 
	{
	  String[] y1prec = y1precpart.split("/");
          Y1 = new BigDecimal( y1prec[0]);  //numerator y1
          Y1 = Y1.divide( new BigDecimal( y1prec[1]), 1024, RoundingMode.HALF_UP);  //numerator y1 / denominator y1
	}
	else
	  Y1 = new BigDecimal( y1precpart );

	if (x2precpart.contains("/")) 
	{
	  String[] x2prec = x2precpart.split("/");
          X2 = new BigDecimal( x2prec[0]);  //numerator x2
          X2 = X2.divide( new BigDecimal( x2prec[1]), 1024, RoundingMode.HALF_UP);  //numerator x2 / denominator x2
	}
	else
	  X2 = new BigDecimal( x2precpart );

	if (y2precpart.contains("/")) 
	{
	  String[] y2prec = y2precpart.split("/");
          Y2 = new BigDecimal( y2prec[0]);  //numerator y2
          Y2 = Y2.divide( new BigDecimal( y2prec[1]), 1024, RoundingMode.HALF_UP);  //numerator y2 / denominator y2
	}
	else
	  Y2 = new BigDecimal( y2precpart );

	X1 = X1.add( new BigDecimal( le.first().intValue()));  
        Y1 = Y1.add( new BigDecimal( le.second().intValue())); 
	X2 = X2.add( new BigDecimal( le.third().intValue()));  
        Y2 = Y2.add( new BigDecimal( le.fourth().intValue())); 
      }
      catch(Exception e) {
        Reporter.writeError("Error in Dividing!");
        return null;
      }
    }
    try {
      X1 = X1.divide(scaleBD, 1024, RoundingMode.HALF_UP);
      Y1 = Y1.divide(scaleBD, 1024, RoundingMode.HALF_UP);
      X2 = X2.divide(scaleBD, 1024, RoundingMode.HALF_UP);
      Y2 = Y2.divide(scaleBD, 1024, RoundingMode.HALF_UP);
    }
    catch(Exception e) {
      Reporter.writeError("Error in Dividing!");
      return null;
    }

    if(X1==null || X2==null || Y1==null || Y2 == null)
      return null;

    return new EdgeMap(X1.doubleValue(),Y1.doubleValue(),X2.doubleValue(),Y2.doubleValue());
  }


  /** The method readUnit writes all cycles contained in a single Unit
    * to a Vector and returns it.
    **/
  protected RegionMap readRegionMap(ListExpr reg, int s) {
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
               EdgeMap em = readEdgeMap(Edge, s);
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
    if(isUndefined(value) || value.listLength()!=2){
      defined=false;
      err=false;
      return;
    }
    defined=true;
    err = true;

    int scale = value.first().intValue();

    if (value.second().first().first().isAtom())  //uregion2
      value = value.rest();
    else   //mregion2
      value = value.second();

    int length = value.listLength();
    RegionMaps = new Vector(length+1);
    Intervals = new Vector(length+1);

    ListExpr unit;

    while (!value.isEmpty())
    {
      unit = value.first();
      value = value.rest();

      Interval in = null;
      RegionMap rm = null;

      in = readInterval(unit.first());
      rm = readRegionMap(unit.second(), scale);

      if ((in == null) || (rm == null))
        return;

      Intervals.add(in);
      RegionMaps.add(rm);

    } 
    err = false;
  }


  public void init (String name, int nameWidth, int indent,
                    ListExpr type, 
                    ListExpr value,
                    QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Dsplmovingregion2 Error in ListExpr :parsing aborted");
      qr.addEntry(new String( AttrName + ": <error>"));
      return;
    }
    if(!defined){
       qr.addEntry(AttrName+": undefined");
       return;
    }
    qr.addEntry(this);
    //ListIterator li=iv.listIterator();
    bounds = null;
    TimeBounds = null;
    // compute bounding box for time and space
    for (int j = 0; j < Intervals.size(); j++) {
      Interval in = (Interval)Intervals.elementAt(j);
      Interval i = new Interval(in.getStart()+0.0001,in.getEnd()-0.0001,true, true);
      RegionMap rm = (RegionMap) RegionMaps.get(j);
      
      Rectangle2D.Double r = rm.getBox();
      if(r!=null){
         if (bounds == null) {
           bounds = r;
           TimeBounds = in;
         } else {
           bounds = (Rectangle2D.Double)bounds.createUnion(r);
           TimeBounds = TimeBounds.union(in);
         }
      }
    }
  }

  /**
   * @return The overall boundingbox of the movingregion
   * @see <a href="Dsplmovingregion2src.html#getBounds">Source</a>
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

      Rectangle2D.Double getBox(){
        Rectangle2D.Double res = null;
        for(int i=0;i<Cycles.size();i++){
           Vector cycle = (Vector) Cycles.get(i);
           for(int j=0;j<cycle.size();j++){
              EdgeMap em = (EdgeMap) cycle.get(j);
              Rectangle2D.Double r= em.getBox();
              if(r!=null){
                 if(res==null){
                    res = r;
                 } else {
                    res = (Rectangle2D.Double)res.createUnion(r);
                 }
              }
           }
        }
        return res;
      } 
   }


  class EdgeMap {
    double x1, y1, x2, y2;


    Rectangle2D.Double getBox(){
       return new Rectangle2D.Double(Math.min(x1,x2), Math.min(y1,y2), Math.abs(x2-x1), Math.abs(y2-y1));
    }

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



