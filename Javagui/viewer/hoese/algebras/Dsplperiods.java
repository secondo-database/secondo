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
import  javax.swing.border.*;
import  java.awt.*;
import tools.Reporter;


/**
 * A displayclass for the periods-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplperiods extends DsplGeneric implements Timed {
  protected Vector Intervals = new Vector(10, 5);
  protected Interval TimeBounds;
  protected boolean err=true;
  protected boolean defined;
  protected String entry;

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplperiodssrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    if(!defined){
        return new JPanel();
    }
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

  /** returns the Timebounds of this object **/
  public Interval getTimeBounds(){
    return TimeBounds;
  }

  /**
   * Scans the representation of a periods datatype and constructs the Intervals Vector
   * @param v A list of time intervals
   * @see sj.lang.ListExpr
   * @see <a href="Dsplperiodssrc.html#ScanValue">Source</a>
   */
  public String getString (ListExpr v) {
    if(isUndefined(v)){
        defined = false;
        err=false;
        return "undefined";
    }
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      if (le.listLength() != 4){
        err=true;
        defined=false;
        return "<error>";
      }
      Interval in = LEUtils.readInterval(le);
      if (in == null){
        err=true;
        defined=false;
        return "<error>";
      }
      Intervals.add(in);
      v = v.rest();
    }
    defined = true;
    err = false;
    return "periods";
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
    String t = type.symbolValue();
    String v = getString(value);
    entry = t+":"+v;
    if(err){
       qr.addEntry(entry); 
       return;
    }
    qr.addEntry(this);
    computeTimeBounds(); 
  }
public void init (ListExpr type, int typewidth, ListExpr value,
                  int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getString(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     entry=(T + " : " + V);
     if(err){
        qr.addEntry(entry);
     } else{
        qr.addEntry(this);
     }
     computeTimeBounds();
  }


 /** computes the timebounds for this objects **/
  private void computeTimeBounds(){
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

   public String toString(){
     return entry;
  }


  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplperiodssrc.html#getIntervals">Source</a>
   */

  public Vector getIntervals(){
    return Intervals;
    } 

}



