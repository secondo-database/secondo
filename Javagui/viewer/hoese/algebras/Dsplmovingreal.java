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

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import  java.util.*;
import  javax.swing.*;
import  java.awt.*;
import  javax.swing.border.*;
import  java.awt.event.*;
import  java.awt.geom.*;
import gui.Environment;
import tools.Reporter;


/**
 * A displayclass for the instant-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplmovingreal extends DsplGeneric implements 
      Timed,Function,ExternDisplay,LabelAttribute, RenderAttribute{

  Interval TimeBounds;
  Vector Intervals = new Vector(10, 5);
  Vector MRealMaps = new Vector(10, 5);
  double PixelTime;
  double min=Double.POSITIVE_INFINITY ;
  double max=Double.NEGATIVE_INFINITY ;
  final static int PSIZE=300;
  boolean err;
  boolean defined;
  /** format specification for label **/ static java.text.DecimalFormat format = new java.text.DecimalFormat("#.####");
  


  public boolean isExternDisplayed(){
      return(functionframe.isVisible() && this.equals(functionframe.getSource()));
  }

  public void  displayExtern(){
      if(!defined){
          Reporter.showInfo("not defined");
          return;
      }
      if(TimeBounds!=null){
         functionframe.setSource(this);
         functionframe.setVisible(true);
         functionframe.toFront();
      } else{
         Reporter.showInfo("The moving real is empty");
      }
  }


  /** This method returns the "bounding box" 
    * of the definition time
    */
  public Interval getTimeBounds(){
     return  TimeBounds;
  }


  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per time unit 
   * @return A JPanel component with the renderer
   */
  public JPanel getTimeRenderer (double PixTime) {
    if(err) // error
       return null;
    if(TimeBounds==null) // empty
       return null;

    // Create a MRealLabel for each unit and add it to
    // the panel 
    PixelTime = PixTime;
    JPanel jp = new JPanel(null);
    if (Intervals == null)
      return  null;

    ListIterator li = Intervals.listIterator();
    int cnt = 0;
    while (li.hasNext()) {
      Interval in = (Interval)li.next();
      int start = (int)((in.getStart() - TimeBounds.getStart())*PixelTime);
      int end = (int)((in.getEnd() - TimeBounds.getStart())*PixelTime);
      String bs = MRealMaps.elementAt(cnt).toString();
      MRealLabel jc = new MRealLabel(bs);
      jc.ValueIndex = cnt++;
      jc.setFont(new Font("Dialog", Font.PLAIN, 12));
      jc.setOpaque(true);
      jc.setForeground(Color.black);
      //jc.setBackground(Color.yellow);
      jc.setToolTipText("...");
      MRealMap mr = (MRealMap)MRealMaps.elementAt(jc.ValueIndex);
      for (int x=start;x<=end;x++) {
      	 double actTime = (double)x/PixelTime+TimeBounds.getStart();
         Double Mv = getValueAt(actTime,jc.ValueIndex);      
         if(Mv==null){
            Reporter.writeError("Cannot determine the value at"+actTime);
         }else{
            double mv=Mv.doubleValue();
        }
      }
      // set posittion and size of this label      
      jc.setBounds(start, 0, end - start, PSIZE);
      // add the label to the panel
      jp.add(jc);
    }
      //jc.setBorder(new MatteBorder(2, (in.isLeftclosed()) ? 2 : 0, 2, (in.isRightclosed()) ?
      //    2 : 0, Color.black));
    jp.setPreferredSize(new Dimension((int)((TimeBounds.getEnd() - TimeBounds.getStart())*PixelTime),
          PSIZE));
    return  jp;
  
  }

  /**  Computes the value of this real for a given instant.
    *  The instant is just given as a double. 
    *  If the moving real is not defined at the given instant null is returned.
    **/
  public    Double getValueAt(double time){
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    return getValueAt(time,index); 
  }


  /** Computes the value for a given index 
    * There is no check wether the given time is contained in
    * the interval determined by index.
    **/
  private Double getValueAt(double time,int index){ 
       // check for correct index
       if(index<0 || index >= Intervals.size())
          return null;
       Interval CurrentInterval = (Interval) Intervals.get(index);
       MRealMap CurrentMap = (MRealMap) MRealMaps.get(index);
       // in this version, the interval is ignored, in the
       // new version implemented by Victor we have to subtract the
       // starting point of the interval from time first
       double start=CurrentInterval.getStart();
       time -= start;
       double polyvalue = CurrentMap.a*time*time + CurrentMap.b*time + CurrentMap.c;
       if(!CurrentMap.f){
         return new Double(polyvalue);
       }else{
         if(polyvalue<0){
            Reporter.writeError("Wrong MRealMap detected !!!"+
                                "attempt to compute the sqareroot of a negative number");
            return null;
         }else{
            return new Double(Math.sqrt(polyvalue));
         }
       }
  }

  /** computes the minimum and the maximum value **/
  private void computeMinMax(){
     min = 0.0;
     max = 0.0;
     if(Intervals==null){
         return; 
     }
     boolean first = true;
     int size = Intervals.size();
     for(int i=0;i<size;i++){
         Interval interval = (Interval) Intervals.get(i);
         MRealMap map = (MRealMap) MRealMaps.get(i);
         if(updateMinMax(interval,map,first)){
              first = false;
         } 
     }
  }

  /** computes the extremums of the given interval-map combination
    * and updates minvalue and maxvalue if required.
    * @param interval the interval to check
    * @param map      the map hlding for this interval
    * @param first    flag indicating that min and max are undefined up to now
    * @return true if the values are defined after calling this functiom 
    **/
  private boolean updateMinMax(Interval interval, MRealMap map, boolean first){
    double a = map.a;
    double b = map.b;
    double c = map.c;
    double start = interval.getStart();
    double end = interval.getEnd();  

 
    // check the intervals start
    double y;
    if(!map.f || c>=0){ // ensure a defined value
       y = map.f?Math.sqrt(c):c;
       if(first){
         first=false;
         min=y;
         max=y;
       }else{
         if(y<min){
           min=y;
         }
         if(y>max){
           max=y;
         }
       }
    } 

    // check the end of the interval
    double t = end-start;
    double ty = a*t*t + b*t + c;
    if(!map.f || ty>=0){
        y = map.f?Math.sqrt(ty):ty; 
        if(first){
           first=false;
           min=y;
           max=y;
        }else{
           if(y<min){
             min=y;
           }
           if(y>max){
             max=y;
           }
        }
    } 

   // check a possible angular point
   if(a!=0.0){
     t = start - b / (2*a); // the moved angular point
     // check whether t is inside interval
     if(t>0 && t < (end-start)){
				 ty = a*t*t + b*t + c;
				 if(!map.f || ty>=0){
						y = map.f?Math.sqrt(ty):ty; 
						if(first){
							 first=false;
							 min=y;
							 max=y;
						}else{
							 if(y<min){
								 min=y;
							 }
							 if(y>max){
								 max=y;
							 }
						}
				 }  
      }
  }
  return !first;

}




  /** Returns the interval of this moving real **/
  public Interval getInterval(){
    return TimeBounds;
  }


  /**
   * Scans the representation of a movingreal datatype
   * @param v A list of time-intervals with the parameter for a quadratic or squareroot formula
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingrealsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if(isUndefined(v)){
       err=false;
       defined=false;
       return;
    }
    defined=true;
    if (v.isEmpty()){
      err=false; // allow empty mreals
      return;
    }
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      Interval in=null;
      ListExpr map=null;
      int len = le.listLength();
      if(len!=2 && len !=8)
         return;
      if (len == 8){
         Reporter.writeWarning("Warning: deprecated list representation for moving real");
         in = LEUtils.readInterval(ListExpr.fourElemList(le.first(),
                       le.second(), le.third(), le.fourth()));
         map = le.rest().rest().rest().rest();
      }
      if(len == 2){
         in = LEUtils.readInterval(le.first());
         map = le.second();
      }
      MRealMap rm = readMRealMap(map);
      if ((in == null) || (rm == null)){
          return;
      }
      Intervals.add(in);
      MRealMaps.add(rm);
      v = v.rest();
    }
    err = false;
  }


  /**
   * This method reads the parameter for a quadratic or squareroot formula into a MRealMap-instance
   * @param le A four element list
   * @return A MRealMap-instance with the formula parameter
   */
  private MRealMap readMRealMap (ListExpr le) {
    Double value[] =  {
      null, null, null
    };
    if (le.listLength() != 4)
      return  null;
    for (int i = 0; i < 3; i++) {
      value[i] = LEUtils.readNumeric(le.first());
      if (value[i] == null)
        return  null;
      le = le.rest();
    }
    if (le.first().atomType() != ListExpr.BOOL_ATOM)
      return  null;
    return  new MRealMap(value[0].doubleValue(), value[1].doubleValue(), value[2].doubleValue(), 
        le.first().boolValue());
  }

  /**
   * Init. the Dsplmovingreal instance.
   * @param type The symbol movingreal
   * @param value A list of time-intervals with the parameter for a quadratic or squareroot formula
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingrealsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    err=true;
    ScanValue(value);
    computeMinMax();
    if (err) {
      defined = false;
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(MReal))"));
      return;
    } 
    else 
      qr.addEntry(this);
    // compute the bounding box of all intervals
    TimeBounds = null;
    for (int i = 0; i < Intervals.size(); i++) {
      Interval in = (Interval)Intervals.elementAt(i);
      if (TimeBounds == null) {
        TimeBounds = in;
      } 
      else {
        TimeBounds = TimeBounds.union(in);
      }
    }
  }

  /** The text representation of this object 
   * @see <a href="Dsplmovingrealsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ": TA(MReal) ";
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplmovingrealsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    return Intervals;
    } 

   /* Implementation of LabelAttribute */
   public String getLabel(double time){
     if(!defined){
        return "undefined";
     }
     Double d = getValueAt(time);
     if(d==null){
        return "undefined";  
     }
     return format.format(d.doubleValue());
  }


  /* Implementation of the RenderAttribute interface **/
  public boolean mayBeDefined(){
     return defined;
  }
  public double getMinRenderValue(){
     return min;
  }
  public double getMaxRenderValue(){
     return max;
  }
  public boolean isDefined(double time){
     if(!defined){
        return false;
     }
     Double d = getValueAt(time);
     return d!=null;
  }
  public double getRenderValue(double time){
      Double d = getValueAt(time);
      if(d==null){
        return (max+min)/2;
      } else{
        return d.doubleValue();
      }
  }




  /** The class which holds the formula parameter for an interval */
  class MRealMap {
    double a, b, c;
    boolean f;

    /**
     * Constructor
     * @param     double x1 Quadratic koefficient
     * @param     double x2 Linear koefficient
     * @param     double x3 Constant koefficient
     * @param     boolean y True if Squareroot
     */
    public MRealMap (double x1, double x2, double x3, boolean y) {
      a = x1;
      b = x2;
      c = x3;
      f = y;
    }

    /**
     * 
     * @return Textrepresentation of a MRealMap
     */
    public String toString () {
      if (!f)
        return  a + "t^2+" + b + "t+" + c; 
      else 
        return  "SQRT(" + a + "t^2+" + b + "t+" + c + ")";
    }
  }

  /** Special class to draw the MRealMap function */
  class MRealLabel extends JLabel {
    int ValueIndex;
    /**
     * constructor
     * @param     String s Label of this function
     */
    public MRealLabel (String s) {
      super(s);
    }
    /**
     * Paints the function
     * @param Graphics g The context to draw in
     */
    public void paintComponent(Graphics g){
      super.paintComponent(g);	     
      g.setColor(Color.black);
      Interval in = (Interval)Intervals.elementAt(ValueIndex);
      MRealMap mr = (MRealMap)MRealMaps.elementAt(ValueIndex);
      int start = (int)((in.getStart() - TimeBounds.getStart())*PixelTime);
      int end = (int)((in.getEnd() - TimeBounds.getStart())*PixelTime);
      double y1=-PSIZE/(max-min);
      double y2=PSIZE*max/(max-min);
      int xpos=0;
      int lastx=0,lasty=0;
      for (int x=start;x<=end;x++) {
      	 double actTime = (double)x/PixelTime+TimeBounds.getStart();
         Double Mv = getValueAt(actTime,ValueIndex);
         if(Mv==null){
           Reporter.writeError("Error in computing the real value for instant "+actTime);
         }else{
      	    double mv=Mv.doubleValue();
	    int ypos=(int)(mv*y1+y2);
            if(x==start){ // ensure to draw also a single point
  	        g.drawLine(xpos,ypos,xpos,ypos+1);
            } else{
                g.drawLine(lastx,lasty,xpos,ypos);
            }
          lastx = xpos;
          lasty = ypos;
         }
	 xpos++;
	}
      }


    /**
     * You can point with the mouse to get the value at a time
     * @param evt MouseEvent
     * @return Function value as string
     */
    public String getToolTipText (MouseEvent evt) {
      Interval in = (Interval)Intervals.elementAt(ValueIndex);
      double actTime = in.getStart() + evt.getX()/PixelTime;
      Double Mv = getValueAt(actTime,ValueIndex);      
      if(Mv!=null){
         double mv=Mv.doubleValue();
         return  LEUtils.convertTimeToString(actTime) + "=" + mv;
      }
      else{
        return "Error in computing value at instant "+LEUtils.convertTimeToString(actTime);
      }
    }
  }

private static  FunctionFrame functionframe = new FunctionFrame();



}



