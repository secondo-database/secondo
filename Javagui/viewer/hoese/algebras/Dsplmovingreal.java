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


/**
 * A displayclass for the instant-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplmovingreal extends DsplGeneric implements Timed,Function,ExternDisplay{

  Interval TimeBounds;
  Vector Intervals = new Vector(10, 5);
  Vector MRealMaps = new Vector(10, 5);
  double PixelTime;
  double min=Double.POSITIVE_INFINITY ;
  double max=Double.NEGATIVE_INFINITY ;
  final static int PSIZE=300;
  boolean err;


  public boolean isExternDisplayed(){
      return(functionframe.isVisible() && this.equals(functionframe.myreal));
  }

  public void  displayExtern(){
      functionframe.setSource(this);
      functionframe.setVisible(true);
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
    if(err)
       return null;

    // Create a MRealLabel for each unit and add it to
    // the panel 
    min=Double.POSITIVE_INFINITY ;
    max=Double.NEGATIVE_INFINITY ;
    PixelTime = PixTime;
    JPanel jp = new JPanel(null);
    if (Intervals == null)
      return  null;
    ListIterator li = Intervals.listIterator();
    int cnt = 0;
    while (li.hasNext()) {
      Interval in = (Interval)li.next();
      /*
       Compute the used x-space in pixels for the current unit
      */
      int start = (int)((in.getStart() - TimeBounds.getStart())*PixelTime);
      int end = (int)((in.getEnd() - TimeBounds.getStart())*PixelTime);
      
      /* Create a Label for this unit */      
      String bs = MRealMaps.elementAt(cnt).toString();
      MRealLabel jc = new MRealLabel(bs);
      jc.ValueIndex = cnt++;
      jc.setFont(new Font("Dialog", Font.PLAIN, 12));
      jc.setOpaque(true);
      jc.setForeground(Color.black);
      //jc.setBackground(Color.yellow);
      jc.setToolTipText("...");
      /*
       * compute the range of the values for the current unit
       */
      MRealMap mr = (MRealMap)MRealMaps.elementAt(jc.ValueIndex);
      for (int x=start;x<=end;x++) {
      	 double actTime = (double)x/PixelTime+TimeBounds.getStart();
         Double Mv = getValueAt(actTime,jc.ValueIndex);      
         if(Mv==null){
            System.err.println("Cannot determine the value at"+actTime);
         }else{
            double mv=Mv.doubleValue();
            max=Math.max(max,mv);
            min=Math.min(min,mv);
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


 /** This functions searchs in the vector of intervals for the 
   * given instant. If found the index within the Vector is returned,
   * otherwise -1.
   */
 private int getIndexFrom(double time){
    for(int i=0;i<Intervals.size();i++)
       if(((Interval)Intervals.get(i)).isDefinedAt(time))
         return i;
    return -1;
 }


  /**  Computes the value of this real for a given instant.
    *  The instant is just given as a double. 
    *  If the moving real is not defined at the given instant null is returned.
    **/
  public    Double getValueAt(double time){
    int index = getIndexFrom(time);
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
            System.err.println("Wrong MRealMap detected !!!");
            System.err.println("attempt to compute the sqareroot of a negative number");
            return null;
         }else{
            return new Double(Math.sqrt(polyvalue));
         }
       }
  }


  /**
   * Scans the representation of a movingreal datatype
   * @param v A list of time-intervals with the parameter for a quadratic or squareroot formula
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingrealsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if (v.isEmpty())
      return;
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      Interval in=null;
      ListExpr map=null;
      int len = le.listLength();
      if(len!=2 && len !=8)
         return;
      if (len == 8){
         if(Environment.DEBUG_MODE)
            System.err.println("Warning: deprecated list represenation for moving real");
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
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
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
           System.err.println("Error in computing the real value for instant "+actTime);
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

private static class FunctionFrame extends JFrame{
   public FunctionFrame(){
      super();
      setSize(640,480);
      getContentPane().setLayout(new BorderLayout());
      functionSP = new JScrollPane(functionpanel);
      getContentPane().add(functionSP,BorderLayout.CENTER);
      JPanel P1 = new JPanel(new GridLayout(1,3));
      P1.add(TimeLabel);
      P1.add(ValueLabel);
      getContentPane().add(P1,BorderLayout.NORTH);
      functionpanel.addMouseMotionListener(new MouseMotionAdapter(){
           public void mouseMoved(MouseEvent e){
               if(functionpanel.getOrig(e.getX(),e.getY(),P,MP)){
                   TimeLabel.setText("x= "+DateTime.getString(P.x));
                   ValueLabel.setText("y= "+P.y);
               } else{ 
                  TimeLabel.setText("");
                  ValueLabel.setText("");
               }
           }
      });
      functionpanel.showCross(true);
      functionpanel.showY(true);
      closeBtn.addActionListener(new ActionListener() {
             public void actionPerformed(ActionEvent evt){
                  setVisible(false);
             }
      });
      
      getContentPane().add(closeBtn,BorderLayout.SOUTH);
   }


   void setSource(Dsplmovingreal mr){
        myreal = mr;
        functionpanel.setFunction(mr);
        functionpanel.setInterval(mr.TimeBounds.getStart(),mr.TimeBounds.getEnd());
   }  

 
   FunctionPanel functionpanel = new FunctionPanel();
   Dsplmovingreal myreal;
   JLabel TimeLabel = new JLabel(" ");
   JLabel ValueLabel = new JLabel(" ");
//   JLabel YPosLabel = new JLabel("");
   Point2D.Double P = new Point2D.Double();
   Point2D.Double MP = new Point2D.Double();
   JButton closeBtn = new JButton("close");
   JScrollPane functionSP; 
} 


}



