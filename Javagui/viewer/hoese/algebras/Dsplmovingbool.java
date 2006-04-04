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

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import javax.swing.border.*;
import tools.Reporter;


/**
 * A displayclass for the movingbool-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplmovingbool extends Dsplinstant
  implements LabelAttribute, RenderAttribute {
  Vector Intervals = new Vector(10, 5);
  Vector Bools = new Vector(10, 5);

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplmovingboolsrc.html#getTimeRenderer">Source</a>
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
      String bs = Bools.elementAt(cnt++).toString();
      JLabel jc = new JLabel(bs);
      jc.setFont(new Font("Dialog", Font.PLAIN, 12));
      jc.setForeground(Color.black);
      jc.setOpaque(true);
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
   * Scans the representation of a movingbool datatype 
   * @param v A list of time-intervals with a bool value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingboolsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if(isUndefined(v)){
       defined=false;
       return;
    } 
    defined = true;
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      int len = le.listLength();
      ListExpr value;
      Interval in;
      if (len == 5){
          in =  LEUtils.readInterval(ListExpr.fourElemList(le.first(), 
                                    le.second(), le.third(), le.fourth()));
          value = le.fifth();
      } else if(len==2){
           in = LEUtils.readInterval(le.first());
           value = le.second();
      } else{ // wrong list length
           return;
      }
      if (in == null)
        return;
      Intervals.add(in);
      if (value.atomType() != ListExpr.BOOL_ATOM)
        return;
      boolean b = value.boolValue();
      Bools.add(new Boolean(b));
      v = v.rest();
    }
    err = false;
  }

  /**
   * Init. the Dsplmovingbool instance.
   * @param type The symbol movingbool.
   * @param value A list of time-intervals with an boolean value
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingboolsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(MBool))"));
      defined=false;
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
   * @see <a href="Dsplmovingboolsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ": TA(MBool) ";
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplmovingboolsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    return Intervals;
    }

   /** implementation of the LabelAttribute interface **/
  public String getLabel(double time){
      int index = IntervalSearch.getTimeIndex(time,Intervals);
      if(index<0){
        return "undefined";
      }
      return Bools.get(index).toString();
  }


  /* Implementation of the renderAttribute interface */

  public boolean mayBeDefined(){
     // the min and max render functions returns valid values in
     // each case, so we can return true even when this instance is
     // undefined
     return true;
  }
  
  public boolean isDefined(double time){
     // we return in each case valid values in the getRenderValue function
     return true;  
  }

  public double getMinRenderValue(){
     return 0;
  }
  public double getMaxRenderValue(){
     return 1;
  }

  public double getRenderValue(double time){
     if(!defined){
        return 0.5; // value for undefined
     }
     int index = IntervalSearch.getTimeIndex(time,Intervals);
     if(index<0){
        return 0.5;
     }
     boolean v = ((Boolean)Bools.get(index)).booleanValue();
     return v?1:0;
  }

  
 
}



