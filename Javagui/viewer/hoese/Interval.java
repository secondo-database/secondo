

package  viewer.hoese;

import java.util.Vector;
/**
 * An instance of this class represent a time interval
 */
public class Interval {
  double start, end;
  boolean leftclosed, rightclosed;

  /**
   * Constructor of an interval
   * @param   double astart start-time of the interval
   * @param   double aend end-time of the interval
   * @param   boolean leftcl Is it leftclosed?
   * @param   boolean rightcl Is it rightclosed?
   * @see <a href="Intervalsrc.html#getAttrName">Source</a> 
   */
  public Interval (double astart, double aend, boolean leftcl, boolean rightcl) {
    start = astart;
    end = aend;
    leftclosed = leftcl;
    rightclosed = rightcl;
  }

  /**
   * 
   * @return True if this interval is leftclosed
   * @see <a href="Intervalsrc.html#isLeftclosed">Source</a> 
   */
  public boolean isLeftclosed () {
    return  leftclosed;
  }

  /**
   * 
   * @return True if this interval is rightclosed
   * @see <a href="Intervalsrc.html#isRightclosed">Source</a> 
   */
  public boolean isRightclosed () {
    return  rightclosed;
  }

  /**
   * 
   * @return Start-time of this interval.
   * @see <a href="Intervalsrc.html#getStart">Source</a> 
   */
  public double getStart () {
    return  start;
  }

  /**
   * 
   * @return End-time of this interval
   * @see <a href="Intervalsrc.html#getEnd">Source</a> 
   */
  public double getEnd () {
    return  end;
  }

  /**
   * Creates the union of this interval and another
   * @param iv The other interval
   * @return The union-interval
   * @see <a href="Intervalsrc.html#union">Source</a> 
   */
  public Interval union (Interval iv) {
    Interval i = new Interval(0, 0, true, true);
    if (start < iv.start) {
      i.leftclosed = leftclosed;
      i.start = start;
    } 
    else if (start == iv.start) {
      i.leftclosed = leftclosed || iv.leftclosed;
      i.start = start;
    } 
    else {
      i.leftclosed = iv.leftclosed;
      i.start = iv.start;
    }
    if (end > iv.end) {
      i.rightclosed = rightclosed;
      i.end = end;
    } 
    else if (end == iv.end) {
      i.rightclosed = rightclosed || iv.rightclosed;
      i.end = end;
    } 
    else {
      i.rightclosed = iv.rightclosed;
      i.end = iv.end;
    }
    return  i;
  }

  /**
   * Tests if this Interval is defined at a certain time t
   * @param t A time value
   * @return True if defined
   * @see <a href="Intervalsrc.html#isDefinedAt">Source</a> 
   */
  public boolean isDefinedAt (double t) {
    if (leftclosed && (Math.abs(t -start)<0.00069))
      return  true;
    if (rightclosed && (Math.abs(t -end)<0.00069))
      return  true;
    return  ((t > start) && (t < end));
  }
  /**
   * Searches the Vector ivs of intervals for the minimal-value greater than t
   * @param t A time value
   * @return The found time in Minutes or Integer.Max_Value if not found
   * @see <a href="Intervalsrc.html#getMinGT">Source</a> 
   */
  public static int getMinGT (Vector ivs,double t) {
    double min=Double.MAX_VALUE;

    for (int i = 0; i < ivs.size(); i++) {
      Interval iv=(Interval)ivs.elementAt(i);
      if (iv.isDefinedAt(t)) min=Math.min(min,t);
      else if (t<=iv.start) min = Math.min(min,(iv.leftclosed)? iv.start:iv.start+0.00069);
    }  
    if (min==Double.MAX_VALUE) return Integer.MAX_VALUE;
    else return  (int)Math.round(min*1440);
  }


}



