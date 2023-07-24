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
public class Dsplmovingint extends DsplGeneric implements LabelAttribute, Timed, 
                                   Function, ExternDisplay, RenderAttribute{

  /** the interval covering only the bounded intervals of this moving int **/
  protected Interval TimeBounds; 

  protected boolean err = true;
  protected boolean defined;
  protected String entry;
  protected Vector Intervals = new Vector(10, 5);
  protected Vector Ints = new Vector(10, 5);
  protected int min;
  protected int max;
  private static  FunctionFrame functionframe = new FunctionFrame();
  //private static  MultiFunctionFrame functionframe = new MultiFunctionFrame();
  
  
  public    Double getValueAt(double time){
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    if(index<0) return null;
    return Double.valueOf(((Integer)Ints.get(index)).doubleValue()); 
  }


  public boolean isExternDisplayed(){
      return(functionframe.isVisible() && this.equals(functionframe.getSource()));
      //return functionframe.isVisible() && functionframe.contains(this);
  }

  public void  displayExtern(){
      if(!defined){
          Reporter.showInfo("not defined");
          return;
      }
      if(TimeBounds!=null){
         functionframe.setSource(this);
         //functionframe.addFunction(this);        
         functionframe.setVisible(true);
         functionframe.toFront();
      } else{
         Reporter.showInfo("The moving int is empty");
      }
  }
  
  public Interval getInterval(){
     return  TimeBounds;
  }

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
  public Interval getBoundingInterval () {
    return TimeBounds;
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
      Intervals.add(in);
	  	Ints.add(Integer.valueOf(i));
      v = v.rest();
    }
    defined = true;
    err = false;
  }
  

  public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr){
    AttrName = extendString(name,nameWidth, indent);
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
      if(!in.isInfinite()){
         if (TimeBounds == null) {
             TimeBounds = in;
         } else {
             TimeBounds = TimeBounds.union(in);
         }
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



