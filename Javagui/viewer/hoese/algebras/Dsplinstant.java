

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import  viewer.hoese.*;
import  java.util.*;
import  javax.swing.*;
import  java.awt.*;


/**
 * A displayclass for the instant-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplinstant extends DsplGeneric
    implements Timed {
  Interval TimeBounds;
  boolean err = true;

  /** A method of the Timed-Interface
   * 
   * @return the global time boundaries [min..max] this instance is defined at
   * @see <a href="Dsplinstantsrc.html#getTimebounds">Source</a>
   */
  public Interval getTimeBounds () {
    return  TimeBounds;
  }

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplinstantsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    int start = 0;              //(int)(TimeBounds.getStart()*PixelTime);
    JLabel label = new JLabel("|"+LEUtils.convertTimeToString(TimeBounds.getStart()).substring(11, 
        16), JLabel.LEFT);
    label.setBounds(start , 15, 100, 15);
    label.setVerticalTextPosition(JLabel.CENTER);
    label.setHorizontalTextPosition(JLabel.RIGHT);
    JPanel jp = new JPanel(null);
    jp.setPreferredSize(new Dimension(100, 25));
    jp.add(label);
    //Add labels to the JPanel. 
    return  jp;
  }

  /**
   * Scans the representation of an instant datatype 
   * @param v An instant value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplinstantsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    Double d;
    //System.out.println(v.writeListExprToString());
    d = LEUtils.readInstant(v);
    if (d == null)
      return;
    TimeBounds = new Interval(d.doubleValue(), d.doubleValue(), true, true);
    err = false;
  }

  /**
   * Init. the Dsplinstant instance.
   * @param type The symbol instant
   * @param value The value of an instant .
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplinstantsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(Instant))"));
      return;
    } 
    else 
      qr.addEntry(this);
  }

  /** The text representation of this object 
   * @see <a href="Dsplinstantsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ":" + LEUtils.convertTimeToString(TimeBounds.getStart())
        + ": TA(Instant) ";
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplinstantsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    Vector v=new Vector(1,0);
    v.add(TimeBounds);
    return v;
    } 
}



