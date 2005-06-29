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
    if(viewer.HoeseViewer.DEBUG_MODE && start==end && (!leftclosed || !rightclosed)){
        System.out.println("Warning: nonclosed interval of length null created !");
    }
  }

  /** returns the String representation for this interval */
  public String toString(){
    String res= leftclosed?"[":"(";
    res = res + DateTime.getString(start)+" , "+DateTime.getString(end);
    res = res + (rightclosed?"]":")");
    return res;
  }


  /** returns the length of this interval in milliseconds **/
  public long length(){
      double diff = end-start;
      return (long)(diff*86400000);
  }

  public String getDoubles(){
     String r= leftclosed?"[":"(";
     r+=start+"  -  "+end;
     r = r + (rightclosed?"]":")");
     return r; 
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
    if (leftclosed && (Math.abs(t -start)<1.0/DateTime.DAY_RESOLUTION))
      return  true;
    if (rightclosed && (Math.abs(t -end)<1.0/DateTime.DAY_RESOLUTION))
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
      else if (t<=iv.start) min = Math.min(min,(iv.leftclosed)? iv.start:iv.start+1.0/DateTime.DAY_RESOLUTION);
    }
    if (min==Double.MAX_VALUE) return Integer.MAX_VALUE;
    else return  (int)Math.round(min*DateTime.DAY_RESOLUTION);
  }


}



