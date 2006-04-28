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
import  gui.Environment;
import tools.Reporter;


/**
 * A displayclass for the movingint-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplmovingint extends DsplGeneric implements LabelAttribute, Timed, RenderAttribute{

  protected Interval TimeBounds;
  protected boolean err = true;
  protected boolean defined;
  protected String entry;
  protected Vector Intervals = new Vector(10, 5);
  protected Vector Ints = new Vector(10, 5);
  protected int min;
  protected int max;
  
  
  /** returns the value of this moving integer at the given time as string */
public String getLabel(double time){
    if(!defined | err){
      return null;
    }
    if(Intervals==null || Ints==null){
        return null; 
    }
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    if(index<0){
       return null;
    }
    return "" + Ints.get(index);
  }

  /** Returns whether this moving integer has any value .
    * This means, this integer is not empty, defined and the
    * scanning of the list has been successfully finished.
    **/
  public boolean mayBeDefined(){
    return !err && defined && (Intervals.size()>0); 
  }
  
  /** Returns the minimum value of this moving integer **/
  public double getMinRenderValue(){
     return min;
  }
  /** Returns the maximum value of this moving integer **/
  public double getMaxRenderValue(){
     return max;
  }

  /** Checks whether this moving integer is defined to the given time **/
  public boolean isDefined(double time){
    if(!defined | err){
       return false;
    }
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    return index>=0;
  } 

  /** Returns the value of this moving integer at the
    *  given time 
    **/
  public double getRenderValue(double time){
    if(!defined | err){
       return (min+max)/2;
    }
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    if(index<0){
       return (min+max)/2;
    }
    return ((Integer)Ints.get(index)).intValue();
  }
  

  /** A method of the Timed-Interface
   * 
   * @return the global time boundaries [min..max] this instance is defined at
   */
  public Interval getTimeBounds () {
    return  TimeBounds;
  }

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplmovingintsrc.html#getTimeRenderer">Source</a>
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
      String bs = Ints.elementAt(cnt++).toString();
      JLabel jc = new JLabel(bs);
      jc.setFont(new Font("Dialog", Font.PLAIN, 12));
      jc.setForeground(Color.black);
      jc.setOpaque(true);
      jc.setBackground(Color.yellow);
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
   * Scans the representation of a movingint datatype
   * @param v A list of time-intervals with an int value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingintsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if(isUndefined(v)){
       defined=false;
       err=false;
       return;
    }
    if(v.atomType()!=ListExpr.NO_ATOM){
       err=true;
       return;
    } 
    boolean first=true;
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      Interval in = null;
      ListExpr value=null;
      if (le.listLength() == 5){
            Reporter.writeWarning("Deprecated list representation of moving int");
            in = LEUtils.readInterval(ListExpr.fourElemList(le.first(),
                  le.second(), le.third(), le.fourth()));
            value = le.fifth();
      }
      if(le.listLength()==2){ // (interval int)
         in = LEUtils.readInterval(le.first());
         value = le.second();
      }
      if (in == null){ // error in reading interval
        Reporter.debug("Dsplmovingint: cannot read the interval from list ");
        return;
      } 
			if (value.atomType() != ListExpr.INT_ATOM){ // error in reading value
				Reporter.debug("Dsplmovingint: error in ListExpr, int atom required");
				return;
			}
			int i = value.intValue();
			if(first){
				min = i;
				max = i;
				first = false;
			} else{
				min = i<min?i:min;
				max = i>max?i:max;  
			}
      if(!in.isInfinite() || (infiniteIntervalMode==LEFT_INFINITE_INTERVALS)){ // handle normal intervals 
					Intervals.add(in);
					Ints.add(new Integer(i));
      }else if(infiniteIntervalMode==RESTRICT_INFINITE_INTERVALS){
          in.restrict(infiniteIntervalLength);
          if(!in.isInfinite()){
             Intervals.add(in);
             Ints.add(new Integer(i));
          }
      }
      v = v.rest();
    }
    defined = true;
    err = false;
  }
  

  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is 
   * neccessary for the displaying this type in the queryresultlist.
   * @param type A ListExpr of the datatype string 
   * @param value A string in a listexpr
   * @param qr The queryresultlist to add alphanumeric representation
   * @see QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplstringsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
      init(type,0,value,0,qr);
  }

  public void init (ListExpr type,int typeWidth,ListExpr value,int valueWidth, QueryResult qr){
    AttrName = extendString(type.symbolValue(),typeWidth);
    ScanValue(value);
    if (err) {
      defined=false;
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry( AttrName + ": <error>");
      return;
    } 
    else 
      qr.addEntry(this);
    // compute the timebounds
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
   * @see <a href="Dsplmovingintsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ": TA(MInt) ";
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplmovingintsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    return Intervals;
    } 
}



