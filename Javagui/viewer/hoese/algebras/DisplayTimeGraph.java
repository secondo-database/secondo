

package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  viewer.*;
import viewer.hoese.*;
import  sj.lang.ListExpr;
import  javax.swing.border.*;
import  java.util.*;
import  javax.swing.*;


/**
 * The base class for moving 2d-Objects
 */
public class DisplayTimeGraph extends DisplayGraph
    implements Timed {
  Interval TimeBounds;
  Vector Intervals;

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="DisplayTimeGraphsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    JPanel jp = new JPanel(null);
    if (Intervals == null)
      return  null;
    ListIterator li = Intervals.listIterator();
    while (li.hasNext()) {
      Interval in = (Interval)li.next();
      int start = (int)((in.getStart() - TimeBounds.getStart())*PixelTime);
      int end = (int)((in.getEnd() - TimeBounds.getStart())*PixelTime);
      //System.out.println(new String(start + " " + end));
      JLabel jc = new JLabel();
      jc.setOpaque(true);
      jc.setBackground(Color.yellow);
      jc.setPreferredSize(new Dimension(1, 10));
      jc.setBorder(new MatteBorder(2, (in.isLeftclosed()) ? 2 : 0, 2, (in.isRightclosed()) ?
          2 : 0, Color.black));
      Dimension d = jc.getPreferredSize();
      jc.setBounds(start, (int)d.getHeight()*0 + 15, end - start, (int)d.getHeight());
      jc.setToolTipText(LEUtils.convertTimeToString(in.getStart()) + "..." +
          LEUtils.convertTimeToString(in.getEnd()));
      jp.setPreferredSize(new Dimension((int)((TimeBounds.getEnd() - TimeBounds.getStart())*PixelTime),
          25));
      jp.add(jc);
    }
    return  jp;
  }

  /** A method of the Timed-Interface
   *
   * @return the global time boundaries [min..max] this instance is defined at
   * @see <a href="DisplayTimeGraphsrc.html#getTimebounds">Source</a>
   */
  public Interval getTimeBounds () {
    return  TimeBounds;
  }

  /**
   * Moving objects have 0..n Intervals and calculation unit
   * @param time A double representing a time
   * @param iv The Time intervals to check
   * @param maps The maps of this instance.
   * @return A unit of this instance, that is defined at time, or null if not defined
   * @see <a href="DisplayTimeGraphsrc.html#getMapAt">Source</a>
   */
  public Object getMapAt (double time, Vector iv, Vector maps) {
    for (int i = 0; i < iv.size(); i++){
      Interval interval = (Interval) iv.get(i);
      if (interval.isDefinedAt(time)){
         if(time<interval.getStart() | time>interval.getEnd())
	    System.out.println("wrong interval found");
         return  maps.elementAt(i);
      }
     }
    return  null;
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at
   * @see <a href="DisplayTimeGraphsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    return Intervals;
    }


  /** returns the first index in Intervals containing t
    * if not an intervals containing t exists -1 is returned
    */
  protected int getTimeIndex(double t,Vector Intervals){
     for(int i=0;i<Intervals.size();i++)
        if( ((Interval) Intervals.get(i)).isDefinedAt(t))
	    return i;
     return -1;
  }



}



