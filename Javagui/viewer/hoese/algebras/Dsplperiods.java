

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import  java.util.*;
import  javax.swing.*;
import  javax.swing.border.*;
import  java.awt.*;


/**
 * A displayclass for the periods-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplperiods extends Dsplinstant {
  Vector Intervals = new Vector(10, 5);

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplperiodssrc.html#getTimeRenderer">Source</a>
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

  /**
   * Scans the representation of a periods datatype and constructs the Intervals Vector
   * @param v A list of time intervals
   * @see sj.lang.ListExpr
   * @see <a href="Dsplperiodssrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    ////System.out.println(v.writeListExprToString());
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      //System.out.println(le.writeListExprToString());
      if (le.listLength() != 4)
        return;
      Interval in = LEUtils.readInterval(le);
      if (in == null)
        return;
      Intervals.add(in);
      v = v.rest();
    }
    err = false;
  }

  /**
   * Init. the Dsplperiods instance.
   * @param type The symbol periods
   * @param value A list of time intervals
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplperiodssrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(Periods))"));
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
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplperiodssrc.html#getIntervals">Source</a>
   */

  public Vector getIntervals(){
    return Intervals;
    } 

  /** The text representation of this object 
   * @see <a href="Dsplperiodssrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ": TA(Periods) ";
  }
}



