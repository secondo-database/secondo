
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


/**
 * A displayclass for the instant-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplmovingreal extends Dsplinstant {
  Vector Intervals = new Vector(10, 5);
  Vector MRealMaps = new Vector(10, 5);
  double PixelTime;
  double min=Double.POSITIVE_INFINITY ;
  double max=Double.NEGATIVE_INFINITY ;
  final static int PSIZE=300;
  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplmovingrealsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixTime) {
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
      //System.out.println(new String(start+" "+end));
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
      	 double mv;
         if (mr.f)
           mv = Math.sqrt(mr.a*actTime*actTime + mr.b*actTime + mr.c); 
         else 
           mv = mr.a*actTime*actTime + mr.b*actTime + mr.c;
       //  System.out.println(mv);
         max=Math.max(max,mv);
         min=Math.min(min,mv);
       }	 
      //Dimension d = jc.getPreferredSize();
      jc.setBounds(start, 0, end - start, PSIZE);
      //   System.out.println(max +" "+min);

      jp.add(jc);
    }
      //jc.setBorder(new MatteBorder(2, (in.isLeftclosed()) ? 2 : 0, 2, (in.isRightclosed()) ?
      //    2 : 0, Color.black));
    jp.setPreferredSize(new Dimension((int)((TimeBounds.getEnd() - TimeBounds.getStart())*PixelTime), 
          PSIZE));
    return  jp;
  }

  /**
   * Scans the representation of a movingreal datatype 
   * @param v A list of time-intervals with the parameter for a quadratic or squareroot formula
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingrealsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    ////System.out.println(v.writeListExprToString());
    if (v.isEmpty())
      return;
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      //System.out.println(le.writeListExprToString());
      if (le.listLength() != 8)
        return;
      Interval in = LEUtils.readInterval(ListExpr.fourElemList(le.first(), 
          le.second(), le.third(), le.fourth()));
      le = le.rest().rest().rest().rest();
      //      System.out.println(aunit.writeListExprToString());
      MRealMap rm = readMRealMap(ListExpr.fourElemList(le.first(), le.second(), 
          le.third(), le.fourth()));
      if ((in == null) || (rm == null))
        return;
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
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(MReal))"));
      return;
    } 
    else 
      qr.addEntry(this);
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
      for (int x=start;x<=end;x++) {
      	 double actTime = (double)x/PixelTime+TimeBounds.getStart();
      	 double mv;
         if (mr.f)
           mv = Math.sqrt(mr.a*actTime*actTime + mr.b*actTime + mr.c); 
         else 
           mv = mr.a*actTime*actTime + mr.b*actTime + mr.c;
	 int ypos=(int)(mv*y1+y2);
	 g.drawLine(xpos,ypos,xpos,ypos+1);
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
      MRealMap mr = (MRealMap)MRealMaps.elementAt(ValueIndex);
      double mv;
      if (mr.f)
        mv = Math.sqrt(mr.a*actTime*actTime + mr.b*actTime + mr.c); 
      else 
        mv = mr.a*actTime*actTime + mr.b*actTime + mr.c;
      //int start=(int)((in.getStart()-TimeBounds.getStart())*PixelTime);
      return  LEUtils.convertTimeToString(actTime) + "=" + mv;
      //return new String(in.getStart() + " " +PixelTime +" "+ evt.getX()) ;
    }
  }
}



