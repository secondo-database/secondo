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
import tools.Reporter;


/**
 * A displayclass for the movingstring-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplmovingstring extends DsplGeneric implements Timed,LabelAttribute {
  protected Vector Intervals = new Vector(10, 5);
  protected Vector Strings = new Vector(10, 5);
  protected Interval TimeBounds;
  protected boolean defined;
  protected String entry;
  protected boolean err=true; 

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplmovingstringsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    if(!defined){
      return new JPanel();
    }
    JPanel jp = new JPanel(null);
    if (Intervals == null)
      return  null;
    ListIterator li = Intervals.listIterator();
    int cnt = 0;
    while (li.hasNext()) {
      Interval in = (Interval)li.next();
      int start = (int)((in.getStart() - TimeBounds.getStart())*PixelTime);
      int end = (int)((in.getEnd() - TimeBounds.getStart())*PixelTime);
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

  /** Returns the label at the given time **/
  public String getLabel(double time){
    if(err | !defined){
      return null;
    }
    int index = IntervalSearch.getTimeIndex(time,Intervals);
    if(index<0){
      return null;
    }
    return Strings.get(index).toString();
 }

  /** returns the bounds of the interval **/
  public Interval getBoundingInterval(){
     return TimeBounds;
  }

  /**
   * Scans the representation of a movingstring datatype 
   * @param v A list of time-intervals with a string value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingstringsrc.html#ScanValue">Source</a>
   */
  public String getString (ListExpr v) {
    if(isUndefined(v)){
        err=false;
        defined=false;
        return "undefined";
    }
    if(v.atomType()!=ListExpr.NO_ATOM){
       err=true;
       defined=false;
       return "<error>";
    }


    while (!v.isEmpty()) {
      ListExpr le = v.first();
      if (le.listLength() != 5){
        err=true;
        defined=false;
        return "<error>";
      }
      Interval in = LEUtils.readInterval(ListExpr.fourElemList(le.first(), 
          le.second(), le.third(), le.fourth()));
      if (in == null){
        defined=false;
        err=true;
        return "<error>";
      }
      Intervals.add(in);
      if (le.fifth().atomType() != ListExpr.STRING_ATOM){
        defined=false;
        err=true;
        return "<error>";

      }
      Strings.add(le.fifth().stringValue());
      v = v.rest();
    }
    err = false;
    defined=true;
    return "mstring";
    
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
    init(type,0,value,0,qr);
  }

   public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getString(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     entry=(T + " : " + V);
     if(err){
        qr.addEntry(entry);
     }else{
        qr.addEntry(this);
     } 
     // compute the bounding interval
     TimeBounds = null;
     for(int i=0;i<Intervals.size();i++){
       Interval in = (Interval) Intervals.get(i);
       if(!in.isInfinite()){
         if(TimeBounds==null){
            TimeBounds = in.copy();
         }else{
            TimeBounds.unionInternal(in);
         }
       }

     }
  }


  /** The text representation of this object 
   * @see <a href="Dsplmovingstringsrc.html#toString">Source</a>
   */
  public String toString () {
    return  entry;
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplmovingstringsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    return Intervals;
    } 
}



