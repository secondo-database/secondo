
package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import  java.util.*;
import  javax.swing.*;
import  java.awt.*;
import  javax.swing.border.*;


/**
 * A displayclass for the movingstring-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplmovingstring extends Dsplinstant {
  Vector Intervals = new Vector(10, 5);
  Vector Strings = new Vector(10, 5);

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplmovingstringsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
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
      String bs = Strings.elementAt(cnt++).toString();
      JLabel jc = new JLabel(bs);
      jc.setFont(new Font("Dialog", Font.PLAIN, 12));
      jc.setOpaque(true);
      jc.setForeground(Color.black);
      jc.setBackground(Color.yellow);
      jc.setPreferredSize(new Dimension(1, 18));
      jc.setBorder(new MatteBorder(2, (in.isLeftclosed()) ? 2 : 0, 2, (in.isRightclosed()) ?
          2 : 0, Color.black));
      Dimension d = jc.getPreferredSize();
      jc.setBounds(start, (int)d.getHeight()*0 + 7, end - start, (int)d.getHeight());
      jc.setToolTipText(LEUtils.convertTimeToString(in.getStart()) + "..." + 
          LEUtils.convertTimeToString(in.getEnd()) + "=" + bs);
      jp.setPreferredSize(new Dimension((int)((TimeBounds.getEnd() - TimeBounds.getStart())*PixelTime), 
          25));
      jp.add(jc);
    }
    return  jp;
  }

  /**
   * Scans the representation of a movingstring datatype 
   * @param v A list of time-intervals with a string value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingstringsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    ////System.out.println(v.writeListExprToString());
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      //System.out.println(le.writeListExprToString());
      if (le.listLength() != 5)
        return;
      Interval in = LEUtils.readInterval(ListExpr.fourElemList(le.first(), 
          le.second(), le.third(), le.fourth()));
      if (in == null)
        return;
      Intervals.add(in);
      if (le.fifth().atomType() != ListExpr.STRING_ATOM)
        return;
      Strings.add(le.fifth().stringValue());
      v = v.rest();
    }
    err = false;
  }

  /**
   * Init. the Dsplmovingstring instance.
   * @param type The symbol movingstring.
   * @param value A list of time-intervals with an string value
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingstringsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(MString))"));
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
   * @see <a href="Dsplmovingstringsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ": TA(MString) ";
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplmovingstringsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    return Intervals;
    } 
}



